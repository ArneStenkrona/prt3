#include "checksum.h"

#include <iostream>
#include <fstream>
#include <array>

using namespace prt3;

MD5String prt3::compute_md5(char const * path) {
    MD5 digestMd5;

    std::ifstream file;
    std::istream* input = nullptr;

    // open file
    file.open(path, std::ios::in | std::ios::binary);
    if (!file)
    {
        std::cerr << "prt3::compute_md5: Can't open '" << path << "'" << std::endl;
        return "";
    }

    input = &file;
    // each cycle processes about 1 MByte (divisible by 144 => improves Keccak/SHA3 performance)
    constexpr size_t BufferSize = 144 * 7 * 1024;
    std::array<char, BufferSize> buffer;
    // process file
    while (*input) {
        input->read(buffer.data(), BufferSize);
        std::size_t numBytesRead = size_t(input->gcount());
        digestMd5.add(buffer.data(), numBytesRead);
    }
    // clean up
    file.close();
    // show results
    return MD5String{digestMd5.getHash().c_str()};
}

CRC32String prt3::compute_crc32(char const * path) {
    CRC32 digestCRC32;

    std::ifstream file;
    std::istream* input = nullptr;

    // open file
    file.open(path, std::ios::in | std::ios::binary);
    if (!file)
    {
        std::cerr << "prt3::compute_md5: Can't open '" << path << "'" << std::endl;
        return "";
    }

    input = &file;
    // each cycle processes about 1 MByte (divisible by 144 => improves Keccak/SHA3 performance)
    constexpr size_t BufferSize = 144 * 7 * 1024;
    std::array<char, BufferSize> buffer;
    // process file
    while (*input) {
        input->read(buffer.data(), BufferSize);
        std::size_t numBytesRead = size_t(input->gcount());
        digestCRC32.add(buffer.data(), numBytesRead);
    }
    // clean up
    file.close();
    // show results
    return CRC32String{digestCRC32.getHash().c_str()};
}
