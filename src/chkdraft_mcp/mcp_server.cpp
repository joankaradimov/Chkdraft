#include "mcp_server.h"
#include <istream>
#include <ostream>

namespace mcp
{
    static constexpr const char* ProtocolVersion = "2024-11-05";
    static constexpr const char* ServerName = "chkdraft-mcp";
    static constexpr const char* ServerVersion = "0.1.0";

    static constexpr int ParseError = -32700;
    static constexpr int InvalidRequest = -32600;
    static constexpr int MethodNotFound = -32601;
    static constexpr int InvalidParams = -32602;

    // A request that cannot be answered as a tool result: the JSON-RPC error goes back instead
    struct ProtocolError : std::runtime_error
    {
        int code;
        ProtocolError(int code, const std::string & message) : std::runtime_error(message), code(code) {}
    };

    McpServer::McpServer(MapService & service) : service(service) {}

    // Argument readers: a missing or mistyped argument answers with what was expected

    static const Json & require(const Json & arguments, const std::string & name)
    {
        const Json* value = arguments.find(name);
        if ( value == nullptr || value->isNull() )
            throw ServiceError("The argument \"" + name + "\" is required");
        return *value;
    }

    static std::string requireString(const Json & arguments, const std::string & name)
    {
        const Json & value = require(arguments, name);
        if ( !value.isString() )
            throw ServiceError("The argument \"" + name + "\" has to be a string");
        return value.string;
    }

    static std::string optionalString(const Json & arguments, const std::string & name, const std::string & fallback = "")
    {
        const Json* value = arguments.find(name);
        if ( value == nullptr || value->isNull() )
            return fallback;
        if ( !value->isString() )
            throw ServiceError("The argument \"" + name + "\" has to be a string");
        return value->string;
    }

    static size_t asIndex(const Json & value, const std::string & name)
    {
        if ( !value.isNumber() || value.number < 0 || value.number != double(static_cast<long long>(value.number)) )
            throw ServiceError("The argument \"" + name + "\" has to be a whole number, 0 or more");
        return size_t(value.number);
    }

    static size_t requireIndex(const Json & arguments, const std::string & name)
    {
        return asIndex(require(arguments, name), name);
    }

    static std::optional<size_t> optionalIndex(const Json & arguments, const std::string & name)
    {
        const Json* value = arguments.find(name);
        if ( value == nullptr || value->isNull() )
            return std::nullopt;
        return asIndex(*value, name);
    }

    static int requireMapId(const Json & arguments)
    {
        return int(requireIndex(arguments, "map"));
    }

    static DataFiles requireDataFiles(const Json & arguments)
    {
        DataFiles files {};
        files.starCraft = requireString(arguments, "starcraft");
        if ( const Json* archives = arguments.find("data_files"); archives != nullptr && !archives->isNull() )
        {
            if ( !archives->isArray() )
                throw ServiceError("The argument \"data_files\" has to be an array of paths");
            for ( const auto & archive : archives->items )
            {
                if ( !archive.isString() )
                    throw ServiceError("The argument \"data_files\" has to be an array of paths");
                files.archives.push_back(archive.string);
            }
        }
        return files;
    }

    // Schema pieces, so that each tool's description below reads as a list of its arguments

    static Json property(const char* type, const std::string & description)
    {
        Json json = Json::object();
        json["type"] = Json(type);
        json["description"] = Json(description);
        return json;
    }

    static Json tool(const std::string & name, const std::string & description, Json properties, std::vector<std::string> required)
    {
        Json schema = Json::object();
        schema["type"] = Json("object");
        schema["properties"] = properties;
        Json requiredJson = Json::array();
        for ( const auto & field : required )
            requiredJson.push(Json(field));
        schema["required"] = requiredJson;

        Json json = Json::object();
        json["name"] = Json(name);
        json["description"] = Json(description);
        json["inputSchema"] = schema;
        return json;
    }

    static void addDataFileProperties(Json & properties)
    {
        properties["starcraft"] = property("string", "The StarCraft installation directory, classic or Remastered");
        Json archives = Json::object();
        archives["type"] = Json("array");
        Json item = Json::object();
        item["type"] = Json("string");
        archives["items"] = item;
        archives["description"] = Json("Archives (MPQ) and folders laid over the game's own, in order, an earlier one shadowing a later one - "
            "the same list a Chkdraft profile holds. Leave it out to use the installation's own archives. The order is part of the "
            "identity of the loaded data, so name the same files in the same order each time.");
        properties["data_files"] = archives;
    }

    static void addMapProperty(Json & properties)
    {
        properties["map"] = property("integer", "The id of an open map, as new_map or load_map gave it");
    }

    static void addRectangleProperties(Json & properties, const char* unit)
    {
        properties["left"] = property("integer", std::string("The first column, in ") + unit);
        properties["top"] = property("integer", std::string("The first row, in ") + unit);
        properties["width"] = property("integer", std::string("How many columns, in ") + unit + "; clipped to the map");
        properties["height"] = property("integer", std::string("How many rows, in ") + unit + "; clipped to the map");
    }

    static std::string formatList()
    {
        std::string list {};
        for ( const auto & [name, saveType] : MapService::formats )
            list += (list.empty() ? "" : ", ") + name;
        return list;
    }

    Json McpServer::tools() const
    {
        Json tools = Json::array();

        Json properties = Json::object();
        addDataFileProperties(properties);
        tools.push(tool("list_tilesets", "Lists the tilesets the game data holds: the eight the game ships and any that arr\\tilesets.tbl in the "
            "data files adds, each with its index, names and the brushes it offers - a brush is a terrain type that can be placed.",
            properties, {"starcraft"}));

        properties = Json::object();
        addDataFileProperties(properties);
        properties["tileset"] = property("integer", "The index of a tileset, as list_tilesets gives it");
        properties["width"] = property("integer", "The width in tiles, 1 to 65535; the game plays 64 to 256");
        properties["height"] = property("integer", "The height in tiles, 1 to 65535; the game plays 64 to 256");
        properties["terrain_type"] = property("integer", "The brush the map is filled with, by terrain type; the tileset's default brush when left out");
        properties["format"] = property("string", "The file format to save as, one of " + formatList() + "; hybrid_scm when left out");
        tools.push(tool("new_map", "Creates a map filled with one terrain and returns its id, which the other tools take.",
            properties, {"starcraft", "tileset", "width", "height"}));

        properties = Json::object();
        addDataFileProperties(properties);
        properties["path"] = property("string", "The path of an scm, scx or chk file");
        tools.push(tool("load_map", "Opens a map file against the given game data and returns its id and tileset.",
            properties, {"starcraft", "path"}));

        properties = Json::object();
        addMapProperty(properties);
        tools.push(tool("list_brushes", "Lists the brushes of an open map's tileset: the terrain types place_brush can place, with their names.",
            properties, {"map"}));

        properties = Json::object();
        addMapProperty(properties);
        properties["terrain_type"] = property("integer", "A terrain type from list_brushes");
        properties["x"] = property("integer", "The column of the tile under which the diamond is placed");
        properties["y"] = property("integer", "The row of the tile under which the diamond is placed");
        properties["extent"] = property("integer", "How many diamonds across the brush is; 1, a single diamond, when left out");
        tools.push(tool("place_brush", "Places a brush on the ISOM diamond under a tile, as a click in the editor does, and redraws the "
            "transitions around it. Returns the diamond, the area of ISOM rectangles that changed, and any doodads and units the new "
            "tiles removed, as place_tile reports them.",
            properties, {"map", "terrain_type", "x", "y"}));

        properties = Json::object();
        addMapProperty(properties);
        properties["x"] = property("integer", "The column of the tile, or of the top-left tile of a rectangle");
        properties["y"] = property("integer", "The row of the tile, or of the top-left tile of a rectangle");
        properties["tile"] = property("integer", "The tile value: 16 times a cv5 tile group plus a megatile index 0 to 15 within it");
        properties["width"] = property("integer", "How many columns to fill; 1 when left out; clipped to the map");
        properties["height"] = property("integer", "How many rows to fill; 1 when left out; clipped to the map");
        tools.push(tool("place_tile", "Writes a tile value onto a tile, or onto every tile of a rectangle, without regard to the ISOM "
            "diamonds, as the editor's tile layer does. The editor's tiles (TILE) and the game's (MTXM) both take it, unless a doodad "
            "stands on the tile legitimately. A doodad the new tile invalidates is removed with its sprite, and a unit left on ground it "
            "cannot stand on is removed; both are reported. The ISOM grid is left as it was, so a later place_brush nearby repaints the tile.",
            properties, {"map", "x", "y", "tile"}));

        properties = Json::object();
        addMapProperty(properties);
        addRectangleProperties(properties, "tiles");
        properties["scope"] = property("string", "\"game\" for the tiles the game plays (MTXM), \"editor\" for the tiles the editor keeps (TILE); game when left out");
        tools.push(tool("read_tiles", "Reads the tile values of a rectangle of the map, row by row. A tile value is 16 times its cv5 "
            "tile group plus the index of the megatile within the group.",
            properties, {"map", "left", "top", "width", "height"}));

        properties = Json::object();
        addMapProperty(properties);
        addRectangleProperties(properties, "ISOM rectangles - the grid is (width/2 + 1) by (height + 1) for a map in tiles");
        tools.push(tool("read_isom", "Reads the ISOM values of a rectangle of the map's ISOM grid, row by row: for each rectangle the "
            "value on its left, top, right and bottom side. An ISOM value indexes the tileset's isomLink table, where each brush has one "
            "entry and each transition fourteen; 0 is no terrain.",
            properties, {"map", "left", "top", "width", "height"}));

        properties = Json::object();
        addMapProperty(properties);
        properties["path"] = property("string", "The path to write; an existing file is overwritten");
        properties["format"] = property("string", "One of " + formatList() + "; the map's current format when left out");
        tools.push(tool("save_map", "Saves an open map to a file. The scm and scx formats are MPQ archives holding the scenario, the chk formats the bare scenario.",
            properties, {"map", "path"}));

        properties = Json::object();
        addMapProperty(properties);
        tools.push(tool("close_map", "Closes an open map without saving it.", properties, {"map"}));

        return tools;
    }

    Json McpServer::callTool(const std::string & name, const Json & arguments)
    {
        if ( name == "list_tilesets" )
            return service.listTilesets(requireDataFiles(arguments));
        else if ( name == "new_map" )
        {
            return service.newMap(requireDataFiles(arguments), requireIndex(arguments, "tileset"), requireIndex(arguments, "width"),
                requireIndex(arguments, "height"), optionalIndex(arguments, "terrain_type"), optionalString(arguments, "format"));
        }
        else if ( name == "load_map" )
            return service.loadMap(requireDataFiles(arguments), requireString(arguments, "path"));
        else if ( name == "list_brushes" )
            return service.listBrushes(requireMapId(arguments));
        else if ( name == "place_brush" )
        {
            return service.placeBrush(requireMapId(arguments), requireIndex(arguments, "terrain_type"), requireIndex(arguments, "x"),
                requireIndex(arguments, "y"), optionalIndex(arguments, "extent").value_or(1));
        }
        else if ( name == "place_tile" )
        {
            return service.placeTile(requireMapId(arguments), requireIndex(arguments, "x"), requireIndex(arguments, "y"),
                optionalIndex(arguments, "width").value_or(1), optionalIndex(arguments, "height").value_or(1), requireIndex(arguments, "tile"));
        }
        else if ( name == "read_tiles" )
        {
            return service.readTiles(requireMapId(arguments), requireIndex(arguments, "left"), requireIndex(arguments, "top"),
                requireIndex(arguments, "width"), requireIndex(arguments, "height"), optionalString(arguments, "scope", "game"));
        }
        else if ( name == "read_isom" )
        {
            return service.readIsom(requireMapId(arguments), requireIndex(arguments, "left"), requireIndex(arguments, "top"),
                requireIndex(arguments, "width"), requireIndex(arguments, "height"));
        }
        else if ( name == "save_map" )
            return service.saveMap(requireMapId(arguments), requireString(arguments, "path"), optionalString(arguments, "format"));
        else if ( name == "close_map" )
            return service.closeMap(requireMapId(arguments));
        else
            throw ProtocolError(InvalidParams, "Unknown tool \"" + name + "\"");
    }

    static Json textContent(const std::string & text, bool isError)
    {
        Json item = Json::object();
        item["type"] = Json("text");
        item["text"] = Json(text);
        Json content = Json::array();
        content.push(item);
        Json result = Json::object();
        result["content"] = content;
        result["isError"] = Json(isError);
        return result;
    }

    static Json errorResponse(const Json & id, int code, const std::string & message)
    {
        Json error = Json::object();
        error["code"] = Json(code);
        error["message"] = Json(message);
        Json response = Json::object();
        response["jsonrpc"] = Json("2.0");
        response["id"] = id;
        response["error"] = error;
        return response;
    }

    static Json resultResponse(const Json & id, Json result)
    {
        Json response = Json::object();
        response["jsonrpc"] = Json("2.0");
        response["id"] = id;
        response["result"] = result;
        return response;
    }

    Json McpServer::handle(const Json & message)
    {
        if ( !message.isObject() )
            return errorResponse(Json(nullptr), InvalidRequest, "A request has to be an object");

        const Json* idField = message.find("id");
        Json id = idField != nullptr ? *idField : Json(nullptr);
        bool isNotification = idField == nullptr;

        const Json* methodField = message.find("method");
        if ( methodField == nullptr || !methodField->isString() )
            return isNotification ? Json(nullptr) : errorResponse(id, InvalidRequest, "A request needs a method");
        const std::string & method = methodField->string;

        const Json* paramsField = message.find("params");
        Json params = paramsField != nullptr ? *paramsField : Json::object();

        try {
            if ( method == "initialize" )
            {
                Json capabilities = Json::object();
                capabilities["tools"] = Json::object();
                Json serverInfo = Json::object();
                serverInfo["name"] = Json(ServerName);
                serverInfo["version"] = Json(ServerVersion);
                Json result = Json::object();
                result["protocolVersion"] = Json(ProtocolVersion);
                result["capabilities"] = capabilities;
                result["serverInfo"] = serverInfo;
                result["instructions"] = Json("A headless StarCraft map editor on Chkdraft's mapping core. Give list_tilesets, new_map and "
                    "load_map the StarCraft directory and, when a mod's archives are wanted, the data files; every other tool takes a map id.");
                return resultResponse(id, result);
            }
            else if ( method == "notifications/initialized" || method.rfind("notifications/", 0) == 0 )
                return Json(nullptr);
            else if ( method == "ping" )
                return resultResponse(id, Json::object());
            else if ( method == "tools/list" )
            {
                Json result = Json::object();
                result["tools"] = tools();
                return resultResponse(id, result);
            }
            else if ( method == "tools/call" )
            {
                const Json* nameField = params.find("name");
                if ( nameField == nullptr || !nameField->isString() )
                    throw ProtocolError(InvalidParams, "tools/call needs a tool name");
                const Json* argumentsField = params.find("arguments");
                Json arguments = argumentsField != nullptr ? *argumentsField : Json::object();
                if ( !arguments.isObject() )
                    throw ProtocolError(InvalidParams, "The tool arguments have to be an object");

                try {
                    return resultResponse(id, textContent(callTool(nameField->string, arguments).dump(), false));
                } catch ( const ServiceError & e ) {
                    return resultResponse(id, textContent(e.what(), true));
                }
            }
            else
                throw ProtocolError(MethodNotFound, "Unknown method \"" + method + "\"");
        } catch ( const ProtocolError & e ) {
            return isNotification ? Json(nullptr) : errorResponse(id, e.code, e.what());
        } catch ( const std::exception & e ) {
            // Anything MappingCore throws while working on a map answers the call rather than ending the server
            return isNotification ? Json(nullptr) : errorResponse(id, InvalidParams, e.what());
        }
    }

    void McpServer::run(std::istream & in, std::ostream & out)
    {
        std::string line {};
        while ( std::getline(in, line) )
        {
            if ( !line.empty() && line.back() == '\r' )
                line.pop_back();
            if ( line.empty() )
                continue;

            Json response {};
            try {
                response = handle(Json::parse(line));
            } catch ( const std::runtime_error & e ) {
                response = errorResponse(Json(nullptr), ParseError, e.what());
            }

            if ( !response.isNull() )
                out << response.dump() << '\n' << std::flush;
        }
    }
}
