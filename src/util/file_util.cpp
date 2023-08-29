#include "file_util.h"

#include <emscripten.h>

using namespace prt3;

#ifdef __EMSCRIPTEN__
void prt3::emscripten_download_file(
    std::string const & path,
    std::string const & dl_name
) {
    thread_local std::string arg;
    arg.clear();

    arg = "downloadFileFromMemory('"
        + path +
        "','"
        + dl_name +
        "')";

    emscripten_run_script(arg.c_str());
}

void prt3::emscripten_save_file_via_put(
    std::string const & path
) {
    thread_local std::string arg;
    arg.clear();

    arg = "saveFileFromMemoryFSToResourceFolder('"
        + path +
        "','"
        + path +
        "')";

    emscripten_run_script(arg.c_str());
}
#endif // __EMSCRIPTEN__

char const * prt3::get_file_extension(char const * filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}
