#include "EncryptedFile.h"
#include <cstring>
#include <stdexcept>

namespace ecc {

std::vector<uint8_t> EncryptedFile::serialize() const
{
    EncryptedFileHeader header;
    header.magic = EncryptedFileHeader::MAGIC;
    header.version = EncryptedFileHeader::VERSION;
    header.curveId = 0;
    header.ephemeralKeyLen = static_cast<uint32_t>(ephemeralPublicKey_.size());
    header.ivLen = static_cast<uint32_t>(iv_.size());
    header.tagLen = static_cast<uint32_t>(tag_.size());
    header.ciphertextLen = static_cast<uint32_t>(ciphertext_.size());

    std::vector<uint8_t> data(sizeof(header)
        + header.ephemeralKeyLen
        + header.ivLen
        + header.tagLen
        + header.ciphertextLen);

    std::memcpy(data.data(), &header, sizeof(header));

    size_t offset = sizeof(header);
    if (header.ephemeralKeyLen > 0) {
        std::memcpy(data.data() + offset, ephemeralPublicKey_.data(), header.ephemeralKeyLen);
        offset += header.ephemeralKeyLen;
    }
    if (header.ivLen > 0) {
        std::memcpy(data.data() + offset, iv_.data(), header.ivLen);
        offset += header.ivLen;
    }
    if (header.tagLen > 0) {
        std::memcpy(data.data() + offset, tag_.data(), header.tagLen);
        offset += header.tagLen;
    }
    if (header.ciphertextLen > 0) {
        std::memcpy(data.data() + offset, ciphertext_.data(), header.ciphertextLen);
    }

    return data;
}

EncryptedFile EncryptedFile::deserialize(const std::vector<uint8_t>& data)
{
    if (data.size() < sizeof(EncryptedFileHeader)) {
        throw std::runtime_error("EncryptedFile: data too small for header");
    }

    EncryptedFileHeader header;
    std::memcpy(&header, data.data(), sizeof(header));

    if (header.magic != EncryptedFileHeader::MAGIC) {
        throw std::runtime_error("EncryptedFile: invalid magic number");
    }

    size_t offset = sizeof(header);
    if (data.size() < offset + header.ephemeralKeyLen + header.ivLen + header.tagLen + header.ciphertextLen) {
        throw std::runtime_error("EncryptedFile: data truncated");
    }

    std::vector<uint8_t> epk(data.begin() + static_cast<ptrdiff_t>(offset),
                             data.begin() + static_cast<ptrdiff_t>(offset + header.ephemeralKeyLen));
    offset += header.ephemeralKeyLen;

    std::vector<uint8_t> iv(data.begin() + static_cast<ptrdiff_t>(offset),
                            data.begin() + static_cast<ptrdiff_t>(offset + header.ivLen));
    offset += header.ivLen;

    std::vector<uint8_t> tag(data.begin() + static_cast<ptrdiff_t>(offset),
                             data.begin() + static_cast<ptrdiff_t>(offset + header.tagLen));
    offset += header.tagLen;

    std::vector<uint8_t> ct(data.begin() + static_cast<ptrdiff_t>(offset),
                            data.begin() + static_cast<ptrdiff_t>(offset + header.ciphertextLen));

    return EncryptedFile(std::move(epk), std::move(iv), std::move(tag), std::move(ct));
}

} // namespace ecc
