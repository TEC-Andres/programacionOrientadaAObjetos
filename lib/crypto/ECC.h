#ifndef CRYPTO_ECC_H
#define CRYPTO_ECC_H

#include "ECC/EccServiceFactory.h"
#include "ECC/EccException.h"
#include "ECC/KeyPair.h"
#include "ECC/EncryptedFile.h"
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace ecc {

struct EccConfig {
    std::string privateKeyPath;
    std::string publicKeyPath;
};

class EccCrypto {
public:
    EccCrypto();
    explicit EccCrypto(const EccConfig& config);
    ~EccCrypto();

    KeyPair generateKeyPair();
    void loadKeyPair(const std::string& privKeyPath, const std::string& pubKeyPath);
    void saveKeyPair(const std::string& privKeyPath, const std::string& pubKeyPath) const;
    void setStoredKeyPair(const std::vector<uint8_t>& pubKey,
                          const std::vector<uint8_t>& privKey);

    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plaintext,
                                 const std::vector<uint8_t>& recipientPublicKey);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext,
                                 const std::vector<uint8_t>& privateKey);

    std::vector<uint8_t> encryptWithStoredKey(const std::vector<uint8_t>& plaintext);
    std::vector<uint8_t> decryptWithStoredKey(const std::vector<uint8_t>& ciphertext);

    std::string encryptDatabase(const std::string& plaintext);
    std::string decryptDatabase(const std::string& ciphertextHex);

    void encryptDatabaseFile(const std::string& dbPath, const std::string& eccPath);
    void decryptDatabaseFile(const std::string& eccPath, const std::string& dbPath);

    static std::string bytesToHex(const std::vector<uint8_t>& bytes);
    static std::vector<uint8_t> hexToBytes(const std::string& hex);

private:
    std::unique_ptr<IEccService> service_;
    std::vector<uint8_t> storedPublicKey_;
    std::vector<uint8_t> storedPrivateKey_;
    EccConfig config_;
};

} // namespace ecc

#endif // CRYPTO_ECC_H
