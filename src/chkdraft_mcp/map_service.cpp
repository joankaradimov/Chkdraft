#include "map_service.h"
#include <algorithm>
#include <filesystem>

namespace mcp
{
    // What writing a tile took off the map: doodads whose footprint the tile invalidated, and units left standing on
    // ground they cannot stand on
    struct Removed
    {
        struct Doodad { uint16_t type; uint16_t xc; uint16_t yc; };
        struct Unit { uint16_t type; uint16_t xc; uint16_t yc; uint8_t owner; };
        std::vector<Doodad> doodads {};
        std::vector<Unit> units {};
    };

    // A map file whose save type can be set and whose tiles can be written the way the editor writes them, both of
    // which Scenario keeps behind its edit tracking
    struct ServiceMap : MapFile
    {
        using MapFile::MapFile;

        SaveType getSaveType() const { return read.saveType; }

        void setSaveType(SaveType saveType)
        {
            if ( saveType != read.saveType )
                create_action()->saveType = saveType;
        }

        // Writes the editor's tile (TILE), and the game's (MTXM) unless a doodad stands there legitimately. A doodad
        // the new tile invalidates is taken off with its sprite and its tiles restored from the editor's, and a unit
        // left on ground it cannot stand on is taken off, as the editor does with its default options. This is
        // GuiMap::setTileValue and validateTileOccupiers, which live with the window rather than in Scenario.
        void placeTile(size_t tileX, size_t tileY, uint16_t tileValue, const Sc::Data & data, Removed & removed)
        {
            const auto & tileset = data.terrain.get(read.tileset);
            size_t mapWidth = size_t(read.dimensions.tileWidth);
            auto edit = create_action(ActionDescriptor::UpdateTileValue);
            edit->editorTiles[tileY*mapWidth + tileX] = tileValue;

            bool tileOccupiedByValidDoodad = false;
            for ( int doodadIndex = int(read.doodads.size())-1; doodadIndex >= 0; --doodadIndex )
            {
                const auto & doodad = read.doodads[doodadIndex];
                auto doodadGroupIndex = tileset.getDoodadGroupIndex(doodad.type);
                if ( !doodadGroupIndex )
                    continue;

                const auto & doodadDat = (const Sc::Terrain::DoodadCv5 &)tileset.tileGroups[*doodadGroupIndex];
                const auto & placability = tileset.doodadPlacibility[doodad.type];
                size_t left = (size_t(doodad.xc) - 16*size_t(doodadDat.tileWidth))/32;
                size_t top = (size_t(doodad.yc) - 16*size_t(doodadDat.tileHeight))/32;
                size_t right = left + size_t(doodadDat.tileWidth);
                size_t bottom = top + size_t(doodadDat.tileHeight);
                if ( tileX < left || tileX >= right || tileY < top || tileY >= bottom )
                    continue; // The tile is outside the doodad's footprint

                size_t x = tileX - left;
                size_t y = tileY - top;
                if ( tileset.tileGroups[*doodadGroupIndex + y].megaTileIndex[x] == 0 )
                    continue; // Inside the footprint but not part of the doodad

                auto doodadTilePlacability = placability.tileGroup[y*doodadDat.tileWidth + x];
                if ( doodadTilePlacability != 0 && tileValue/16 != doodadTilePlacability ) // The doodad cannot stand on the new tile
                {
                    for ( int spriteIndex = int(read.sprites.size())-1; spriteIndex >= 0; --spriteIndex )
                    {
                        const auto & sprite = read.sprites[spriteIndex];
                        if ( sprite.type == doodadDat.overlayIndex && sprite.xc == doodad.xc && sprite.yc == doodad.yc )
                            deleteSprite(spriteIndex);
                    }
                    for ( size_t restoreY = top; restoreY < bottom; ++restoreY )
                    {
                        for ( size_t restoreX = left; restoreX < right; ++restoreX )
                            edit->tiles[restoreY*mapWidth + restoreX] = read.editorTiles[restoreY*mapWidth + restoreX];
                    }
                    removed.doodads.push_back({uint16_t(doodad.type), doodad.xc, doodad.yc});
                    deleteDoodad(doodadIndex);
                }
                else
                    tileOccupiedByValidDoodad = true;
            }

            if ( !tileOccupiedByValidDoodad )
                edit->tiles[tileY*mapWidth + tileX] = tileValue;

            for ( int unitIndex = int(read.units.size())-1; unitIndex >= 0; --unitIndex )
            {
                const auto & unit = read.units[unitIndex];
                if ( unit.type >= data.units.numUnitTypes() )
                    continue;

                const auto & unitDat = data.units.getUnit(unit.type);
                bool isBuilding = (unitDat.flags & Sc::Unit::Flags::Building) == Sc::Unit::Flags::Building;
                bool isFlyer = (unitDat.flags & Sc::Unit::Flags::Flyer) == Sc::Unit::Flags::Flyer;
                bool isFlyingBuilding = isBuilding && (unitDat.flags & Sc::Unit::Flags::FlyingBuilding) == Sc::Unit::Flags::FlyingBuilding &&
                    (unit.stateFlags & Chk::Unit::State::InTransit) == Chk::Unit::State::InTransit;
                if ( isFlyer || isFlyingBuilding )
                    continue; // Only what stands on the ground can be left without it

                s32 left = s32(unit.xc) - s32(unitDat.unitSizeLeft);
                s32 right = s32(unit.xc) + s32(unitDat.unitSizeRight);
                s32 top = s32(unit.yc) - s32(unitDat.unitSizeUp);
                s32 bottom = s32(unit.yc) + s32(unitDat.unitSizeDown);
                if ( s32(tileX) < left/32 || s32(tileX) > right/32 || s32(tileY) < top/32 || s32(tileY) > bottom/32 )
                    continue; // The unit does not overlap the tile

                size_t groupIndex = Sc::Terrain::Tiles::getGroupIndex(tileValue);
                if ( groupIndex >= tileset.tileGroups.size() )
                    continue;

                u16 megaTileIndex = tileset.tileGroups[groupIndex].megaTileIndex[Sc::Terrain::Tiles::getGroupMemberIndex(tileValue)];
                if ( megaTileIndex >= tileset.tileFlags.size() )
                    continue;

                // The minitiles of this tile the unit overlaps all have to be walkable
                s32 xTileStart = 32*s32(tileX);
                s32 yTileStart = 32*s32(tileY);
                s32 xMiniTileMin = left < xTileStart ? 0 : (left-xTileStart)/8;
                s32 xMiniTileMax = right >= xTileStart+24 ? 4 : (right-xTileStart)/8+1;
                s32 yMiniTileMin = top < yTileStart ? 0 : (top-yTileStart)/8;
                s32 yMiniTileMax = bottom >= yTileStart+24 ? 4 : (bottom-yTileStart)/8+1;
                bool standing = true;
                for ( s32 yMiniTile = yMiniTileMin; standing && yMiniTile < yMiniTileMax; ++yMiniTile )
                {
                    for ( s32 xMiniTile = xMiniTileMin; xMiniTile < xMiniTileMax; ++xMiniTile )
                    {
                        if ( !tileset.tileFlags[megaTileIndex].miniTileFlags[yMiniTile][xMiniTile].isWalkable() )
                        {
                            standing = false;
                            break;
                        }
                    }
                }
                if ( !standing )
                {
                    removed.units.push_back({uint16_t(unit.type), unit.xc, unit.yc, unit.owner});
                    deleteUnit(unitIndex);
                }
            }
        }
    };

    // The ISOM cache writes the tiles a placement settles on through this; the base cache writes nothing at all
    struct ServiceCache : Chk::IsomCache
    {
        ServiceMap & map;
        const Sc::Data & data;
        Removed removed {};

        ServiceCache(ServiceMap & map, const Sc::Data & data, const Sc::Terrain::Tiles & tiles) :
            Chk::IsomCache(map.getTileset(), map.getTileWidth(), map.getTileHeight(), tiles), map(map), data(data) {}

        inline void setTileValue(size_t tileX, size_t tileY, uint16_t tileValue) final
        {
            map.placeTile(tileX, tileY, tileValue, data, removed);
        }
    };

    struct MapService::OpenMap
    {
        std::string dataKey {};
        std::shared_ptr<Sc::Data> data {};
        std::unique_ptr<ServiceMap> map {};
        std::unique_ptr<ServiceCache> cache {};
        std::string path {};
    };

    const std::vector<std::pair<std::string, SaveType>> MapService::formats {
        { "starcraft_scm", SaveType::StarCraftScm },
        { "hybrid_scm", SaveType::HybridScm },
        { "expansion_scx", SaveType::ExpansionScx },
        { "remastered_scx", SaveType::RemasteredScx },
        { "starcraft_chk", SaveType::StarCraftChk },
        { "hybrid_chk", SaveType::HybridChk },
        { "expansion_chk", SaveType::ExpansionChk },
        { "remastered_chk", SaveType::RemasteredChk }
    };

    static SaveType parseFormat(const std::string & format)
    {
        for ( const auto & [name, saveType] : MapService::formats )
        {
            if ( name == format )
                return saveType;
        }
        std::string known {};
        for ( const auto & [name, saveType] : MapService::formats )
            known += (known.empty() ? "" : ", ") + name;
        throw ServiceError("Unknown format \"" + format + "\"; the formats are " + known);
    }

    static std::string formatName(SaveType saveType)
    {
        for ( const auto & [name, type] : MapService::formats )
        {
            if ( type == saveType )
                return name;
        }
        return "unknown";
    }

    std::string DataFiles::key() const
    {
        std::string key = starCraft;
        for ( const auto & archive : archives )
            key += '\n' + archive;
        return key;
    }

    MapService::MapService() = default;
    MapService::~MapService() = default;

    // Paths arrive from the protocol as UTF-8. libstdc++ reads a narrow path as UTF-8 but libc++ reads it in the ANSI
    // code page, so a path with a character outside ASCII names the wrong file under clang unless it is marked as UTF-8,
    // which is how the mapping core itself hands paths to std::filesystem
    static std::filesystem::path fsPath(const std::string & utf8Path)
    {
        return std::filesystem::path(asUtf8(utf8Path));
    }

    // The descriptors Sc::Data loads from, built the way ChkdDataFileBrowser builds them from a profile: named archives
    // in the order given, earlier ones shadowing later ones, or the StarCraft directory's own files when none are named
    static std::vector<Sc::DataFile::Descriptor> descriptors(const DataFiles & files)
    {
        if ( files.archives.empty() )
            return Sc::DataFile::getDefaultDataFiles(Sc::DataFile::RemasteredDescriptor::Either);

        std::vector<Sc::DataFile::Descriptor> dataFiles {};
        Sc::DataFile::Priority priority = Sc::DataFile::Priority::AutoPriorityStart;
        for ( const auto & path : files.archives )
        {
            bool isDirectory = std::filesystem::is_directory(fsPath(path));
            bool isCasc = isDirectory && CascArchive{}.isValid(path);
            bool isPureDirectory = isDirectory && !isCasc;
            std::string fileName = isCasc ? std::string("Data") : getSystemFileName(path);
            bool isExpectedInScDirectory = false;
            try {
                isExpectedInScDirectory = std::filesystem::equivalent(fsPath(path), fsPath(files.starCraft)) ||
                    std::filesystem::equivalent(fsPath(getSystemFileDirectory(path)), fsPath(files.starCraft));
            } catch ( ... ) {}

            dataFiles.emplace_back(priority, isCasc, isPureDirectory, false, fileName, path, nullptr, isExpectedInScDirectory);
            priority = Sc::DataFile::Priority((u32 &)priority + 1);
        }
        return dataFiles;
    }

    std::shared_ptr<Sc::Data> MapService::data(const DataFiles & files)
    {
        std::string key = files.key();
        auto found = loaded.find(key);
        if ( found != loaded.end() )
            return found->second;

        // Data no open map uses is let go before more is loaded, so switching archive sets does not pile up
        for ( auto entry = loaded.begin(); entry != loaded.end(); )
        {
            bool used = std::any_of(maps.begin(), maps.end(), [&](const auto & map) { return map.second->dataKey == entry->first; });
            entry = used ? std::next(entry) : loaded.erase(entry);
        }

        for ( const auto & archive : files.archives )
        {
            if ( !std::filesystem::exists(fsPath(archive)) )
                throw ServiceError("Data file not found: " + archive);
        }

        auto data = std::make_shared<Sc::Data>();
        // No browser is given for the StarCraft files, so a missing file fails the load rather than asking for it
        if ( !data->load(Sc::DataFile::BrowserPtr(new Sc::DataFile::Browser()), descriptors(files), files.starCraft, nullptr) )
            throw ServiceError("Could not load StarCraft data from " + files.starCraft + (files.archives.empty() ? "" : " and the data files given"));

        loaded[key] = data;
        return data;
    }

    MapService::OpenMap & MapService::openMap(int mapId)
    {
        auto found = maps.find(mapId);
        if ( found == maps.end() )
            throw ServiceError("No open map has id " + std::to_string(mapId));
        return *found->second;
    }

    static Json brushJson(const Sc::Isom::TerrainTypeInfo & brush)
    {
        Json json = Json::object();
        json["terrain_type"] = Json(brush.index);
        json["name"] = Json(std::string(brush.name));
        return json;
    }

    static Json tilesetJson(const Sc::Terrain & terrain, size_t index)
    {
        const auto & tiles = terrain.get(Sc::Terrain::Tileset(index));
        Json json = Json::object();
        json["index"] = Json(index);
        json["name"] = Json(terrain.tilesetNames[index]);
        json["display_name"] = Json(terrain.tilesetDisplayNames[index]);
        json["default_brush"] = Json(tiles.defaultBrush.index);
        Json brushes = Json::array();
        for ( const auto & brush : tiles.brushes )
            brushes.push(brushJson(brush));
        json["brushes"] = brushes;
        return json;
    }

    static Json removedJson(const Removed & removed)
    {
        Json doodads = Json::array();
        for ( const auto & doodad : removed.doodads )
        {
            Json json = Json::object();
            json["type"] = Json(doodad.type);
            json["x"] = Json(doodad.xc);
            json["y"] = Json(doodad.yc);
            doodads.push(json);
        }
        Json units = Json::array();
        for ( const auto & unit : removed.units )
        {
            Json json = Json::object();
            json["type"] = Json(unit.type);
            json["x"] = Json(unit.xc);
            json["y"] = Json(unit.yc);
            json["player"] = Json(unit.owner);
            units.push(json);
        }
        Json json = Json::object();
        json["doodads"] = doodads;
        json["units"] = units;
        return json;
    }

    Json MapService::describe(int mapId, const OpenMap & open) const
    {
        const auto & terrain = open.data->terrain;
        size_t tileset = terrain.indexOf(open.map->getTileset());
        Json json = Json::object();
        json["map"] = Json(mapId);
        json["width"] = Json(open.map->getTileWidth());
        json["height"] = Json(open.map->getTileHeight());
        json["tileset"] = Json(tileset);
        json["tileset_name"] = Json(terrain.tilesetDisplayNames[tileset]);
        json["format"] = Json(formatName(open.map->getSaveType()));
        if ( !open.path.empty() )
            json["path"] = Json(open.path);
        return json;
    }

    Json MapService::listTilesets(const DataFiles & files)
    {
        auto data = this->data(files);
        Json tilesets = Json::array();
        for ( auto index : data->terrain.loadedTilesets() )
            tilesets.push(tilesetJson(data->terrain, index));
        Json json = Json::object();
        json["tilesets"] = tilesets;
        return json;
    }

    Json MapService::newMap(const DataFiles & files, size_t tileset, size_t width, size_t height, std::optional<size_t> terrainType, const std::string & format)
    {
        if ( width == 0 || height == 0 || width > 65535 || height > 65535 )
            throw ServiceError("The map has to be between 1 and 65535 tiles wide and high");

        auto data = this->data(files);
        const auto & terrain = data->terrain;
        if ( terrain.loadedPositionOf(tileset) < 0 )
            throw ServiceError("No tileset is loaded at index " + std::to_string(tileset));

        const auto & tiles = terrain.get(Sc::Terrain::Tileset(tileset));
        size_t initialTerrain = terrainType ? *terrainType : size_t(tiles.defaultBrush.index);
        bool isBrush = std::any_of(tiles.brushes.begin(), tiles.brushes.end(), [&](const auto & brush) { return brush.index == initialTerrain; });
        if ( !isBrush )
            throw ServiceError("Terrain type " + std::to_string(initialTerrain) + " is not a brush of tileset " + std::to_string(tileset));

        SaveType saveType = format.empty() ? SaveType::HybridScm : parseFormat(format);

        auto open = std::make_unique<OpenMap>();
        open->dataKey = files.key();
        open->data = data;
        open->map = std::make_unique<ServiceMap>(Sc::Terrain::Tileset(tileset), u16(width), u16(height), initialTerrain,
            Chk::DefaultTriggers::NoTriggers, saveType, &tiles);
        open->cache = std::make_unique<ServiceCache>(*open->map, *data, tiles);

        int mapId = nextMapId++;
        maps[mapId] = std::move(open);
        return describe(mapId, *maps[mapId]);
    }

    Json MapService::loadMap(const DataFiles & files, const std::string & path)
    {
        if ( !std::filesystem::is_regular_file(fsPath(path)) )
            throw ServiceError("Map file not found: " + path);

        auto data = this->data(files);
        auto map = std::make_unique<ServiceMap>();
        if ( !map->load(path) )
            throw ServiceError("Could not load the map at " + path);

        const auto & tiles = data->terrain.get(map->getTileset());
        auto open = std::make_unique<OpenMap>();
        open->dataKey = files.key();
        open->data = data;
        open->cache = std::make_unique<ServiceCache>(*map, *data, tiles);
        open->map = std::move(map);
        open->path = path;

        int mapId = nextMapId++;
        maps[mapId] = std::move(open);
        return describe(mapId, *maps[mapId]);
    }

    Json MapService::listBrushes(int mapId)
    {
        auto & open = openMap(mapId);
        const auto & tiles = open.data->terrain.get(open.map->getTileset());
        Json brushes = Json::array();
        for ( const auto & brush : tiles.brushes )
            brushes.push(brushJson(brush));
        Json json = Json::object();
        json["map"] = Json(mapId);
        json["default_brush"] = Json(tiles.defaultBrush.index);
        json["brushes"] = brushes;
        return json;
    }

    Json MapService::placeBrush(int mapId, size_t terrainType, size_t tileX, size_t tileY, size_t extent)
    {
        auto & open = openMap(mapId);
        if ( tileX >= open.map->getTileWidth() || tileY >= open.map->getTileHeight() )
            throw ServiceError("Tile (" + std::to_string(tileX) + ", " + std::to_string(tileY) + ") is outside the map");
        if ( extent == 0 )
            throw ServiceError("The brush extent has to be at least 1");

        const auto & tiles = open.data->terrain.get(open.map->getTileset());
        bool isBrush = std::any_of(tiles.brushes.begin(), tiles.brushes.end(), [&](const auto & brush) { return brush.index == terrainType; });
        if ( !isBrush )
            throw ServiceError("Terrain type " + std::to_string(terrainType) + " is not a brush of this map's tileset");

        // The diamond under the middle of the tile, found the way the editor finds the one under the mouse
        auto diamond = Chk::IsomDiamond::fromMapCoordinates(tileX*Sc::Terrain::PixelsPerTile + Sc::Terrain::PixelsPerTile/2,
            tileY*Sc::Terrain::PixelsPerTile + Sc::Terrain::PixelsPerTile/2);
        if ( !open.map->placeIsomTerrain(diamond, terrainType, extent, *open.cache) )
            throw ServiceError("Terrain type " + std::to_string(terrainType) + " could not be placed at tile (" + std::to_string(tileX) + ", " + std::to_string(tileY) + ")");

        auto changed = open.cache->changedArea;
        open.cache->removed = Removed{};
        open.map->updateTilesFromIsom(*open.cache);

        Json json = Json::object();
        json["map"] = Json(mapId);
        json["placed"] = Json(true);
        Json diamondJson = Json::object();
        diamondJson["x"] = Json(diamond.x);
        diamondJson["y"] = Json(diamond.y);
        json["diamond"] = diamondJson;
        Json area = Json::object(); // In ISOM rectangle coordinates, the grid read_isom reads
        area["left"] = Json(changed.left);
        area["top"] = Json(changed.top);
        area["right"] = Json(changed.right);
        area["bottom"] = Json(changed.bottom);
        json["changed_area"] = area;
        json["removed"] = removedJson(open.cache->removed);
        return json;
    }

    Json MapService::placeTile(int mapId, size_t left, size_t top, size_t width, size_t height, size_t tileValue)
    {
        auto & open = openMap(mapId);
        size_t mapWidth = open.map->getTileWidth();
        size_t mapHeight = open.map->getTileHeight();
        if ( left >= mapWidth || top >= mapHeight )
            throw ServiceError("Tile (" + std::to_string(left) + ", " + std::to_string(top) + ") is outside the map");
        if ( width == 0 || height == 0 )
            throw ServiceError("The width and height have to be at least 1");

        const auto & tiles = open.data->terrain.get(open.map->getTileset());
        if ( tileValue/16 >= tiles.tileGroups.size() )
            throw ServiceError("Tile value " + std::to_string(tileValue) + " is past the tileset's " + std::to_string(tiles.tileGroups.size()) + " tile groups");

        size_t right = std::min(mapWidth, left + width);
        size_t bottom = std::min(mapHeight, top + height);
        Removed removed {};
        for ( size_t y=top; y<bottom; ++y )
        {
            for ( size_t x=left; x<right; ++x )
                open.map->placeTile(x, y, uint16_t(tileValue), *open.data, removed);
        }

        Json json = Json::object();
        json["map"] = Json(mapId);
        json["left"] = Json(left);
        json["top"] = Json(top);
        json["width"] = Json(right - left);
        json["height"] = Json(bottom - top);
        json["tile"] = Json(tileValue);
        json["removed"] = removedJson(removed);
        return json;
    }

    Json MapService::readTiles(int mapId, size_t left, size_t top, size_t width, size_t height, const std::string & scope)
    {
        auto & open = openMap(mapId);
        Chk::Scope tileScope = Chk::Scope::Game;
        if ( scope == "editor" )
            tileScope = Chk::Scope::Editor;
        else if ( !scope.empty() && scope != "game" )
            throw ServiceError("The scope is \"game\" or \"editor\"");

        size_t mapWidth = open.map->getTileWidth();
        size_t mapHeight = open.map->getTileHeight();
        size_t right = std::min(mapWidth, left + width);
        size_t bottom = std::min(mapHeight, top + height);

        Json rows = Json::array();
        for ( size_t y=top; y<bottom; ++y )
        {
            Json row = Json::array();
            for ( size_t x=left; x<right; ++x )
                row.push(Json(open.map->getTile(x, y, tileScope)));
            rows.push(row);
        }

        Json json = Json::object();
        json["map"] = Json(mapId);
        json["left"] = Json(left);
        json["top"] = Json(top);
        json["width"] = Json(right > left ? right - left : 0);
        json["height"] = Json(bottom > top ? bottom - top : 0);
        json["scope"] = Json(tileScope == Chk::Scope::Editor ? "editor" : "game");
        json["tiles"] = rows;
        return json;
    }

    Json MapService::readIsom(int mapId, size_t left, size_t top, size_t width, size_t height)
    {
        auto & open = openMap(mapId);
        size_t isomWidth = open.map->getIsomWidth();
        size_t isomHeight = open.map->getIsomHeight();
        size_t right = std::min(isomWidth, left + width);
        size_t bottom = std::min(isomHeight, top + height);

        // Each rectangle holds the ISOM value of the four diamond quadrants that overlap it, without the editor's flags
        Json rows = Json::array();
        for ( size_t y=top; y<bottom; ++y )
        {
            Json row = Json::array();
            for ( size_t x=left; x<right; ++x )
            {
                const auto & rect = open.map->getIsomRect(Chk::IsomRect::Point{x, y});
                Json cell = Json::object();
                cell["left"] = Json(rect.getIsomValue(Sc::Isom::Side::Left));
                cell["top"] = Json(rect.getIsomValue(Sc::Isom::Side::Top));
                cell["right"] = Json(rect.getIsomValue(Sc::Isom::Side::Right));
                cell["bottom"] = Json(rect.getIsomValue(Sc::Isom::Side::Bottom));
                row.push(cell);
            }
            rows.push(row);
        }

        Json json = Json::object();
        json["map"] = Json(mapId);
        json["left"] = Json(left);
        json["top"] = Json(top);
        json["width"] = Json(right > left ? right - left : 0);
        json["height"] = Json(bottom > top ? bottom - top : 0);
        json["isom_width"] = Json(isomWidth);
        json["isom_height"] = Json(isomHeight);
        json["rects"] = rows;
        return json;
    }

    Json MapService::saveMap(int mapId, const std::string & path, const std::string & format)
    {
        auto & open = openMap(mapId);
        if ( path.empty() )
            throw ServiceError("A path to save to is needed");
        if ( open.map->isProtected() )
            throw ServiceError("The map is protected and cannot be saved");
        if ( !format.empty() )
            open.map->setSaveType(parseFormat(format));

        if ( !open.map->save(path, true) )
            throw ServiceError("Could not save the map to " + path);

        open.path = path;
        return describe(mapId, open);
    }

    Json MapService::closeMap(int mapId)
    {
        openMap(mapId);
        maps.erase(mapId);
        Json json = Json::object();
        json["map"] = Json(mapId);
        json["closed"] = Json(true);
        return json;
    }
}
