#include "HeapCrypto.h"
#include <stdexcept>
#include <iostream>

HeapCrypto::HeapCrypto(int capacity) {
    maxCapacity = capacity;
    memoryBlock = new int[maxCapacity];
    currentSize = 0;

    for(int i = 0; i < maxCapacity; ++i) {
        memoryBlock[i] = 0;
    }
}

HeapCrypto::~HeapCrypto() {
    delete[] memoryBlock;
}

void HeapCrypto::encryptWholeString(const std::string& password) {

    if (password.length() > maxCapacity) {
        throw std::overflow_error("Heap limit exceeded! Password is too large for allocated block.");
    }

    currentSize = password.length();

    // 1. Calculate the block-modifier based on the WHOLE string
    int blockSum = 0;
    for (char c : password) {
        blockSum += static_cast<int>(c);
    }

    // 2. Encrypt the data into a chaotic, printable ASCII character
    for (int i = 0; i < currentSize; ++i) {
        // Create a chaotic value using the character, the block sum, and its position
        int chaoticValue = static_cast<int>(password[i]) + blockSum + (i * 17);

        // Map it strictly to printable ASCII characters (from 33 '!' to 126 '~')
        int printableAscii = (chaoticValue % 94) + 33;

        memoryBlock[i] = printableAscii;
    }
}

// 3. Output as a solid string
std::string HeapCrypto::getEncryptedData() const {
    std::string result = "";
    for (int i = 0; i < currentSize; ++i) {
        // Cast the integer back to an actual character and append it
        result += static_cast<char>(memoryBlock[i]);
    }
    return result;
}