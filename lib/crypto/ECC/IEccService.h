#ifndef ECCSERVICE_IECCSERVICE_H
#define ECCSERVICE_IECCSERVICE_H

#include <string>
#include "KeyPair.h"
#include "EncryptedFile.h"

namespace ecc {

class IEccService {
public:
    virtual ~IEccService() = default;

    virtual KeyPair generateKeyPair() = 0;

    virtual KeyPair deriveFromPrivateKey(const std::vector<uint8_t>& privateKey) = 0;

    virtual void encryptFile(const std::string& inputPath,
                             const std::string& outputPath,
                             const std::vector<uint8_t>& recipientPublicKey) = 0;

    virtual void decryptFile(const std::string& inputPath,
                             const std::string& outputPath,
                             const std::vector<uint8_t>& privateKey) = 0;

    virtual EncryptedFile encrypt(const std::vector<uint8_t>& plaintext,
                                  const std::vector<uint8_t>& recipientPublicKey) = 0;

    virtual std::vector<uint8_t> decrypt(const EncryptedFile& encryptedFile,
                                         const std::vector<uint8_t>& privateKey) = 0;

    virtual std::vector<uint8_t> loadPublicKey(const std::string& path) = 0;
    virtual void savePublicKey(const std::string& path,
                               const std::vector<uint8_t>& publicKey) = 0;

    virtual std::vector<uint8_t> loadPrivateKey(const std::string& path) = 0;
    virtual void savePrivateKey(const std::string& path,
                                const std::vector<uint8_t>& privateKey) = 0;
};

} // namespace ecc

#endif // ECCSERVICE_IECCSERVICE_H
