#include "ECC.h"
#include "ECC/EccServiceImpl.h"
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <fstream>

namespace ecc {

std::unique_ptr<IEccService> createEccService()
{
    return std::make_unique<EccServiceImpl>();
}

// ---------------------------------------------------------------------------
// bytesToHex / hexToBytes
// ---------------------------------------------------------------------------
std::string EccCrypto::bytesToHex(const std::vector<uint8_t>& bytes)
{
    std::ostringstream oss;
    for (auto b : bytes)
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return oss.str();
}

std::vector<uint8_t> EccCrypto::hexToBytes(const std::string& hex)
{
    std::vector<uint8_t> bytes(hex.size() / 2);
    for (size_t i = 0; i < bytes.size(); i++)
        std::sscanf(hex.c_str() + i * 2, "%2hhx", &bytes[i]);
    return bytes;
}

// ---------------------------------------------------------------------------
// EccCrypto
// ---------------------------------------------------------------------------
EccCrypto::EccCrypto()
    : service_(createEccService())
{
}

EccCrypto::EccCrypto(const EccConfig& config)
    : service_(createEccService())
    , config_(config)
{
    if (!config.privateKeyPath.empty() && !config.publicKeyPath.empty()) {
        loadKeyPair(config.privateKeyPath, config.publicKeyPath);
    }
}

EccCrypto::~EccCrypto() = default;

KeyPair EccCrypto::generateKeyPair()
{
    auto kp = service_->generateKeyPair();
    storedPublicKey_ = kp.publicKey();
    storedPrivateKey_ = kp.privateKey();
    return kp;
}

void EccCrypto::loadKeyPair(const std::string& privKeyPath, const std::string& pubKeyPath)
{
    storedPublicKey_ = service_->loadPublicKey(pubKeyPath);
    storedPrivateKey_ = service_->loadPrivateKey(privKeyPath);
}

void EccCrypto::saveKeyPair(const std::string& privKeyPath, const std::string& pubKeyPath) const
{
    if (!storedPublicKey_.empty())
        service_->savePublicKey(pubKeyPath, storedPublicKey_);
    if (!storedPrivateKey_.empty())
        service_->savePrivateKey(privKeyPath, storedPrivateKey_);
}

void EccCrypto::setStoredKeyPair(const std::vector<uint8_t>& pubKey,
                                  const std::vector<uint8_t>& privKey)
{
    storedPublicKey_ = pubKey;
    storedPrivateKey_ = privKey;
}

std::vector<uint8_t> EccCrypto::encrypt(const std::vector<uint8_t>& plaintext,
                                         const std::vector<uint8_t>& recipientPublicKey)
{
    EncryptedFile ef = service_->encrypt(plaintext, recipientPublicKey);
    return ef.serialize();
}

std::vector<uint8_t> EccCrypto::decrypt(const std::vector<uint8_t>& ciphertext,
                                         const std::vector<uint8_t>& privateKey)
{
    EncryptedFile ef = EncryptedFile::deserialize(ciphertext);
    return service_->decrypt(ef, privateKey);
}

std::vector<uint8_t> EccCrypto::encryptWithStoredKey(const std::vector<uint8_t>& plaintext)
{
    if (storedPublicKey_.empty())
        throw EccException("No public key loaded. Call generateKeyPair() or loadKeyPair() first.");
    return encrypt(plaintext, storedPublicKey_);
}

std::vector<uint8_t> EccCrypto::decryptWithStoredKey(const std::vector<uint8_t>& ciphertext)
{
    if (storedPrivateKey_.empty())
        throw EccException("No private key loaded. Call generateKeyPair() or loadKeyPair() first.");
    return decrypt(ciphertext, storedPrivateKey_);
}

std::string EccCrypto::encryptDatabase(const std::string& plaintext)
{
    std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
    std::vector<uint8_t> ct = encryptWithStoredKey(pt);
    return bytesToHex(ct);
}

std::string EccCrypto::decryptDatabase(const std::string& ciphertextHex)
{
    std::vector<uint8_t> ct = hexToBytes(ciphertextHex);
    std::vector<uint8_t> pt = decryptWithStoredKey(ct);
    return std::string(pt.begin(), pt.end());
}

void EccCrypto::encryptDatabaseFile(const std::string& dbPath, const std::string& eccPath)
{
    if (storedPublicKey_.empty())
        throw EccException("No public key loaded. Call generateKeyPair() or loadKeyPair() first.");
    service_->encryptFile(dbPath, eccPath, storedPublicKey_);
    std::remove(dbPath.c_str());
}

void EccCrypto::decryptDatabaseFile(const std::string& eccPath, const std::string& dbPath)
{
    if (storedPrivateKey_.empty())
        throw EccException("No private key loaded. Call generateKeyPair() or loadKeyPair() first.");
    service_->decryptFile(eccPath, dbPath, storedPrivateKey_);
    std::remove(eccPath.c_str());
}

} // namespace ecc
