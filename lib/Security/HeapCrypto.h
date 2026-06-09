#ifndef HEAPCRYPTO_H
#define HEAPCRYPTO_H

#include <string>

class HeapCrypto {
private:
    int* memoryBlock; // Pointer to our dynamic heap memory
    int maxCapacity;  // Maximum memory limit
    int currentSize;  // How much memory is actually used

public:
    HeapCrypto(int capacity); // Constructor
    ~HeapCrypto();            // Destructor

    void encryptWholeString(const std::string& password);
    std::string getEncryptedData() const;
};

#endif