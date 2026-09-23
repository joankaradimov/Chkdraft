#ifndef CHKDRAFT_MCP_MAP_SERVICE_H
#define CHKDRAFT_MCP_MAP_SERVICE_H
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <mapping_core/mapping_core.h>
#include "json.h"

namespace mcp
{
    // A problem with a request that the caller should hear about: a bad argument, a map that is not open, a file that
    // could not be read or written. The message is what the tool answers with.
    struct ServiceError : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    // The game data a map is opened against: the StarCraft directory, and the archives and folders laid over its own,
    // first over last, the way a Chkdraft profile lists them. With no archives named, the directory's own StarDat,
    // BrooDat, patch_rt and Remastered Data are used.
    struct DataFiles
    {
        std::string starCraft {};
        std::vector<std::string> archives {};

        std::string key() const; // The same files in the same order give the same key
    };

    // The maps the server has open and the game data they were opened against. Game data is loaded once per distinct
    // set of files and kept while a map uses it, since a map's ISOM cache points into its tileset.
    class MapService
    {
    public:
        MapService();
        ~MapService();

        Json listTilesets(const DataFiles & files);
        Json newMap(const DataFiles & files, size_t tileset, size_t width, size_t height, std::optional<size_t> terrainType, const std::string & format);
        Json loadMap(const DataFiles & files, const std::string & path);
        Json listBrushes(int mapId);
        Json placeBrush(int mapId, size_t terrainType, size_t tileX, size_t tileY, size_t extent);
        Json placeTile(int mapId, size_t left, size_t top, size_t width, size_t height, size_t tileValue);
        Json readTiles(int mapId, size_t left, size_t top, size_t width, size_t height, const std::string & scope);
        Json readIsom(int mapId, size_t left, size_t top, size_t width, size_t height);
        Json saveMap(int mapId, const std::string & path, const std::string & format);
        Json closeMap(int mapId);

        // The formats a map can be saved as, by the names the tools take
        static const std::vector<std::pair<std::string, SaveType>> formats;

    private:
        struct OpenMap; // Defined in the .cpp, which is also where the constructor and destructor are, so the map of
                        // unique pointers to it is only ever destroyed where the type is complete

        std::map<std::string, std::shared_ptr<Sc::Data>> loaded;
        std::map<int, std::unique_ptr<OpenMap>> maps;
        int nextMapId = 1;

        std::shared_ptr<Sc::Data> data(const DataFiles & files);
        OpenMap & openMap(int mapId);
        Json describe(int mapId, const OpenMap & open) const;
    };
}

#endif
