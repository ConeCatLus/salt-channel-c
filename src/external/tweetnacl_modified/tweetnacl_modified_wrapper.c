/**
 * @file salt_crypto_wrapper.c
 *
 * Salt crypto wrapper for modified version of TweetNaCl.
 *
 */

/*======= Includes ==========================================================*/

#include <stddef.h>
#include "salt_crypto_wrapper.h"
#include "tweetnacl_modified.h"

/*======= Local Macro Definitions ===========================================*/
/*======= Type Definitions ==================================================*/
/*======= Local function prototypes =========================================*/
/*======= Local variable declarations =======================================*/
/*======= Global function implementations ===================================*/

/**
 * @brief Randomly generates a secret- and public key for encryption.
 * 
 * Example usage:
 *  unsigned char public_key[api_crypto_box_PUBLICKEYBYTES];
 *  unsigned char secret_key[api_crypto_box_SECRETKEYBYTES];
 * 
 *  api_crypto_box_keypair(public_key, secret_key);
 * 
 * @param public_key Pointer where to store public key.
 * @param secret_key Pointer where to store secret key.
 * 
 * @return 0    The keypair was successfully generated.
 * @return != 0 The keypair could not be generated.
 */
int api_crypto_box_keypair(unsigned char *public_key,
                           unsigned char *secret_key)
{
    return crypto_box_keypair(public_key, secret_key);
}

/**
 * @brief Calculates a symmetric encryption key based on a public
 *        and a private key. Precalculation interface used when comminucating
 *        with a peer to gain speed. "beforenm" means calc. of sym.key is done before
 *        adding nonce and message.
 * 
 * Example usage:
 *  // Common symmetric key that can be used to communicate.
 *  unsigned char symmetric_key[api_crypto_box_BEFORENMBYTES];
 *  // Public key received from other part.
 *  unsigned char public_key[api_crypto_box_PUBLICKEYBYTES];
 *  // Secret (private) key generated by this device
 *  unsigned char secret_key[api_crypto_box_SECRETKEYBYTES];
 * 
 *  api_crypto_box_beforenm(symmetric_key, public_key, secret_key);
 * 
 * @param symmetric_key Pointer to symmetric key, 
 * @param public_key    Pointer to public key.
 * @param secret_key    Pointer to secret key.
 * 
 * @return 0    The symmetric key was successfully calculated.
 * @return != 0 The symmetric key could not be calculated.
 */
int api_crypto_box_beforenm(unsigned char *symmetric_key,
                            const unsigned char *public_key,
                            const unsigned char *secret_key)
{
    return crypto_box_beforenm(symmetric_key, public_key, secret_key);
}

/**
 * @brief Encrypt and authenticate a message using a symmetric
 *        encryption key. "afternm" means encrytion is done after adding nonce and message.
 *         
 * Example usage:
 *  unsigned long long length = n;
 *  unsigned char cipher[n + api_crypto_secretbox_BOXZEROBYTES];
 *  const unsigned char clear_text[n];
 *  const unsigned char nonce[api_crypto_secretbox_NONCEBYTES];
 *  const unsigned char symmetric_key[api_crypto_box_PUBLICKEYBYTES];
 *  
 *  // Clear text (message) structure:
 *  // clear_text = {
 *  //     zeroPadded[api_crypto_secretbox_ZEROBYTES] ||
 *  //     clearText[n - api_crypto_secretbox_ZEROBYTES]
 *  // }
 *  // i.e. zeros[32] + clear_text[n]
 *  
 *  crypto_box_afternm(cipher, clear_text, length, nonce, symmetric_key);
 *  
 *  // Cipher message structure after crypto_box_afternm:
 *  // cipher = {
 *  //     zeroPadded[api_crypto_secretbox_BOXZEROBYTES] ||
 *  //     authenticatedEncrypted[n + api_crypto_secretbox_BOXZEROBYTES]
 *  // }
 *  // i.e. zeros[16] + MAC[16] + cipher[n]
 * 
 * Note: The selected implementation MUST allow for in-place operation. I.e.,
 *       the cipher and clear_text pointer points to the same memory address.
 *       cipher and clear_text will use the same amount of memory.
 * 
 * @param cipher        Pointer to authenticated cipher text. The first api_crypto_secretbox_BOXZEROBYTES
 *                      bytes will be zero padded. Hence, the authenticated cipher length will be
 *                      length + api_crypto_secretbox_BOXZEROBYTES.
 *                      
 * @param clear_text    Pointer to clear text message.
 *                      Note: The first api_crypto_secretbox_ZEROBYTES bytes must be zero padded
 *                      prior to the call.
 *                      
 * @param length        Length of clear text message, exluding api_crypto_secretbox_ZEROBYTES
 *                      bytes zero padding.
 *                      
 * @param nonce         Nonce, api_crypto_secretbox_NONCEBYTES bytes long.
 *                      Note: The nonce MUST only be used once.
 *                      
 * @param symmetric_key Pointer to Symmetric key, api_crypto_box_BEFORENMBYTES bytes long.
 * 
 * @return 0    The authenticated and encrypted message could be created.
 * @return != 0 The authenticated and encrypted message could not be created.
 */
int api_crypto_box_afternm(unsigned char *cipher,
                           const unsigned char *clear_text,
                           unsigned long long length,
                           const unsigned char *nonce,
                           const unsigned char *symmetric_key)
{
    return crypto_box_afternm(cipher, clear_text, length, nonce, symmetric_key);
}
/**
 * @brief Decrypts and verifies an authenticated encrypted message.
 *        
 * Example usage:
 *  const char clear_text[n + api_crypto_secretbox_BOXZEROBYTES];
 *  const unsigned char cipher[n];
 *  unsigned long long cipher_len = n;
 *  const unsigned char nonce[api_crypto_secretbox_NONCEBYTES];
 *  const unsigned char k[api_crypto_box_PUBLICKEYBYTES];
 *  const unsigned char key[crypto_sign_SECRETKEYBYTES];
 *  
 *  // Cipher message structure:
 *  // cipher = { 
 *  //      zeroPadded[api_crypto_secretbox_BOXZEROBYTES] ||
 *  //      authenticatedEncrypted[n + api_crypto_secretbox_BOXZEROBYTES]
 *  // }
 *  
 *  api_crypto_box_open_afternm(clear_text,cipher,cipher_len,nonce,key);
 *  
 *  // Clear text message structure after crypto_box_afternm:
 *  // clear_text = { 
 *  //      zeroPadded[api_crypto_secretbox_ZEROBYTES] ||
 *  //      clearText[n - api_crypto_secretbox_BOXZEROBYTES]
 *  // }
 * 
 * Note: The selected implementation MUST allow for in-place operation. I.e.,
 *       the cipher and clear_text pointer points to the same memory address.
 * 
 * @param clear_text    Pointer to clear text message.
 *                      Note: The first api_crypto_secretbox_ZEROBYTES bytes MUST be zero padded
 *                      prior to the call.
 * 
 * @param cipher        Pointer to authenticated cipher text. The first api_crypto_secretbox_BOXZEROBYTES
 *                      bytes MUST be zero padded.
 *                      
 * @param length        Length of cipher message, excluding api_crypto_secretbox_BOXZEROBYTES
 *                      bytes zero padding.
 *                      
 * @param nonce         Nonce, api_crypto_secretbox_NONCEBYTES bytes long.
 *                      Note: The nonce MUST only be used once.
 *                      
 * @param key           Symmetric key, api_crypto_box_BEFORENMBYTES bytes long.
 * 
 * @return 0    Verification and decryption of encrypted message was successfull.
 * @return != 0 Verification and decryption of encrypted message failed.
 */
int api_crypto_box_open_afternm(unsigned char *clear_text,
                            const unsigned char *cipher,
                            unsigned long long length,
                            const unsigned char *nonce,
                            const unsigned char *key)
{
    return crypto_box_open_afternm(clear_text, cipher, length, nonce, key);
}

/**
 * @brief Randomly generates a secret- and public key for signing.
 * 
 * Example usage:
 *  unsigned char pk[api_crypto_sign_PUBLICKEYBYTES];
 *  unsigned char sk[api_crypto_sign_SECRETKEYBYTES];
 *  api_crypto_sign_keypair(pk,sk);
 * 
 * @param public_key Pointer where to store public key.
 * @param secret_key Pointer where to store secret key.
 * 
 * @return 0    The keypair was successfully generated.
 * @return != 0 The keypair could not be generated.
 */
int api_crypto_sign_keypair(unsigned char *public_key,
                            unsigned char *secret_key)
{
    return crypto_sign_keypair(public_key, secret_key);
}

/**
 * @brief Creates a signed message using a secret signing key.
 * 
 * Example usage:
 *  const unsigned char signed_message[4 + api_crypto_sign_BYTES];
 *  const unsigned char signed_length;
 *  const unsigned char message[4] = { 'a', 'b', 'c', 'd' };
 *  unsigned long long mlen = 4;
 *  const unsigned char secret_key[crypto_sign_SECRETKEYBYTES];
 *  api_crypto_sign(signed_message, &signed_length, message, mlen, secret_key);
 *  
 *  signed_length is now 4 + api_crypto_sign_BYTES.
 *  
 *  Note: In place operation MUST be supported if and only if the pointer value
 *        signed_message = message - api_crypto_sign_BYTES. I.e.:
 *        
 *        buffer = { reserved[api_crypto_sign_BYTES] || message[n] }
 *        -> api_crypto_sign =>
 *        buffer = { signature[api_crypto_sign_BYTES] || message[n] }
 *  
 * @param signed_message    Pointer where to store signed message.
 * @param signed_length     Signed message length will be returned if pointed value
 *                          is not NULL. I.e, the pointer may be NULL. The length of
 *                          the signed message is always length + api_crypto_sign_BYTES.
 *                      
 * @param message           Pointer to message to sign.
 * @param message_length    Length of message to sign.
 * @param secret_key        Pointer to signer's secret key, api_crypto_sign_SECRETKEYBYTES
 *                          bytes long.
 *                          
 * @return 0    The message was successfully signed.
 * @return != 0 The message could not be signed.
 */
int api_crypto_sign(unsigned char *signed_message,
                    unsigned long long *signed_length,
                    const unsigned char *message,
                    unsigned long long message_length,
                    const unsigned char *secret_key)
{
    unsigned long long smlen;
    int ret = crypto_sign(signed_message,
                          &smlen,
                          message,
                          message_length,
                          secret_key);
    if (signed_length != NULL) {
        *signed_length = smlen;
    }
    return ret;
}

/**
 * @brief Verifies a signed message using the signer's public key.
 * 
 * Example usage:
 *  const unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
 *  const unsigned char signed_message[n];
 *  unsigned long long smlen = n;
 *  const unsigned char message[n];
 *  const unsigned char mlen;
 *  api_crypto_sign_open(message, &mlen, message, smlen, public_key);
 *  
 *  Note: The size of message must be at least the size of signed_message. In place operation
 *        is not supported.
 * 
 * @param message           Pointer where to store the message.
 * @param message_length    Message length will be returned if pointed value is
 *                          not NULL. I.e., the pointer may be NULL. The length of
 *                          the message is always length - api_crypto_sign_BYTES. * @param signed_message    Pointer to the signed message.
 * 
 * @param signed_message_length Length of signed message, >= api_crypto_sign_BYTES.
 *                              
 * @param public_key        Pointer to signer's public key, api_crypto_sign_SECRETKEYBYTES
 *                          bytes long.
 * 
 * @return 0    The signed message was verified using the public key,
 * @return != 0 The message could not be verified using the public key.
 */
int api_crypto_sign_open(unsigned char *message,
                         unsigned long long *message_length,
                         const unsigned char *signed_message,
                         unsigned long long signed_message_length,
                         const unsigned char *public_key)
{
    unsigned long long mlen;
    int ret = crypto_sign_open(message,
                               &mlen,
                               signed_message,
                               signed_message_length,
                               public_key);
    if (message_length != NULL) {
        *message_length = mlen;
    }
    return ret;
}

/**
 * @brief Verify a signed message in detached mode.
 * 
 * Enables for verifying a message in detached mode. I.e., the signature and the
 * message does not need to be concatenated.
 * 
 * Example usage:
 * const unsigned char signature[api_crypto_sign_BYTES];
 * const unsigned char message[n];
 * const unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
 * api_crypto_sign_verify_detached(signature, message, n, public_key);
 * 
 * @param signature         Pointer to signature, must be api_crypto_sign_BYTES bytes long.
 * @param message           Pointer to message to verify.
 * @param message_length    Message length.
 * @param public_key        Pointer to signer's public key.
 * 
 * @return 0    The signed message was verified using the public key,
 * @return != 0 The message could not be verified using the public key.
 */
int api_crypto_sign_verify_detached(const unsigned char *signature,
                                    const unsigned char *message,
                                    unsigned long long message_length,
                                    const unsigned char *public_key)
{
    return crypto_sign_verify_detached(signature,
                                       message,
                                       message_length,
                                       public_key);
}

/**
 * @brief Hashes a message using the SHA512 algorithm.
 * 
 * Example usage:
 *  const unsigned char message[n];
 *  unsigned char hash[api_crypto_hash_sha512_BYTES];
 *  api_crypto_hash_sha512(hash, message, n);
 * 
 * @param hash      Pointer to hash destination.
 * @param message   Pointer to message to hash.
 * @param length    Length of message to hash.
 * 
 * @return 0    The hash was successfully calculated.
 * @return != 0 The hash could not be calculated.
 */
int api_crypto_hash_sha512(unsigned char *hash,
                           const unsigned char *message,
                           unsigned long long length)
{
    return crypto_hash_sha512(hash, message, length);
}


/**
 * @brief Initiate multi-part hashing using the SHA512 algorithm.
 * 
 * The hash state typically uses this structure:
 * 
 * struct crypto_hash_sha512_state {
 *   uint64_t state[8];
 *   uint64_t count[2];
 *   uint8_t  buf[128];
 * }
 * 
 * This structure is used by libsodium internal and is used as reference
 * size of the hash state. I.e., normally the hash state size required is 208 bytes.
 * Note that the size might differ from this. If multi-part is not required, and the library
 * does not know what cryptographic library to use, the recommendation is to use the
 * api_crypto_hash_sha512 interface instead.
 * 
 * TODO: Consider adding int api_crypto_hash_sha512_state_size(void);
 * 
 * @param hash_state        Pointer to hash state buffer.
 * @param hash_state_size   Size of allocated bytes for hash state.
 * 
 * @return 0    The hash state was successfully initiated.
 * @return != 0 The hash state size was to small to keep the hash state.
 */
int api_crypto_hash_sha512_init(unsigned char *hash_state,
                                unsigned char hash_state_size)
{
    if (hash_state_size < sizeof(crypto_hash_sha512_state)) {
        return -1;
    }
    return crypto_hash_sha512_init((crypto_hash_sha512_state *) hash_state);
}

/**
 * @brief Update the hash state with part of a message.
 * 
 * The hash_state must have been initialized using api_crypto_hash_sha512_init
 * prior to this call.
 * 
 * @param hash_state    Pointer to hash state.
 * @param in            Pointer to message to update the hash state with.
 * @param inlen         Length of message.
 * 
 * @return 0    The hash state was successfully updated.
 * @return != 0 The hash state could not be updated.
 */
int api_crypto_hash_sha512_update(unsigned char *hash_state,
                                  const unsigned char *in,
                                  unsigned long long inlen)
{
    return crypto_hash_sha512_update((crypto_hash_sha512_state *) hash_state,
                                     in,
                                     inlen);
}

/**
 * @brief Finalizes the hash of the multi-part message.
 * 
 * The fcuntions api_crypto_hash_sha512_init must have been used prior
 * to use this call.
 * 
 * Example usage:
 * 
 * unsigned char hash_state[api_crypto_hash_sha512_state_size];
 * unsigned char part1[n];
 * unsigned char part2[m];
 * unsigned char hash[api_crypto_hash_sha512_BYTES];
 * 
 * api_crypto_hash_sha512_init(hash_state, sizeof(hash_state));
 * api_crypto_hash_sha512_update(hash_state, part1, n);
 * api_crypto_hash_sha512_update(hash_state, part2, m);
 * api_crypto_hash_sha512_final(hash_state, hash);
 * 
 * @param hash_state    Pointer to hash state.
 * @param out           Pointer to hash destination.
 * 
 * @return [description]
 */
int api_crypto_hash_sha512_final(unsigned char *hash_state,
                                 unsigned char *out)
{
    return crypto_hash_sha512_final((crypto_hash_sha512_state *) hash_state,
                                    out);
}


/*======= Local function implementations ====================================*/

