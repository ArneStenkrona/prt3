#include "file_util.h"

#include <emscripten.h>

using namespace prt3;

void prt3::emscripten_download_file(
    std::string const & path,
    std::string const & dl_name
) {
    thread_local std::string arg;
    arg.clear();

    arg = "saveFileFromMemoryFSToDisk('"
        + path +
        "','"
        + dl_name +
        "')";

    emscripten_run_script(arg.c_str());
}
