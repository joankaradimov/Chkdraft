#include <gtest/gtest.h>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <chkdraft_mcp/json.h>
#include <chkdraft_mcp/map_service.h>
#include <chkdraft_mcp/mcp_server.h>

using JsonDoc = mcp::Json; // RareCpp owns the name Json at global scope

TEST(ChkdraftMcpJson, RoundTrip)
{
    std::string text = "{\"jsonrpc\":\"2.0\",\"id\":7,\"method\":\"tools/call\",\"params\":{\"name\":\"read_tiles\","
        "\"arguments\":{\"map\":1,\"left\":0,\"top\":0,\"width\":2,\"height\":1,\"scope\":\"editor\",\"flag\":true,\"none\":null,"
        "\"text\":\"a \\\"quote\\\" and a \\u00e9 and a \\ud83d\\ude00\",\"list\":[1,2.5,-3]}}}";
    JsonDoc parsed = JsonDoc::parse(text);
    ASSERT_TRUE(parsed.isObject());
    EXPECT_EQ(parsed.find("id")->number, 7);
    EXPECT_EQ(parsed.find("method")->string, "tools/call");
    const JsonDoc & arguments = *parsed.find("params")->find("arguments");
    EXPECT_EQ(arguments.find("scope")->string, "editor");
    EXPECT_TRUE(arguments.find("flag")->boolean);
    EXPECT_TRUE(arguments.find("none")->isNull());
    EXPECT_EQ(arguments.find("text")->string, "a \"quote\" and a \xC3\xA9 and a \xF0\x9F\x98\x80");
    ASSERT_EQ(arguments.find("list")->items.size(), 3u);
    EXPECT_EQ(arguments.find("list")->items[1].number, 2.5);

    JsonDoc again = JsonDoc::parse(parsed.dump());
    EXPECT_EQ(again.dump(), parsed.dump());
    EXPECT_THROW(JsonDoc::parse("{\"a\":}"), std::runtime_error);
    EXPECT_THROW(JsonDoc::parse("[1,2"), std::runtime_error);
}

TEST(ChkdraftMcpServer, ProtocolWithoutGameData)
{
    mcp::MapService service {};
    mcp::McpServer server { service };

    JsonDoc initialized = server.handle(JsonDoc::parse("{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{}}"));
    ASSERT_TRUE(initialized.isObject());
    EXPECT_EQ(initialized.find("id")->number, 1);
    EXPECT_EQ(initialized.find("result")->find("protocolVersion")->string, "2024-11-05");
    EXPECT_NE(initialized.find("result")->find("capabilities")->find("tools"), nullptr);

    EXPECT_TRUE(server.handle(JsonDoc::parse("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}")).isNull());

    JsonDoc listed = server.handle(JsonDoc::parse("{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/list\"}"));
    const JsonDoc & tools = *listed.find("result")->find("tools");
    ASSERT_EQ(tools.items.size(), 10u);
    for ( const auto & tool : tools.items )
    {
        EXPECT_TRUE(tool.find("name")->isString());
        EXPECT_TRUE(tool.find("inputSchema")->find("properties")->isObject());
    }

    JsonDoc unknown = server.handle(JsonDoc::parse("{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"nothing\"}"));
    EXPECT_EQ(unknown.find("error")->find("code")->number, -32601);

    // A tool that fails answers with an error result, not a protocol error
    JsonDoc noMap = server.handle(JsonDoc::parse("{\"jsonrpc\":\"2.0\",\"id\":4,\"method\":\"tools/call\",\"params\":{\"name\":\"list_brushes\",\"arguments\":{\"map\":42}}}"));
    ASSERT_NE(noMap.find("result"), nullptr);
    EXPECT_TRUE(noMap.find("result")->find("isError")->boolean);

    JsonDoc missing = server.handle(JsonDoc::parse("{\"jsonrpc\":\"2.0\",\"id\":5,\"method\":\"tools/call\",\"params\":{\"name\":\"new_map\",\"arguments\":{\"width\":64}}}"));
    EXPECT_TRUE(missing.find("result")->find("isError")->boolean);

    // The line loop: a parse error answers with id null, a notification with nothing
    std::istringstream in("not json\n{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}\n{\"jsonrpc\":\"2.0\",\"id\":6,\"method\":\"ping\"}\n");
    std::ostringstream out {};
    server.run(in, out);
    std::string lines = out.str();
    EXPECT_NE(lines.find("-32700"), std::string::npos);
    EXPECT_NE(lines.find("\"id\":6"), std::string::npos);
    EXPECT_EQ(std::count(lines.begin(), lines.end(), '\n'), 2);
}

// The rest needs the game: SC_ASSET names a directory holding StarDat.mpq, BrooDat.mpq and patch_rt.mpq, as the
// mapping core tests take it
static std::optional<mcp::DataFiles> gameFiles()
{
    const char* scAsset = std::getenv("SC_ASSET");
    if ( scAsset == nullptr || *scAsset == '\0' )
        return std::nullopt;
    return mcp::DataFiles { scAsset, {} };
}

static size_t count(const JsonDoc & json, const char* name)
{
    const JsonDoc* field = json.find(name);
    return field == nullptr ? 0 : field->items.size();
}

TEST(ChkdraftMcpService, TilesetsAndBrushes)
{
    auto files = gameFiles();
    if ( !files )
        GTEST_SKIP() << "SC_ASSET is not set";

    mcp::MapService service {};
    JsonDoc listed = service.listTilesets(*files);
    ASSERT_GE(count(listed, "tilesets"), 8u);
    const JsonDoc & jungle = listed.find("tilesets")->items[4];
    EXPECT_EQ(jungle.find("index")->number, 4);
    EXPECT_EQ(jungle.find("name")->string, "jungle");
    EXPECT_EQ(count(jungle, "brushes"), 13u); // Dirt through Mud, as the jungle brush table has them
    EXPECT_EQ(jungle.find("brushes")->items[0].find("name")->string, "Dirt");
}

TEST(ChkdraftMcpService, PlaceBrushSaveAndReload)
{
    auto files = gameFiles();
    if ( !files )
        GTEST_SKIP() << "SC_ASSET is not set";

    mcp::MapService service {};
    JsonDoc created = service.newMap(*files, 4, 64, 64, std::nullopt, "expansion_chk");
    int mapId = int(created.find("map")->number);
    EXPECT_EQ(created.find("width")->number, 64);
    EXPECT_EQ(created.find("format")->string, "expansion_chk");

    JsonDoc brushes = service.listBrushes(mapId);
    ASSERT_GE(count(brushes, "brushes"), 2u);
    size_t defaultBrush = size_t(brushes.find("default_brush")->number);
    size_t otherBrush = 0;
    for ( const auto & brush : brushes.find("brushes")->items )
    {
        if ( size_t(brush.find("terrain_type")->number) != defaultBrush )
        {
            otherBrush = size_t(brush.find("terrain_type")->number);
            break;
        }
    }
    ASSERT_NE(otherBrush, 0u);

    // A fresh map is one plain everywhere: every ISOM side holds the default brush's value
    JsonDoc before = service.readIsom(mapId, 0, 0, 33, 65);
    ASSERT_EQ(count(before, "rects"), 65u);
    uint16_t plainValue = uint16_t(before.find("rects")->items[10].items[10].find("left")->number);
    EXPECT_NE(plainValue, 0);

    JsonDoc placed = service.placeBrush(mapId, otherBrush, 32, 32, 1);
    EXPECT_TRUE(placed.find("placed")->boolean);
    const JsonDoc & area = *placed.find("changed_area");
    EXPECT_LE(area.find("left")->number, area.find("right")->number);

    // The placement changed some ISOM sides and some tiles, and stayed bounded rather than running across the map
    JsonDoc after = service.readIsom(mapId, 0, 0, 33, 65);
    size_t changedSides = 0;
    for ( size_t y=0; y<65; ++y )
    {
        for ( size_t x=0; x<33; ++x )
        {
            const JsonDoc & rect = after.find("rects")->items[y].items[x];
            for ( const char* side : {"left", "top", "right", "bottom"} )
            {
                if ( uint16_t(rect.find(side)->number) != plainValue )
                    ++changedSides;
            }
        }
    }
    EXPECT_GT(changedSides, 0u);
    EXPECT_LT(changedSides, 33u*65u);

    JsonDoc tiles = service.readTiles(mapId, 24, 24, 16, 16, "editor");
    ASSERT_EQ(count(tiles, "tiles"), 16u);
    ASSERT_EQ(tiles.find("tiles")->items[0].items.size(), 16u);
    JsonDoc corner = service.readTiles(mapId, 0, 0, 4, 4, "game");
    uint16_t cornerTile = uint16_t(corner.find("tiles")->items[0].items[0].number);
    uint16_t middleTile = uint16_t(tiles.find("tiles")->items[8].items[8].number);
    EXPECT_NE(cornerTile / 16, middleTile / 16); // The middle is another tile group than the untouched corner

    // Reading past the edge clips
    JsonDoc clipped = service.readTiles(mapId, 60, 62, 10, 10, "game");
    EXPECT_EQ(clipped.find("width")->number, 4);
    EXPECT_EQ(clipped.find("height")->number, 2);

    // A tile written directly lands in both scopes, a rectangle of them clips to the map, and nothing stood there to remove
    JsonDoc placedTile = service.placeTile(mapId, 2, 2, 1, 1, middleTile);
    EXPECT_EQ(count(*placedTile.find("removed"), "doodads"), 0u);
    EXPECT_EQ(count(*placedTile.find("removed"), "units"), 0u);
    for ( const char* scope : {"game", "editor"} )
    {
        JsonDoc written = service.readTiles(mapId, 2, 2, 1, 1, scope);
        EXPECT_EQ(uint16_t(written.find("tiles")->items[0].items[0].number), middleTile);
    }
    JsonDoc filled = service.placeTile(mapId, 62, 63, 5, 5, cornerTile);
    EXPECT_EQ(filled.find("width")->number, 2);
    EXPECT_EQ(filled.find("height")->number, 1);
    EXPECT_THROW(service.placeTile(mapId, 64, 0, 1, 1, cornerTile), mcp::ServiceError);
    EXPECT_THROW(service.placeTile(mapId, 0, 0, 1, 1, 65535), mcp::ServiceError);

    // The service takes UTF-8 paths, and a narrow path would be read in the ANSI code page under libc++
    auto filePath = std::filesystem::temp_directory_path() / "chkdraft_mcp_test.chk";
    auto utf8Path = filePath.u8string();
    std::string path(utf8Path.begin(), utf8Path.end());
    JsonDoc saved = service.saveMap(mapId, path, "");
    EXPECT_EQ(saved.find("path")->string, path);
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    JsonDoc loaded = service.loadMap(*files, path);
    int loadedId = int(loaded.find("map")->number);
    EXPECT_NE(loadedId, mapId);
    EXPECT_EQ(loaded.find("tileset")->number, 4);
    JsonDoc reloaded = service.readTiles(loadedId, 24, 24, 16, 16, "game");
    JsonDoc original = service.readTiles(mapId, 24, 24, 16, 16, "game");
    EXPECT_EQ(reloaded.find("tiles")->dump(), original.find("tiles")->dump());

    EXPECT_TRUE(service.closeMap(mapId).find("closed")->boolean);
    EXPECT_TRUE(service.closeMap(loadedId).find("closed")->boolean);
    EXPECT_THROW(service.listBrushes(mapId), mcp::ServiceError);
    std::filesystem::remove(filePath);
}
