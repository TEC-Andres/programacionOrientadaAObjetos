#ifndef ECCSERVICE_ECCSERVICEIMPL_H
#define ECCSERVICE_ECCSERVICEIMPL_H

#include "IEccService.h"

namespace ecc {

class EccServiceImpl : public IEccService {
public:
    EccServiceImpl();
    ~EccServiceImpl() override;

    KeyPair generateKeyPair() override;

    KeyPair deriveFromPrivateKey(const std::vector<uint8_t>& privateKey) override;

    void encryptFile(const std::string& inputPath,
                     const std::string& outputPath,
                     const std::vector<uint8_t>& recipientPublicKey) override;

    void decryptFile(const std::string& inputPath,
                     const std::string& outputPath,
                     const std::vector<uint8_t>& privateKey) override;

    EncryptedFile encrypt(const std::vector<uint8_t>& plaintext,
                          const std::vector<uint8_t>& recipientPublicKey) override;

    std::vector<uint8_t> decrypt(const EncryptedFile& encryptedFile,
                                 const std::vector<uint8_t>& privateKey) override;

    std::vector<uint8_t> loadPublicKey(const std::string& path) override;
    void savePublicKey(const std::string& path,
                       const std::vector<uint8_t>& publicKey) override;

    std::vector<uint8_t> loadPrivateKey(const std::string& path) override;
    void savePrivateKey(const std::string& path,
                        const std::vector<uint8_t>& privateKey) override;
};

} // namespace ecc

#endif // ECCSERVICE_ECCSERVICEIMPL_H
