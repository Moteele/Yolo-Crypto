#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <dlfcn.h>
#include <iomanip>
#include <cstring>
#include <memory>
#include <vector>
#include <cstdint>
//#include <mbedtls/sha512.h>
#include <mbedtls/sha512.h>
#include <mbedtls/ecp.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>



class Util {
private:
	Util() {}
public:
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

	        mbedtls_sha512_ret(&toHash[0], in.size(), hash, 0);

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
	        mbedtls_sha512_ret(in, len, out, 0);
	        return 0;
	}

	static int generateKeyPair(mbedtls_ecp_keypair *pair)
	{
		//if (pair == nullptr) {
		//	std::cerr << MBEDTLS_ERR_ECP_BAD_INPUT_DATA << " - \'mbedtls_ecp_keypair *pair\' is nullptr!" << std::endl;
		//	return 1;
		//}

		/* needed structures */
		mbedtls_ecp_group grp;
		mbedtls_mpi priv;
		mbedtls_ecp_point point;
		mbedtls_ctr_drbg_context ctr_drbg;
		mbedtls_entropy_context entropy;

		/* initialization */
		mbedtls_ctr_drbg_init(&ctr_drbg);
		mbedtls_entropy_init(&entropy);
		mbedtls_ecp_group_init(&grp);
		mbedtls_ecp_point_init(&point);
		mbedtls_mpi_init(&priv);
		mbedtls_ecp_group_load(&grp, mbedtls_ecp_group_id::MBEDTLS_ECP_DP_CURVE25519);
		/* TODO: seed could be better */
		mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) "RANDOM_GEN", 10);


		/* generate keypair */
		mbedtls_ecp_gen_keypair(&grp, &priv, &point, mbedtls_ctr_drbg_random, &ctr_drbg);

		/* write */
		pair->grp = grp;
		pair->d = priv;
		pair->Q = point;

		/* cleanup */
		//mbedtls_ecp_group_free(&grp);
		//mbedtls_ecp_point_free(&point);
		//mbedtls_mpi_free(&priv);
		//mbedtls_ctr_drbg_free(&ctr_drbg);
		//mbedtls_entropy_free(&entropy);

		return 0;
	}

	static void printKeyPair(mbedtls_ecp_keypair *pair)
	{
		if (pair == nullptr) {
			std::cerr << "ERROR: missing keypair" << std::endl;
			return;
		}

		size_t len;
		char private_buff[256];
		char public_x_buff[256];
		char public_y_buff[256];
		char public_z_buff[256];
		mbedtls_mpi *priv = &pair->d;
		mbedtls_ecp_point *point = &pair->Q;

		mbedtls_mpi_write_string(priv, 10, private_buff, 256, &len);
		mbedtls_mpi_write_string(&point->X, 10, public_x_buff, 256, &len);
		mbedtls_mpi_write_string(&point->Y, 10, public_y_buff, 256, &len);
		mbedtls_mpi_write_string(&point->Z, 10, public_z_buff, 256, &len);

		std::cout << "some private key:  " << private_buff << std::endl;
		std::cout << "some public key x: " << public_x_buff << std::endl;
		std::cout << "some public key y: " << public_y_buff << std::endl;
		std::cout << "some public key z: " << public_z_buff << std::endl;
	}

	static int signPubKey(mbedtls_ecp_keypair *pair, mbedtls_ecp_keypair *sig)
	{
		/* pile of garbage doing nothing useful */
		mbedtls_ecdsa_context ctx;
		mbedtls_ecdsa_init(&ctx);

		mbedtls_ecdsa_from_keypair(&ctx, pair);

		size_t len;
		unsigned char exported[512];
		std::string hash;

		mbedtls_ecp_point_write_binary(&pair->grp, &pair->Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &len, exported, 512);

		unsigned char buffer[64];
		std::memset(&buffer, 0, 64);

		hash512(exported, len, buffer);

		//std::cout << "DEBUG: hash of exported pubkey = " << buffer << std::endl;

		std::stringstream buff;

	        buff << std::hex;

	        for (size_t i = 0; i < 64; ++i) {
	                buff << std::setfill('0') << std::setw(2) << static_cast<unsigned>(buffer[i]);
	        }

	        buff << std::dec;


		std::cout << buff.str() << std::endl;

		printKeyPair(pair);

		printEcpErrorMessages();
		int r;
		r = mbedtls_ecp_check_privkey(&pair->grp, &pair->d);
		std::cout << "retval = " << r << std::endl;

		unsigned char signature[128];

		len = 0;
		r = mbedtls_ecdsa_write_signature_det(&ctx, buffer, 64, signature, &len, mbedtls_md_type_t::MBEDTLS_MD_SHA512);

		std::cout << "retval = " << r << ", len = " << len << std::endl;

		char part1_buff[512];
		char part2_buff[512];





		/* cleanup */
		mbedtls_ecp_keypair_free(&ctx);


		return 0;
	}

	static int verifySignature()
	{
		return 0;
	}

	static void printEcpErrorMessages()
	{
		std::cout << MBEDTLS_ERR_ECP_BAD_INPUT_DATA << " -  Bad input parameters to function." << std::endl;
		std::cout << MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL << " -  The buffer is too small to write to" << std::endl;
		std::cout << MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE << " -  The requested feature is not available, for example, the requested curve is not supported." << std::endl;
		std::cout << MBEDTLS_ERR_ECP_VERIFY_FAILED << " -  The signature is not valid." << std::endl;
		std::cout << MBEDTLS_ERR_ECP_ALLOC_FAILED << " -  Memory allocation failed." << std::endl;
		std::cout << MBEDTLS_ERR_ECP_RANDOM_FAILED << " -  Generation of random value, such as ephemeral key, failed." << std::endl;
		std::cout << MBEDTLS_ERR_ECP_INVALID_KEY << " -  Invalid private or public key." << std::endl;
		std::cout << MBEDTLS_ERR_ECP_SIG_LEN_MISMATCH << " -  The buffer contains a valid signature followed by more data." << std::endl;
	}
};

#endif // UTIL_HPP
