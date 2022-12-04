#ifndef PRT3_FILE_UTIL_H
#define PRT3_FILE_UTIL_H

#include <string>

namespace prt3 {

void emscripten_download_file(
    std::string const & path,
    std::string const & dl_name
);

void emscripten_save_file_via_put(
    std::string const & path
);

} // namespace prt3

#endif
