#ifndef CHKDRAFT_MCP_MCP_SERVER_H
#define CHKDRAFT_MCP_MCP_SERVER_H
#include <iosfwd>
#include <string>
#include "json.h"
#include "map_service.h"

namespace mcp
{
    // The Model Context Protocol over standard input and output: one JSON-RPC message per line in, one per line out.
    // The methods are initialize, ping, tools/list and tools/call; the tools are the map service's operations.
    class McpServer
    {
    public:
        McpServer(MapService & service);

        // Reads messages until input ends
        void run(std::istream & in, std::ostream & out);

        // The response to one message, or null for a notification, which gets none
        Json handle(const Json & message);

        // The tools offered, as tools/list answers them
        Json tools() const;

        // One tool's result, as the content of a tools/call answer
        Json callTool(const std::string & name, const Json & arguments);

    private:
        MapService & service;
    };
}

#endif
