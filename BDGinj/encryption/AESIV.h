#include <iostream>
#include <iomanip>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <vector>

#pragma comment(lib, "libcryptoMT.lib")
#pragma comment(lib, "libsslMT.lib")


// Function to handle errors
void handleErrors() {
    std::cerr << "Error occurred in AES decryption" << std::endl;
    logfff(-1, "Error occurred in AES decryption\n");
    ERR_print_errors_fp(stderr);
    exit(1);
}

// AES-128 decryption function
bool AESDecrypt(const std::vector<unsigned char>& cipherText, 
                const std::vector<unsigned char>& key, 
                const std::vector<unsigned char>& iv,
                std::vector<unsigned char>& plainText) {
    AES_KEY decryptKey;

    // Set AES key for decryption
    if (AES_set_decrypt_key(key.data(), 128, &decryptKey) < 0) {
        handleErrors();
        return false;
    }

    // Prepare the output buffer
    plainText.resize(cipherText.size());

    // Perform the AES decryption (CBC mode)
    int numBlocks = cipherText.size() / AES_BLOCK_SIZE;
    unsigned char iv_copy[AES_BLOCK_SIZE];
    std::copy(iv.begin(), iv.end(), iv_copy);

    for (int i = 0; i < numBlocks; ++i) {
        AES_cbc_encrypt(&cipherText[i * AES_BLOCK_SIZE], 
                        &plainText[i * AES_BLOCK_SIZE], 
                        AES_BLOCK_SIZE, 
                        &decryptKey, 
                        iv_copy, 
                        AES_DECRYPT);
    }

    return true;
}

