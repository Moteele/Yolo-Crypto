#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <dlfcn.h>
#include <iomanip>
#include <cstring>
#include <memory>
#include <vector>
#include <cstdint>

#include "include/openssl/sha.h"
#include "include/openssl/evp.h"
#include "include/openssl/kdf.h"
#include "include/openssl/pem.h"
#include "include/openssl/err.h"

#include "openssl_internals/curve25519.h"



/**
 * class with cryptographic functions
 * TODO: add checks
 */
class Util {
private:
	Util() {}
public:
	/**
	 * prints an unsigned char as hexadecimal to std::cout
	 * @param array		array of unsigned chars
	 * @param len		length of array
	 */
	static void printUnsignedChar(unsigned char *array, size_t len)
	{
		std::cout << std::hex;
		for (size_t i = 0; i < len; ++i) {
			std::cout << std::setfill('0') << std::setw(2) << static_cast<unsigned>(array[i]);
		}

		std::cout << std::dec << std::endl;
	}

	/**
	 * hashes the input with sha512
	 * @param in    input string
	 * @param len   length of input
	 * @param out   output string
	 * @return 0 success, 2 library does not have required symbol
	 */
	static int hash512(const std::string &in, size_t len, std::string &out)
	{
	        unsigned char hash[64];
		std::vector<unsigned char> toHash(in.begin(), in.end());

		SHA512(&toHash[0], len, hash);
	        //mbedtls_sha512_ret(&toHash[0], in.size(), hash, 0);

		std::stringstream buff;

	        buff << std::hex;

	        for (size_t i = 0; i < 64; ++i) {
	                buff << std::setfill('0') << std::setw(2) << static_cast<unsigned>(hash[i]);
	        }

	        buff << std::dec;

		out = buff.str();

	        return 0;
	}

	/**
	 * hashes the input with sha512
	 * @param in    input string
	 * @param len   length of input
	 * @param out   output string
	 * @return 0 success, 2 library does not have required symbol
	 */
	static int hash512(const unsigned char *in, size_t len, unsigned char *out)
	{
	        SHA512(in, len, out);
	        return 0;
	}

	static void printKeys(EVP_PKEY *key)
	{
		PEM_write_PUBKEY(stdout, key);
		PEM_write_PrivateKey(stdout, key, NULL, NULL, 0, NULL, NULL);
	}

	/**
	 * generates an X25519 keypair and writes it to @param key
	 * @param key		EVP_PKEY ** structure
	 */
	static void genKeyX25519(EVP_PKEY **key)
	{
		*key = EVP_PKEY_new();
		EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, NULL);
		EVP_PKEY_keygen_init(kctx);
		EVP_PKEY_keygen(kctx, key);

		EVP_PKEY_CTX_free(kctx);
		std::cout << "end of generation" << std::endl;
	}

	/**
	 * generates an ED25519 keypair and writes it to @param key
	 * @param key		EVP_PKEY ** structure
	 */
	static void genKeyED25519(EVP_PKEY **key)
	{
		*key = EVP_PKEY_new();
		EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, NULL);
		EVP_PKEY_keygen_init(kctx);
		EVP_PKEY_keygen(kctx, key);

		EVP_PKEY_CTX_free(kctx);
		std::cout << "end of generation" << std::endl;
	}

	/**
	 * TODO: add error return codes
	 * calculates shared secret from two X25519 keys.
	 * @param key		complete keypair
	 * @param peer		public key of the other side
	 * @param secret	shared secret
	 * @param ssize		size of shared secret
	 */
	static void ecdh(EVP_PKEY *key, EVP_PKEY *peer, unsigned char *secret, size_t *ssize)
	{
	        EVP_PKEY_CTX *ctx;

	        ctx = EVP_PKEY_CTX_new(key, NULL);

	        if (EVP_PKEY_derive_init(ctx) != 1) {
	                std::cout << "ctx initialization failed" << std::endl;
	                return;
	        }

	        if (EVP_PKEY_derive_set_peer(ctx, peer) != 1) {
	                std::cout << "setting peer failed" << std::endl;
	                return;
	        }

	        if (EVP_PKEY_derive(ctx, NULL, ssize) != 1) {
	                std::cout << "getting length failed" << std::endl;
	                return;
	        }

	        std::cout << "Length of secret = " << *ssize << std::endl;

	        EVP_PKEY_derive(ctx, secret, ssize);
	        printUnsignedChar(secret, *ssize);

	        EVP_PKEY_CTX_free(ctx);
	}

	/**
	 * derives a key from shared secret with sha512
	 * @param secret		shared secret
	 * @param ssize			length of shared secret
	 * @param key			derived key
	 * @param keylen		length of key
	 */
	static void kdf(unsigned char *secret, size_t ssize, unsigned char *key, size_t *keylen)
	{
		while (ERR_get_error() != 0) {}
		EVP_KDF *kdf;
		EVP_KDF_CTX *kctx = NULL;
		unsigned char derived[64];
		OSSL_PARAM params[5], *p = params;


		kdf = EVP_KDF_fetch(NULL, "hkdf", NULL);
		if (kdf == NULL) {
			std::cout << ERR_error_string(ERR_get_error(), NULL) << std::endl;
			return;
		}

		kctx = EVP_KDF_CTX_new(kdf);

		if (kctx == NULL) {
			std::cout << ERR_error_string(ERR_get_error(), NULL) << std::endl;
			return;
		}

		// for X25519 it is sequence of 32 0xFF bytes
		// for X448 it is sequence of 57 0xFF bytes
		unsigned char FKM[32 + ssize];
		std::memset(FKM, 0xFF, 32);
		std::memcpy(&FKM[32], secret, ssize);

		// salt is zero-filled byte sequence with same length as hash output
		unsigned char salt[64];
		std::memset(salt, 0, 64);
		// not particularly nice
		*p++ = OSSL_PARAM_construct_utf8_string("digest", const_cast<char *>("sha512"), static_cast<size_t>(7));
		*p++ = OSSL_PARAM_construct_octet_string("salt", salt, static_cast<size_t>(64));
		*p++ = OSSL_PARAM_construct_octet_string("key", FKM, ssize + 32);
		*p++ = OSSL_PARAM_construct_octet_string("info", (void *)("yolo"), static_cast<size_t>(4));
		*p = OSSL_PARAM_construct_end();

		EVP_KDF_CTX_set_params(kctx, params);

		EVP_KDF_derive(kctx, key, 64);

		EVP_KDF_free(kdf);
		EVP_KDF_CTX_free(kctx);
	}

	/**
	 * encrypts text using aes-256 in cbc mode
	 * @param plain		plaintext to be encrypted
	 * @param plen		length of the plaintext
	 * @param key		key used for encryption, must be 32 bytes long
	 * @param iv		initialization vector, must be 16 bytes long
	 * @param ciphertext	buffer where the ciphertext is written
	 * @return length of ciphertext
	 */
	static int aes256encrypt(unsigned char *plain, size_t plen, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
	{
		int len = plen;
		int clen = 0;
		EVP_CIPHER_CTX *ctx;

		ctx = EVP_CIPHER_CTX_new();

		EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

		EVP_EncryptUpdate(ctx, ciphertext, &len, plain, plen);
		clen = len;

		EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
		clen += len;

		EVP_CIPHER_CTX_free(ctx);
		return clen;
	}

	/**
	 * decrypts ciphertext from aes-256 in cbc mode
	 * @param ciphertext	encrypted text
	 * @param clen		length of ciphertext
	 * @param key		key used for decryption, must be 32 bytes long
	 * @param iv		initialization vector, must be 16 bytes long
	 * @param plain		buffer, where the decrypted text is written
	 * @return length of decrypted plaintext
	 */
	static int aes256decrypt(unsigned char *ciphertext, size_t clen, unsigned char *key, unsigned char *iv, unsigned char *plain)
	{
		int len;
		int plen;
		EVP_CIPHER_CTX *ctx;

		ctx = EVP_CIPHER_CTX_new();

		EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

		EVP_DecryptUpdate(ctx, plain, &len, ciphertext, clen);
		plen = len;

		EVP_DecryptFinal_ex(ctx, plain + len, &len);
		plen += len;

		EVP_CIPHER_CTX_free(ctx);
		return plen;
	}

	/**
	 * converts montgomery u coordinate to edwards y coordinate
	 * @param out		encoded y coordinate
	 * @param in		encoded u coordinate
	 */
	static void convert_mont(unsigned char *out, const unsigned char *in)
	{
		fe u;
		fe y;

		fe_frombytes(u, in);

		fe up1;
		fe um1;
		fe inv;
		fe f1;
		fe_1(f1);

		fe_add(up1, u, f1);
		fe_sub(um1, u, f1);
		fe_invert(inv, up1);
		fe_mul(y, um1, inv);

		fe_tobytes(out, y);
	}

	/**
	 * creates edwards25519 keypair from curve25519 private key
	 * @param orig_pvt		curve25519 private key
	 * @param new_pvt		edwards25519 private key
	 * @param new_pub		edwards25519 public key
	 */
	static void calculate_key_pair(unsigned char *orig_pvt, unsigned char *new_pvt, unsigned char *new_pub)
	{
		unsigned char Eb[32];
		unsigned char A[32];
		unsigned char test[32];
		ge_p3 E;

		unsigned char sign = 0x00;
		ge_scalarmult_base(&E, orig_pvt);
		ge_p3_tobytes(Eb, &E);

		sign = Eb[31] & 0x80;
		sign = sign >> 7;

		fe a;
		fe aneg;
		fe_frombytes(a, orig_pvt);

		fe_neg(aneg, a);

		// if sign == 1
		fe_cmov(a, aneg, sign);

		// copy to A
		std::memcpy(new_pub, Eb, 32);
		//new_pub[31] &= 0x7F;

		fe_tobytes(new_pvt, a);
	}

	/**
	 * creates a signature from curve25519 private key and message
	 * @param priv		private key used for signing
	 * @param message	message to be signed
	 * @param mlen		length of message
	 * @param random	64 bytes of random data
	 * @param sig		signature
	 */
	void xeddsa_sign(unsigned char *priv, const unsigned char *message, size_t mlen, unsigned char *random /* 64 bytes long*/, unsigned char *sig)
	{
		unsigned char new_priv[32];
		unsigned char new_pub[32];
		unsigned char hash[64];
		unsigned char fill[32];
		unsigned char r[64];
		unsigned char h[64];
		unsigned char R[32];
		unsigned char s[32];
		std::memset(s, 0, 32);
		ge_p3 Rp;
		std::memset(&fill[0], 0xFE, 1);
		std::memset(&fill[1], 0xFF, 31);
		auto cat = std::make_unique<unsigned char[]>(32 + 32 + mlen + 64);

		// create new keypair
		calculate_key_pair(priv, new_priv, new_pub);

		// save sign
		unsigned char sign = new_pub[31] & 0x80;

		// concatenate input to hash
		std::memcpy(&cat[0], fill, 32);
		std::memcpy(&cat[32], priv, 32);
		std::memcpy(&cat[64], message, mlen);
		std::memcpy(&cat[64 + mlen], random, 64);

		// r = hash1(new_priv || message || random) (mod q)
		SHA512(cat.get(), 64 + mlen + 64, r);
		x25519_sc_reduce(r);

		// R = rB
		ge_scalarmult_base(&Rp, r);
		ge_p3_tobytes(R, &Rp);

		std::memcpy(cat.get(), R, 32);
		std::memcpy(&cat[32], new_pub, 32);


		// h = hash (R || new_pub || message) (mod q)
		SHA512(cat.get(), 64 + mlen, h);
		x25519_sc_reduce(h);

		// s = r + ha (modq)
		sc_muladd(s, h, priv, r);

		std::memcpy(sig, R, 32);
		std::memcpy(&sig[32], s, 32);

		// clear add sign to signature
		sig[63] &= 0x7F;
		sig[63] |= sign;
	}

	/**
	 * verifies a signature given the message and public key
	 * @param pub		public key for checking
	 * @param message	message
	 * @param mlen		length of message
	 * @param sig		signature ofmessage
	 * @return 0 on success
	 */
	int xeddsa_verify(unsigned char *pub, const unsigned char *message, size_t mlen, unsigned char *sig)
	{
		// TODO: check validity of parameters
		unsigned char A[32];
		unsigned char R[32];
		unsigned char Rc[32];
		unsigned char h[64];
		unsigned char hh[32];
		unsigned char s[32];
		auto cat = std::make_unique<unsigned char []>(32 + 32 + mlen);
		ge_p2 Rcp;
		ge_p3 Ap;

		std::memset(s, 0, 32);
		std::memset(A, 0, 32);
		std::memset(h, 0, 32);
		std::memset(R, 0, 32);
		std::memset(Rc, 0, 32);

		convert_mont(A, pub);
		// TODO: check if A is on curve

		// add sign bit from signature
		A[31] &= 0x7F;
		A[31] |= (sig[63] & 0x80);

		// and delete it from signature
		sig[63] &= 0x7F;

		std::memcpy(s, &sig[32], 32);

		// populate buffer
		std::memcpy(&cat[0], sig, 32);
		std::memcpy(&cat[32], A, 32);
		std::memcpy(&cat[64], message, mlen);

		// h = hash(R || A || M) (mod q)
		SHA512(cat.get(), 32 + 32 + mlen, h);
		x25519_sc_reduce(h);

		std::memcpy(hh, h, 32);

		// negate A
		ge_frombytes_vartime(&Ap, A);
		fe_neg(Ap.X, Ap.X);
		fe_neg(Ap.T, Ap.T);

		// Rc = sB - hA
		ge_double_scalarmult_vartime(&Rcp, h, &Ap, s);

		// compare
		int result = 0;
		std::memcpy(R, sig, 32);
		ge_tobytes(Rc, &Rcp);
		result = CRYPTO_memcmp(R, Rc, 32);

		return result;
	}
};

#endif // UTIL_HPP
