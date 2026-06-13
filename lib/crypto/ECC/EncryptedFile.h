#ifndef ECCSERVICE_ENCRYPTEDFILE_H
#define ECCSERVICE_ENCRYPTEDFILE_H

#include <vector>
#include <cstdint>

namespace ecc {

struct EncryptedFileHeader {
    static constexpr uint32_t MAGIC = 0x45434346; // "ECCF"
    static constexpr uint16_t VERSION = 1;

    uint32_t magic;
    uint16_t version;
    uint16_t curveId;
    uint32_t ephemeralKeyLen;
    uint32_t ivLen;
    uint32_t tagLen;
    uint32_t ciphertextLen;
};

class EncryptedFile {
public:
    EncryptedFile() = default;

    EncryptedFile(std::vector<uint8_t> ephemeralPublicKey,
                  std::vector<uint8_t> iv,
                  std::vector<uint8_t> tag,
                  std::vector<uint8_t> ciphertext)
        : ephemeralPublicKey_(std::move(ephemeralPublicKey))
        , iv_(std::move(iv))
        , tag_(std::move(tag))
        , ciphertext_(std::move(ciphertext))
    {
    }

    const std::vector<uint8_t>& ephemeralPublicKey() const { return ephemeralPublicKey_; }
    const std::vector<uint8_t>& iv() const { return iv_; }
    const std::vector<uint8_t>& tag() const { return tag_; }
    const std::vector<uint8_t>& ciphertext() const { return ciphertext_; }

    bool isValid() const
    {
        return !ephemeralPublicKey_.empty()
            && !iv_.empty()
            && !tag_.empty()
            && !ciphertext_.empty();
    }

    std::vector<uint8_t> serialize() const;
    static EncryptedFile deserialize(const std::vector<uint8_t>& data);

private:
    std::vector<uint8_t> ephemeralPublicKey_;
    std::vector<uint8_t> iv_;
    std::vector<uint8_t> tag_;
    std::vector<uint8_t> ciphertext_;
};

} // namespace ecc

#endif // ECCSERVICE_ENCRYPTEDFILE_H
