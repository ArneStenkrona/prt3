#ifndef PRT3_CHECKSUM_H
#define PRT3_CHECKSUM_H

#include "src/util/fixed_string.h"

#include "md5.h"


namespace prt3 {

typedef FixedString<2 * MD5::HashBytes + 1> MD5String;

MD5String compute_md5(char const * path);

} // namespace prt3

#endif
