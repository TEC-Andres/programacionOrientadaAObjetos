#ifndef ECCSERVICE_KEYPAIR_H
#define ECCSERVICE_KEYPAIR_H

#include <vector>
#include <cstdint>

namespace ecc {

class KeyPair {
public:
    KeyPair() = default;

    KeyPair(std::vector<uint8_t> publicKey, std::vector<uint8_t> privateKey)
        : publicKey_(std::move(publicKey))
        , privateKey_(std::move(privateKey))
    {
    }

    const std::vector<uint8_t>& publicKey() const { return publicKey_; }
    const std::vector<uint8_t>& privateKey() const { return privateKey_; }

    bool isValid() const { return !publicKey_.empty() && !privateKey_.empty(); }

private:
    std::vector<uint8_t> publicKey_;
    std::vector<uint8_t> privateKey_;
};

} // namespace ecc

#endif // ECCSERVICE_KEYPAIR_H
