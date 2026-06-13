#include <iostream>
#include <fstream>
#include "crypto/ECC.h"

int main()
{
    ecc::EccCrypto crypto;
    std::cout << "[1] Generating ECC key pair (P-384)...\n";
    ecc::KeyPair kp = crypto.generateKeyPair();
    std::cout << "  Public key size:  " << kp.publicKey().size() << " bytes\n";
    std::cout << "  Private key size: " << kp.privateKey().size() << " bytes\n";
    std::string pubHex = ecc::EccCrypto::bytesToHex(kp.publicKey());
    std::cout << "  Public key (hex): " << pubHex.substr(0, 64) << "...\n";

    std::cout << "\n[2] Saving keys to disk...\n";
    crypto.saveKeyPair("ecc_private.key", "ecc_public.key");
    std::cout << "  Saved: ecc_private.key, ecc_public.key\n";

    std::cout << "\n[3] Testing database encrypt/decrypt...\n";
    std::string dbRecord = R"({"username":"admin","role":"editor","email":"admin@example.com"})";
    std::cout << "  Original:  " << dbRecord << "\n";
    std::string encryptedHex = crypto.encryptDatabase(dbRecord);
    std::cout << "  Encrypted: " << encryptedHex.substr(0, 64) << "... (" << encryptedHex.size() << " hex chars)\n";
    std::string decrypted = crypto.decryptDatabase(encryptedHex);
    std::cout << "  Decrypted: " << decrypted << "\n";
    bool match = (dbRecord == decrypted);
    std::cout << "  Round-trip: " << (match ? "PASS" : "FAIL") << "\n";

    std::cout << "\n[4] Loading keys into a new instance...\n";
    ecc::EccCrypto crypto2;
    crypto2.loadKeyPair("ecc_private.key", "ecc_public.key");
    std::string dbRecord2 = "user=tester&score=100&level=5";
    std::string enc2 = crypto2.encryptDatabase(dbRecord2);
    std::string dec2 = crypto2.decryptDatabase(enc2);
    std::cout << "  Original:  " << dbRecord2 << "\n";
    std::cout << "  Decrypted: " << dec2 << "\n";
    std::cout << "  Match: " << ((dbRecord2 == dec2) ? "PASS" : "FAIL") << "\n";

    std::cout << "\n[5] Testing binary data encryption...\n";
    std::vector<uint8_t> binaryData = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0x7F, 0x80, 0xAA, 0xBB, 0xCC};
    std::vector<uint8_t> encryptedBin = crypto.encryptWithStoredKey(binaryData);
    std::vector<uint8_t> decryptedBin = crypto.decryptWithStoredKey(encryptedBin);
    bool binMatch = (binaryData == decryptedBin);
    std::cout << "  Binary round-trip: " << (binMatch ? "PASS" : "FAIL") << "\n";

    std::cout << "\n==============================\n";
    std::cout << "  All tests: "
              << (match && binMatch && (dbRecord2 == dec2) ? "PASSED" : "FAILED")
              << "\n";
    std::cout << "==============================\n";

    // Clean up key files
    std::remove("ecc_private.key");
    std::remove("ecc_public.key");

    return (match && binMatch && (dbRecord2 == dec2)) ? 0 : 1;
}
