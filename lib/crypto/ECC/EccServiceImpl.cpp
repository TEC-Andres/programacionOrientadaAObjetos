#include "EccServiceImpl.h"
#include "EccException.h"
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <cstring>

namespace ecc {

namespace {

constexpr int EC_CURVE_NID = NID_secp384r1;
constexpr size_t EC_PUBKEY_SIZE = 97;   // 04 + 48 + 48 for P-384 uncompressed
constexpr size_t EC_PRIVKEY_SIZE = 48;
constexpr size_t AES_KEY_SIZE = 32;     // AES-256
constexpr size_t GCM_IV_SIZE = 12;      // 96-bit IV for GCM
constexpr size_t GCM_TAG_SIZE = 16;

std::vector<uint8_t> i2dPkey(EVP_PKEY* pkey)
{
    int len = i2d_PUBKEY(pkey, nullptr);
    if (len <= 0)
        throw EccException("Failed to encode public key");
    std::vector<uint8_t> buf(static_cast<size_t>(len));
    unsigned char* ptr = buf.data();
    if (i2d_PUBKEY(pkey, &ptr) <= 0)
        throw EccException("Failed to encode public key");
    return buf;
}

std::vector<uint8_t> i2dPrivateKey(EVP_PKEY* pkey)
{
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) throw EccException("BIO_new failed");
    if (i2d_PKCS8PrivateKey_bio(bio, pkey, nullptr, nullptr, 0, nullptr, nullptr) == 0) {
        BIO_free(bio);
        throw EccException("Failed to encode private key");
    }
    unsigned char* data = nullptr;
    long dataLen = BIO_get_mem_data(bio, &data);
    std::vector<uint8_t> result(data, data + dataLen);
    BIO_free(bio);
    return result;
}

EVP_PKEY* d2iPubKey(const std::vector<uint8_t>& data)
{
    const unsigned char* ptr = data.data();
    EVP_PKEY* pkey = d2i_PUBKEY(nullptr, &ptr, static_cast<long>(data.size()));
    if (!pkey)
        throw EccException("Failed to decode public key");
    return pkey;
}

EVP_PKEY* d2iPrivKey(const std::vector<uint8_t>& data)
{
    const unsigned char* ptr = data.data();
    EVP_PKEY* pkey = d2i_AutoPrivateKey(nullptr, &ptr, static_cast<long>(data.size()));
    if (!pkey)
        throw EccException("Failed to decode private key");
    return pkey;
}

std::vector<uint8_t> kdfSha256(const std::vector<uint8_t>& sharedSecret, size_t keyLen)
{
    std::vector<uint8_t> key(keyLen);
    EVP_KDF* kdf = EVP_KDF_fetch(nullptr, "HKDF", nullptr);
    if (!kdf)
        throw EccException("Failed to fetch HKDF");

    EVP_KDF_CTX* kctx = EVP_KDF_CTX_new(kdf);
    EVP_KDF_free(kdf);
    if (!kctx)
        throw EccException("Failed to create KDF context");

    OSSL_PARAM params[5];
    std::string digest = "SHA-256";
    const unsigned char salt[] = "ECC-DB-ENC";
    params[0] = OSSL_PARAM_construct_utf8_string("digest", digest.data(), 0);
    params[1] = OSSL_PARAM_construct_octet_string("salt", const_cast<unsigned char*>(salt), sizeof(salt) - 1);
    params[2] = OSSL_PARAM_construct_octet_string("key", const_cast<uint8_t*>(sharedSecret.data()), sharedSecret.size());
    params[3] = OSSL_PARAM_construct_octet_string("info", const_cast<unsigned char*>(
        reinterpret_cast<const unsigned char*>("AES-256-GCM")), 11);
    params[4] = OSSL_PARAM_construct_end();

    if (EVP_KDF_derive(kctx, key.data(), key.size(), params) <= 0) {
        EVP_KDF_CTX_free(kctx);
        throw EccException("HKDF derivation failed");
    }

    EVP_KDF_CTX_free(kctx);
    return key;
}

} // anonymous namespace

EccServiceImpl::EccServiceImpl() = default;
EccServiceImpl::~EccServiceImpl() = default;

KeyPair EccServiceImpl::generateKeyPair()
{
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if (!ctx) throw EccException("Failed to create PKEY context");

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw EccException("Keygen init failed");
    }
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, EC_CURVE_NID) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw EccException("Failed to set curve");
    }

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw EccException("Key generation failed");
    }
    EVP_PKEY_CTX_free(ctx);

    std::vector<uint8_t> pubKey = i2dPkey(pkey);
    std::vector<uint8_t> privKey = i2dPrivateKey(pkey);
    EVP_PKEY_free(pkey);

    return KeyPair(std::move(pubKey), std::move(privKey));
}

KeyPair EccServiceImpl::deriveFromPrivateKey(const std::vector<uint8_t>& privateKey)
{
    EVP_PKEY* pkey = d2iPrivKey(privateKey);
    if (!pkey)
        throw EccException("Failed to import private key");

    std::vector<uint8_t> pubKey = i2dPkey(pkey);
    std::vector<uint8_t> privKeyBytes(privateKey);
    EVP_PKEY_free(pkey);

    return KeyPair(std::move(pubKey), std::move(privKeyBytes));
}

EncryptedFile EccServiceImpl::encrypt(const std::vector<uint8_t>& plaintext,
                                       const std::vector<uint8_t>& recipientPublicKey)
{
    EVP_PKEY* recipKey = d2iPubKey(recipientPublicKey);

    // Generate ephemeral key pair
    EVP_PKEY_CTX* genCtx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if (!genCtx) { EVP_PKEY_free(recipKey); throw EccException("Failed to create gen context"); }

    if (EVP_PKEY_keygen_init(genCtx) <= 0) {
        EVP_PKEY_CTX_free(genCtx); EVP_PKEY_free(recipKey);
        throw EccException("Keygen init failed");
    }
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(genCtx, EC_CURVE_NID) <= 0) {
        EVP_PKEY_CTX_free(genCtx); EVP_PKEY_free(recipKey);
        throw EccException("Failed to set curve");
    }

    EVP_PKEY* ephemeralKey = nullptr;
    if (EVP_PKEY_keygen(genCtx, &ephemeralKey) <= 0) {
        EVP_PKEY_CTX_free(genCtx); EVP_PKEY_free(recipKey);
        throw EccException("Ephemeral key generation failed");
    }
    EVP_PKEY_CTX_free(genCtx);

    // ECDH key agreement
    EVP_PKEY_CTX* deriveCtx = EVP_PKEY_CTX_new(ephemeralKey, nullptr);
    if (!deriveCtx) {
        EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("Failed to create derive context");
    }

    if (EVP_PKEY_derive_init(deriveCtx) <= 0) {
        EVP_PKEY_CTX_free(deriveCtx); EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("Derive init failed");
    }
    if (EVP_PKEY_derive_set_peer(deriveCtx, recipKey) <= 0) {
        EVP_PKEY_CTX_free(deriveCtx); EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("Derive set peer failed");
    }

    size_t secretLen = 0;
    if (EVP_PKEY_derive(deriveCtx, nullptr, &secretLen) <= 0) {
        EVP_PKEY_CTX_free(deriveCtx); EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("Derive get length failed");
    }

    std::vector<uint8_t> sharedSecret(secretLen);
    if (EVP_PKEY_derive(deriveCtx, sharedSecret.data(), &secretLen) <= 0) {
        EVP_PKEY_CTX_free(deriveCtx); EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("ECDH key agreement failed");
    }
    sharedSecret.resize(secretLen);
    EVP_PKEY_CTX_free(deriveCtx);

    // Derive AES key using HKDF
    std::vector<uint8_t> aesKey = kdfSha256(sharedSecret, AES_KEY_SIZE);

    // Get ephemeral public key bytes
    std::vector<uint8_t> ephemPubKey = i2dPkey(ephemeralKey);

    // AES-256-GCM encryption
    EVP_CIPHER_CTX* cipherCtx = EVP_CIPHER_CTX_new();
    if (!cipherCtx) {
        EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("Failed to create cipher context");
    }

    if (EVP_EncryptInit_ex(cipherCtx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        EVP_CIPHER_CTX_free(cipherCtx); EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("Encrypt init failed");
    }

    // Generate random IV
    std::vector<uint8_t> iv(GCM_IV_SIZE);
    if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
        EVP_CIPHER_CTX_free(cipherCtx); EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("Failed to generate IV");
    }

    if (EVP_EncryptInit_ex(cipherCtx, nullptr, nullptr, aesKey.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(cipherCtx); EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("Encrypt init with key failed");
    }

    std::vector<uint8_t> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
    int outLen = 0;
    if (EVP_EncryptUpdate(cipherCtx, ciphertext.data(), &outLen,
                          plaintext.data(), static_cast<int>(plaintext.size())) != 1) {
        EVP_CIPHER_CTX_free(cipherCtx); EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("Encrypt update failed");
    }
    int totalLen = outLen;

    if (EVP_EncryptFinal_ex(cipherCtx, ciphertext.data() + totalLen, &outLen) != 1) {
        EVP_CIPHER_CTX_free(cipherCtx); EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("Encrypt final failed");
    }
    totalLen += outLen;
    ciphertext.resize(static_cast<size_t>(totalLen));

    std::vector<uint8_t> tag(GCM_TAG_SIZE);
    if (EVP_CIPHER_CTX_ctrl(cipherCtx, EVP_CTRL_GCM_GET_TAG, static_cast<int>(tag.size()), tag.data()) != 1) {
        EVP_CIPHER_CTX_free(cipherCtx); EVP_PKEY_free(ephemeralKey); EVP_PKEY_free(recipKey);
        throw EccException("Failed to get GCM tag");
    }

    EVP_CIPHER_CTX_free(cipherCtx);
    EVP_PKEY_free(ephemeralKey);
    EVP_PKEY_free(recipKey);

    return EncryptedFile(std::move(ephemPubKey), std::move(iv), std::move(tag), std::move(ciphertext));
}

std::vector<uint8_t> EccServiceImpl::decrypt(const EncryptedFile& encryptedFile,
                                              const std::vector<uint8_t>& privateKey)
{
    EVP_PKEY* privPkey = d2iPrivKey(privateKey);
    EVP_PKEY* ephemPubKey = d2iPubKey(encryptedFile.ephemeralPublicKey());

    // ECDH key agreement
    EVP_PKEY_CTX* deriveCtx = EVP_PKEY_CTX_new(privPkey, nullptr);
    if (!deriveCtx) {
        EVP_PKEY_free(privPkey); EVP_PKEY_free(ephemPubKey);
        throw EccException("Failed to create derive context");
    }

    if (EVP_PKEY_derive_init(deriveCtx) <= 0) {
        EVP_PKEY_CTX_free(deriveCtx); EVP_PKEY_free(privPkey); EVP_PKEY_free(ephemPubKey);
        throw EccException("Derive init failed");
    }
    if (EVP_PKEY_derive_set_peer(deriveCtx, ephemPubKey) <= 0) {
        EVP_PKEY_CTX_free(deriveCtx); EVP_PKEY_free(privPkey); EVP_PKEY_free(ephemPubKey);
        throw EccException("Derive set peer failed");
    }

    size_t secretLen = 0;
    if (EVP_PKEY_derive(deriveCtx, nullptr, &secretLen) <= 0) {
        EVP_PKEY_CTX_free(deriveCtx); EVP_PKEY_free(privPkey); EVP_PKEY_free(ephemPubKey);
        throw EccException("Derive get length failed");
    }

    std::vector<uint8_t> sharedSecret(secretLen);
    if (EVP_PKEY_derive(deriveCtx, sharedSecret.data(), &secretLen) <= 0) {
        EVP_PKEY_CTX_free(deriveCtx); EVP_PKEY_free(privPkey); EVP_PKEY_free(ephemPubKey);
        throw EccException("ECDH key agreement failed");
    }
    sharedSecret.resize(secretLen);
    EVP_PKEY_CTX_free(deriveCtx);

    // Derive AES key
    std::vector<uint8_t> aesKey = kdfSha256(sharedSecret, AES_KEY_SIZE);

    // AES-256-GCM decryption
    EVP_CIPHER_CTX* cipherCtx = EVP_CIPHER_CTX_new();
    if (!cipherCtx) {
        EVP_PKEY_free(privPkey); EVP_PKEY_free(ephemPubKey);
        throw EccException("Failed to create cipher context");
    }

    if (EVP_DecryptInit_ex(cipherCtx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        EVP_CIPHER_CTX_free(cipherCtx); EVP_PKEY_free(privPkey); EVP_PKEY_free(ephemPubKey);
        throw EccException("Decrypt init failed");
    }

    if (EVP_DecryptInit_ex(cipherCtx, nullptr, nullptr, aesKey.data(), encryptedFile.iv().data()) != 1) {
        EVP_CIPHER_CTX_free(cipherCtx); EVP_PKEY_free(privPkey); EVP_PKEY_free(ephemPubKey);
        throw EccException("Decrypt init with key failed");
    }

    std::vector<uint8_t> plaintext(encryptedFile.ciphertext().size() + EVP_MAX_BLOCK_LENGTH);
    int outLen = 0;
    if (EVP_DecryptUpdate(cipherCtx, plaintext.data(), &outLen,
                          encryptedFile.ciphertext().data(),
                          static_cast<int>(encryptedFile.ciphertext().size())) != 1) {
        EVP_CIPHER_CTX_free(cipherCtx); EVP_PKEY_free(privPkey); EVP_PKEY_free(ephemPubKey);
        throw EccException("Decrypt update failed");
    }
    int totalLen = outLen;

    if (EVP_CIPHER_CTX_ctrl(cipherCtx, EVP_CTRL_GCM_SET_TAG, static_cast<int>(encryptedFile.tag().size()),
                            const_cast<uint8_t*>(encryptedFile.tag().data())) != 1) {
        EVP_CIPHER_CTX_free(cipherCtx); EVP_PKEY_free(privPkey); EVP_PKEY_free(ephemPubKey);
        throw EccException("Failed to set GCM tag");
    }

    if (EVP_DecryptFinal_ex(cipherCtx, plaintext.data() + totalLen, &outLen) != 1) {
        EVP_CIPHER_CTX_free(cipherCtx); EVP_PKEY_free(privPkey); EVP_PKEY_free(ephemPubKey);
        throw EccException("Decrypt final failed - data may be tampered");
    }
    totalLen += outLen;
    plaintext.resize(static_cast<size_t>(totalLen));

    EVP_CIPHER_CTX_free(cipherCtx);
    EVP_PKEY_free(privPkey);
    EVP_PKEY_free(ephemPubKey);

    return plaintext;
}

void EccServiceImpl::encryptFile(const std::string& inputPath,
                                  const std::string& outputPath,
                                  const std::vector<uint8_t>& recipientPublicKey)
{
    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile)
        throw EccException("Cannot open input file: " + inputPath);

    std::vector<uint8_t> plaintext((std::istreambuf_iterator<char>(inFile)),
                                    std::istreambuf_iterator<char>());
    inFile.close();

    EncryptedFile encrypted = encrypt(plaintext, recipientPublicKey);
    std::vector<uint8_t> serialized = encrypted.serialize();

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile)
        throw EccException("Cannot open output file: " + outputPath);

    outFile.write(reinterpret_cast<const char*>(serialized.data()),
                  static_cast<std::streamsize>(serialized.size()));
    outFile.close();
}

void EccServiceImpl::decryptFile(const std::string& inputPath,
                                  const std::string& outputPath,
                                  const std::vector<uint8_t>& privateKey)
{
    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile)
        throw EccException("Cannot open input file: " + inputPath);

    std::vector<uint8_t> serialized((std::istreambuf_iterator<char>(inFile)),
                                     std::istreambuf_iterator<char>());
    inFile.close();

    EncryptedFile encrypted = EncryptedFile::deserialize(serialized);
    std::vector<uint8_t> plaintext = decrypt(encrypted, privateKey);

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile)
        throw EccException("Cannot open output file: " + outputPath);

    outFile.write(reinterpret_cast<const char*>(plaintext.data()),
                  static_cast<std::streamsize>(plaintext.size()));
    outFile.close();
}

std::vector<uint8_t> EccServiceImpl::loadPublicKey(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
        throw EccException("Cannot open public key file: " + path);

    std::string hexStr;
    file >> hexStr;
    file.close();

    if (hexStr.empty())
        throw EccException("Public key file is empty");

    std::vector<uint8_t> key(hexStr.size() / 2);
    for (size_t i = 0; i < key.size(); i++)
        std::sscanf(hexStr.c_str() + i * 2, "%2hhx", &key[i]);
    return key;
}

void EccServiceImpl::savePublicKey(const std::string& path,
                                    const std::vector<uint8_t>& publicKey)
{
    std::ofstream file(path);
    if (!file)
        throw EccException("Cannot open public key file for writing: " + path);

    for (auto b : publicKey)
        file << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    file << "\n";
    file.close();
}

std::vector<uint8_t> EccServiceImpl::loadPrivateKey(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
        throw EccException("Cannot open private key file: " + path);

    std::string hexStr;
    file >> hexStr;
    file.close();

    if (hexStr.empty())
        throw EccException("Private key file is empty");

    std::vector<uint8_t> key(hexStr.size() / 2);
    for (size_t i = 0; i < key.size(); i++)
        std::sscanf(hexStr.c_str() + i * 2, "%2hhx", &key[i]);
    return key;
}

void EccServiceImpl::savePrivateKey(const std::string& path,
                                     const std::vector<uint8_t>& privateKey)
{
    std::ofstream file(path);
    if (!file)
        throw EccException("Cannot open private key file for writing: " + path);

    for (auto b : privateKey)
        file << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    file << "\n";
    file.close();
}

} // namespace ecc
