#include "util.hpp"
#include <string>

void Util::printUnsignedChar(unsigned char *array, size_t len)
{
	std::cout << std::hex;
	for (size_t i = 0; i < len; ++i) {
		std::cout << std::setfill('0') << std::setw(2) << static_cast<unsigned>(array[i]);
	}

	std::cout << std::dec << std::endl;
}

void Util::stringToUnsignedChar(const std::string &str, unsigned char *out)
{
	size_t len;
	std::string tmp;

	for (size_t i = 0; i < str.size(); i += 2) {
		tmp = str[i];
		tmp += str[i + 1];
		out[i / 2] = std::stoul(tmp, &len, 16);
	}
}

int Util::hash512(const std::string &in, size_t len, std::string &out)
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

int Util::hash512(const unsigned char *in, size_t len, unsigned char *out)
{
        SHA512(in, len, out);
        return 0;
}

void Util::printKeys(EVP_PKEY *key)
{
	PEM_write_PUBKEY(stdout, key);
	PEM_write_PrivateKey(stdout, key, NULL, NULL, 0, NULL, NULL);
}

void Util::genKeyX25519(EVP_PKEY **key)
{
	*key = EVP_PKEY_new();
	EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, NULL);
	EVP_PKEY_keygen_init(kctx);
	EVP_PKEY_keygen(kctx, key);

	EVP_PKEY_CTX_free(kctx);
	//std::cout << "end of generation" << std::endl;
}

void Util::genKeyED25519(EVP_PKEY **key)
{
	*key = EVP_PKEY_new();
	EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, NULL);
	EVP_PKEY_keygen_init(kctx);
	EVP_PKEY_keygen(kctx, key);

	EVP_PKEY_CTX_free(kctx);
	//std::cout << "end of generation" << std::endl;
}

int Util::kdf(unsigned char *secret, size_t ssize, unsigned char *key, size_t *keylen, unsigned char *salt)
{
	while (ERR_get_error() != 0) {}
	EVP_KDF *kdf;
	EVP_KDF_CTX *kctx = NULL;
	unsigned char derived[64];
	// maybe use unique_ptr?
	unsigned char FKM[32 + ssize];
	OSSL_PARAM params[5], *p = params;


	kdf = EVP_KDF_fetch(NULL, "hkdf", NULL);
	if (kdf == NULL) {
#ifdef DEBUG
		std::cerr << ERR_error_string(ERR_get_error(), NULL) << std::endl;
#endif // DEBUG
		EVP_KDF_free(kdf);
		return 1;
	}

	kctx = EVP_KDF_CTX_new(kdf);
	if (kctx == NULL)
		goto error;

	// for X25519 it is sequence of 32 0xFF bytes
	// for X448 it is sequence of 57 0xFF bytes
	std::memset(FKM, 0xFF, 32);
	std::memcpy(&FKM[32], secret, ssize);

	// salt is zero-filled byte sequence with same length as hash output
	//unsigned char salt[64];
	//std::memset(salt, 0, 64);

	// not particularly nice
	*p++ = OSSL_PARAM_construct_utf8_string("digest", const_cast<char *>("sha512"), static_cast<size_t>(7));
	*p++ = OSSL_PARAM_construct_octet_string("salt", salt, static_cast<size_t>(64));
	*p++ = OSSL_PARAM_construct_octet_string("key", FKM, ssize + 32);
	*p++ = OSSL_PARAM_construct_octet_string("info", (void *)("yolo"), static_cast<size_t>(4));
	*p = OSSL_PARAM_construct_end();

	if (EVP_KDF_CTX_set_params(kctx, params) <= 0)
		goto error;

	if (EVP_KDF_derive(kctx, key, 64) <= 0)
		goto error;

	EVP_KDF_free(kdf);
	EVP_KDF_CTX_free(kctx);
	return 0;

error:
#ifdef DEBUG
	std::cerr << ERR_error_string(ERR_get_error(), NULL) << std::endl;
#endif // DEBUG
	EVP_KDF_CTX_free(kctx);
	EVP_KDF_free(kdf);
	return 1;
}
int Util::ecdh(EVP_PKEY *key, EVP_PKEY *peer, unsigned char *secret, size_t *ssize)
{
	while (ERR_get_error() != 0) {}

        EVP_PKEY_CTX *ctx;
        ctx = EVP_PKEY_CTX_new(key, NULL);

        if (EVP_PKEY_derive_init(ctx) != 1)
		goto error;

        if (EVP_PKEY_derive_set_peer(ctx, peer) != 1)
		goto error;

        if (EVP_PKEY_derive(ctx, NULL, ssize) != 1)
		goto error;

        if (EVP_PKEY_derive(ctx, secret, ssize) != 1)
		goto error;

        EVP_PKEY_CTX_free(ctx);
	return 0;
error:
#ifdef DEBUG
	std::cerr << ERR_error_string(ERR_get_error(), NULL) << std::endl;
#endif // DEBUG
	EVP_PKEY_CTX_free(ctx);
	return 1;
}

int Util::aes256encrypt(unsigned char *plain, size_t plen, unsigned char *key, unsigned char *iv, unsigned char *ciphertext, int pad /* = 1 */)
{
	while (ERR_get_error() != 0) {}

	int len = plen;
	int clen = 0;
	EVP_CIPHER_CTX *ctx;

	ctx = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_set_padding(ctx, pad);

	if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1)
		goto error;

	if (pad == 0 && plen % EVP_CIPHER_CTX_block_size(ctx) != 0)
	    goto error;

	if (EVP_EncryptUpdate(ctx, ciphertext, &len, plain, plen) != 1)
		goto error;

	clen = len;

	if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1)
		goto error;

	clen += len;

	EVP_CIPHER_CTX_free(ctx);
	return clen;

error:
#ifdef DEBUG
	std::cerr << ERR_error_string(ERR_get_error(), NULL) << std::endl;
#endif // DEBUG
	EVP_CIPHER_CTX_free(ctx);
	return 0;
}

int Util::aes256decrypt(unsigned char *ciphertext, size_t clen, unsigned char *key, unsigned char *iv, unsigned char *plain, int pad /* = 1 */)
{
	while (ERR_get_error() != 0) {}

	int len = 0;
	int plen = 0;
	EVP_CIPHER_CTX *ctx;

	ctx = EVP_CIPHER_CTX_new();

	EVP_CIPHER_CTX_set_padding(ctx, pad);

	if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1)
		goto error;

	if (EVP_DecryptUpdate(ctx, plain, &len, ciphertext, clen) != 1)
		goto error;

	plen = len;

	if (EVP_DecryptFinal_ex(ctx, plain + len, &len) != 1)
		goto error;

	plen += len;

	EVP_CIPHER_CTX_free(ctx);
	return plen;

error:
#ifdef DEBUG
	std::cerr << ERR_error_string(ERR_get_error(), NULL) << std::endl;
#endif // DEBUG
	EVP_CIPHER_CTX_free(ctx);
	return 0;
}

void Util::convert_mont(unsigned char *out, const unsigned char *in)
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

void Util::calculate_key_pair(unsigned char *orig_pvt, unsigned char *new_pvt, unsigned char *new_pub)
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

void Util::xeddsa_sign(unsigned char *priv, const unsigned char *message, size_t mlen, unsigned char *random /* 64 bytes long*/, unsigned char *sig)
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

int Util::xeddsa_verify(unsigned char *pub, const unsigned char *message, size_t mlen, unsigned char *sig)
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

	/* 27742317777372353535851937790883648493 in little endian format */
	const uint8_t l_low[16] = {
	    0xED, 0xD3, 0xF5, 0x5C, 0x1A, 0x63, 0x12, 0x58, 0xD6, 0x9C, 0xF7, 0xA2,
	    0xDE, 0xF9, 0xDE, 0x14
	};

	const char allzeroes[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	/*
	 * Check 0 <= s < L where L = 2^252 + 27742317777372353535851937790883648493
	 *
	 * If not the signature is publicly invalid. Since it's public we can do the
	 * check in variable time.
	 *
	 * First check the most significant byte
	 */
	int i;
	if (s[31] > 0x10)
	    return 0;
	if (s[31] == 0x10) {
	    /*
	     * Most significant byte indicates a value close to 2^252 so check the
	     * rest
	     */
	    if (memcmp(s + 16, allzeroes, sizeof(allzeroes)) != 0)
	        return 1;
	    for (i = 15; i >= 0; i--) {
	        if (s[i] < l_low[i])
	            break;
	        if (s[i] > l_low[i])
	            return 1;
	    }
	    if (i < 0)
	        return 1;
	}


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


keyPair Ratchet::kdf_rk(unsigned char* RK, unsigned char* dh_out) {
    keyPair keypair;
    size_t len = 64;
    unsigned char output[64];
    Util::kdf(dh_out, 32, output, &len, RK);
    std:: memcpy(output, keypair.key1, 32);
    std:: memcpy(output, keypair.key2, 32);
    return keypair;

}
void Ratchet::InitA (unsigned char* SK, unsigned char* BpubKey) {
    //Util::genKeyX25519(&DHs);
    std::memcpy(BpubKey, &DHr, 32);

    unsigned char secret[32];
    size_t ssize;
    Util::ecdh(DHs->key, DHr, secret, &ssize );
    keyPair pairA = kdf_rk(RK, secret);
    std:: memcpy(RK, pairA.key1, 32);
    std:: memcpy(CKs, pairA.key2, 32);

}

void Ratchet::InitB (unsigned char* SK, Key *BkeyPair) {
    DHs = BkeyPair;
}
