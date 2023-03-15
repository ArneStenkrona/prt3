#include "checksum.h"

#include <iostream>
#include <fstream>
#include <array>
#include <cstdio>

using namespace prt3;

MD5String prt3::compute_md5(char const * path) {
    MD5 digest_md5;

    std::FILE * file;
    file = std::fopen(path, "rb");

    if (!file)
    {
        std::cerr << "prt3::compute_md5: Can't open '" << path << "'" << std::endl;
        return "";
    }

    // each cycle processes about 1 MByte (divisible by 144 => improves Keccak/SHA3 performance)
    constexpr size_t buffer_size = 144 * 7 * 1024;
    std::array<char, buffer_size> buffer;
    // process file
    std::size_t num_bytes_read;
    while ((num_bytes_read = std::fread(buffer.data(), 1, buffer_size, file)) > 0) {
        digest_md5.add(buffer.data(), num_bytes_read);
    }
    // clean up
    std::fclose(file);
    // show results
    return MD5String{digest_md5.getHash().c_str()};
}

CRC32String prt3::compute_crc32(char const * path) {
    CRC32 digest_crc32;

    std::FILE * file;
    file = std::fopen(path, "rb");

    if (!file)
    {
        std::cerr << "prt3::compute_crc32: Can't open '" << path << "'" << std::endl;
        return "";
    }

    // each cycle processes about 1 MByte (divisible by 144 => improves Keccak/SHA3 performance)
    constexpr size_t buffer_size = 144 * 7 * 1024;
    std::array<char, buffer_size> buffer;
    // process file
    std::size_t num_bytes_read;
    while ((num_bytes_read = std::fread(buffer.data(), 1, buffer_size, file)) > 0) {
        digest_crc32.add(buffer.data(), num_bytes_read);
    }
    std::fclose(file);
    // show results
    return CRC32String{digest_crc32.getHash().c_str()};
}
