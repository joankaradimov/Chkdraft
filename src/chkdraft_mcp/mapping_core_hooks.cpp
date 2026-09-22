// The two hooks the mapping core leaves to whatever links it: Chkdraft defines them in its debug and profile code, the
// mapping core's own tests in basics_test.cpp and test_assets.cpp. The server links neither, so it defines them here.
#include <cstdarg>
#include <cstdio>
#include <optional>
#include <string>
#include <cross_cut/logger.h>
#include <mapping_core/mapping_core.h>

extern Logger logger; // main.cpp defines it

// basics.cpp has an empty version of this, but only where CHKDRAFT is undefined, and src/CMakeLists.txt defines it for
// the whole tree. Standard output carries the protocol, so the error goes to the log on standard error.
void PrintError(const std::string & file, unsigned int line, const std::string msg, ...)
{
    constexpr size_t maxErrorLength = 512;
    char error[maxErrorLength] = "";
    va_list args;
    va_start(args, msg);
    std::vsnprintf(error, maxErrorLength, msg.c_str(), args);
    va_end(args);
    logger.error() << "File: " << file << " | Line: " << line << " | " << error << std::endl;
}

// Nothing is staged for a save: a map the server saves holds what the map itself holds
std::optional<std::string> getPreSavePath()
{
    return std::nullopt;
}
