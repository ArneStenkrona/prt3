#ifndef PRT3_FILE_UTIL_H
#define PRT3_FILE_UTIL_H

#include <string>

namespace prt3 {

#ifdef __EMSCRIPTEN__
void emscripten_download_file(
    std::string const & path,
    std::string const & dl_name
);

void emscripten_save_file_via_put(
    std::string const & path
);
#endif // __EMSCRIPTEN__

char const * get_file_extension(char const * filename);

} // namespace prt3

#endif
