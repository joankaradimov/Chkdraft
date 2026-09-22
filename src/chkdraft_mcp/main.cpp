#include <iostream>
#include <cross_cut/logger.h>
#include "map_service.h"
#include "mcp_server.h"
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

// MappingCore logs through this. Standard output carries only the protocol, so the log goes to standard error, and
// only what is worth a look there: the tileset slots a tbl leaves empty and the like are below this level.
Logger logger(std::cerr, LogLevel::Warn);

int main()
{
#ifdef _WIN32
    // Lines end in a bare newline both ways, whatever the console would do
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    mcp::MapService service {};
    mcp::McpServer server { service };
    server.run(std::cin, std::cout);
    return 0;
}
