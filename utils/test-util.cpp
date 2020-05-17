#include <string>
#include "catch.hpp"
#include "util.hpp"
#include "key.hpp"
#include <google/protobuf/stubs/common.h>
#include <openssl/rand.h>

TEST_CASE("aes256")
{
	std::vector<std::string> keys;
	std::vector<std::string> ivs;
	std::vector<std::string> pts;
	std::vector<std::string> cts;
	std::vector<std::string> cts2;

	keys.emplace_back("6ed76d2d97c69fd1339589523931f2a6cff554b15f738f21ec72dd97a7330907");
	keys.emplace_back("dce26c6b4cfb286510da4eecd2cffe6cdf430f33db9b5f77b460679bd49d13ae");
	keys.emplace_back("fe8901fecd3ccd2ec5fdc7c7a0b50519c245b42d611a5ef9e90268d59f3edf33");
	keys.emplace_back("0493ff637108af6a5b8e90ac1fdf035a3d4bafd1afb573be7ade9e8682e663e5");
	keys.emplace_back("9adc8fbd506e032af7fa20cf5343719de6d1288c158c63d6878aaf64ce26ca85");
	keys.emplace_back("73b8faf00b3302ac99855cf6f9e9e48518690a5906a4869d4dcf48d282faae2a");
	keys.emplace_back("9ddf3745896504ff360a51a3eb49c01b79fccebc71c3abcb94a949408b05b2c9");
	keys.emplace_back("458b67bf212d20f3a57fce392065582dcefbf381aa22949f8338ab9052260e1d");
	keys.emplace_back("d2412db0845d84e5732b8bbd642957473b81fb99ca8bff70e7920d16c1dbec89");
	keys.emplace_back("48be597e632c16772324c8d3fa1d9c5a9ecd010f14ec5d110d3bfec376c5532b");

	keys.emplace_back("43e953b2aea08a3ad52d182f58c72b9c60fbe4a9ca46a3cb89e3863845e22c9e");
	keys.emplace_back("addf88c1ab997eb58c0455288c3a4fa320ada8c18a69cc90aa99c73b174dfde6");
	keys.emplace_back("54682728db5035eb04b79645c64a95606abb6ba392b6633d79173c027c5acf77");
	keys.emplace_back("7482c47004aef406115ca5fd499788d582efc0b29dc9e951b1f959406693a54f");
	keys.emplace_back("3ae38d4ebf7e7f6dc0a1e31e5efa7ca123fdc321e533e79fedd5132c5999ef5b");
	keys.emplace_back("d30bfc0b2a19d5b8b6f8f46ab7f444ee136a7fa3fbdaf530cc3e8976339afcc4");
	keys.emplace_back("64a256a663527ebea71f8d770990b4cee4a2d3afbfd33fb12c7ac300ef59e49a");
	keys.emplace_back("31358e8af34d6ac31c958bbd5c8fb33c334714bffb41700d28b07f11cfe891e7");
	keys.emplace_back("5b4b69339891db4e3337c3486f439dfbd0fb2a782ca71ef0059819d51669d93c");
	keys.emplace_back("87725bd43a45608814180773f0e7ab95a3c859d83a2130e884190e44d14c6996");

	ivs.emplace_back("851e8764776e6796aab722dbb644ace8");
	ivs.emplace_back("fdeaa134c8d7379d457175fd1a57d3fc");
	ivs.emplace_back("bd416cb3b9892228d8f1df575692e4d0");
	ivs.emplace_back("c0cd2bebccbb6c49920bd5482ac756e8");
	ivs.emplace_back("11958dc6ab81e1c7f01631e9944e620f");
	ivs.emplace_back("b3cb97a80a539912b8c21f450d3b9395");
	ivs.emplace_back("e79026639d4aa230b5ccffb0b29d79bc");
	ivs.emplace_back("4c12effc5963d40459602675153e9649");
	ivs.emplace_back("51c619fcf0b23f0c7925f400a6cacb6d");
	ivs.emplace_back("d6d581b8cf04ebd3b6eaa1b53f047ee1");

	ivs.emplace_back("ddbbb0173f1e2deb2394a62aa2a0240e");
	ivs.emplace_back("60cc50e0887532e0d4f3d2f20c3c5d58");
	ivs.emplace_back("2eb94297772851963dd39a1eb95d438f");
	ivs.emplace_back("485ebf2215d20b816ea53944829717ce");
	ivs.emplace_back("36d55dc9edf8669beecd9a2a029092b9");
	ivs.emplace_back("80be76a7f885d2c06b37d6a528fae0cd");
	ivs.emplace_back("18cce9147f295c5c00dbe0424089d3b4");
	ivs.emplace_back("144516246a752c329056d884daf3c89d");
	ivs.emplace_back("2b28a2d19ba9ecd149dae96622c21769");
	ivs.emplace_back("e49651988ebbb72eb8bb80bb9abbca34");

	pts.emplace_back("6282b8c05c5c1530b97d4816ca434762");
	pts.emplace_back("50e9eee1ac528009e8cbcd356975881f957254b13f91d7c6662d10312052eb00");
	pts.emplace_back("8d3aa196ec3d7c9b5bb122e7fe77fb1295a6da75abe5d3a510194d3a8a4157d5c89d40619716619859da3ec9b247ced9");
	pts.emplace_back("8b37f9148df4bb25956be6310c73c8dc58ea9714ff49b643107b34c9bff096a94fedd6823526abc27a8e0b16616eee254ab4567dd68e8ccd4c38ac563b13639c");
	pts.emplace_back("c7917f84f747cd8c4b4fedc2219bdbc5f4d07588389d8248854cf2c2f89667a2d7bcf53e73d32684535f42318e24cd45793950b3825e5d5c5c8fcd3e5dda4ce9246d18337ef3052d8b21c5561c8b660e");
	pts.emplace_back("3adea6e06e42c4f041021491f2775ef6378cb08824165edc4f6448e232175b60d0345b9f9c78df6596ec9d22b7b9e76e8f3c76b32d5d67273f1d83fe7a6fc3dd3c49139170fa5701b3beac61b490f0a9e13f844640c4500f9ad3087adfb0ae10");
	pts.emplace_back("cf52e5c3954c51b94c9e38acb8c9a7c76aebdaa9943eae0a1ce155a2efdb4d46985d935511471452d9ee64d2461cb2991d59fc0060697f9a671672163230f367fed1422316e52d29eceacb8768f56d9b80f6d278093c9a8acd3cfd7edd8ebd5c293859f64d2f8486ae1bd593c65bc014");
	pts.emplace_back("256fd73ce35ae3ea9c25dd2a9454493e96d8633fe633b56176dce8785ce5dbbb84dbf2c8a2eeb1e96b51899605e4f13bbc11b93bf6f39b3469be14858b5b720d4a522d36feed7a329c9b1e852c9280c47db8039c17c4921571a07d1864128330e09c308ddea1694e95c84500f1a61e614197e86a30ecc28df64ccb3ccf5437aa");
	pts.emplace_back("026006c4a71a180c9929824d9d095b8faaa86fc4fa25ecac61d85ff6de92dfa8702688c02a282c1b8af4449707f22d75e91991015db22374c95f8f195d5bb0afeb03040ff8965e0e1339dba5653e174f8aa5a1b39fe3ac839ce307a4e44b4f8f1b0063f738ec18acdbff2ebfe07383e734558723e741f0a1836dafdf9de82210a9248bc113b3c1bc8b4e252ca01bd803");
	pts.emplace_back("0c63d413d3864570e70bb6618bf8a4b9585586688c32bba0a5ecc1362fada74ada32c52acfd1aa7444ba567b4e7daaecf7cc1cb29182af164ae5232b002868695635599807a9a7f07a1f137e97b1e1c9dabc89b6a5e4afa9db5855edaa575056a8f4f8242216242bb0c256310d9d329826ac353d715fa39f80cec144d6424558f9f70b98c920096e0f2c855d594885a00625880e9dfb734163cecef72cf030b8");

	pts.emplace_back("07270d0e63aa36daed8c6ade13ac1af1");
	pts.emplace_back("98a8a9d84356bf403a9ccc384a06fe043dfeecb89e59ce0cb8bd0a495ef76cf0");
	pts.emplace_back("0faa5d01b9afad3bb519575daaf4c60a5ed4ca2ba20c625bc4f08799addcf89d19796d1eff0bd790c622dc22c1094ec7");
	pts.emplace_back("82fec664466d585023821c2e39a0c43345669a41244d05018a23d7159515f8ff4d88b01cd0eb83070d0077e065d74d7373816b61505718f8d4f270286a59d45e");
	pts.emplace_back("8d22db30c4253c3e3add9685c14d55b05f7cf7626c52cccfcbe9b99fd8913663b8b1f22e277a4cc3d0e7e978a34782eb876867556ad4728486d5e890ea738243e3700a696d6eb58cd81c0e60eb121c50");
	pts.emplace_back("0b6e2a8213169b3b78db6de324e286f0366044e035c6970afbf0a1a5c32a05b24ba706cd9c6609737651a81b2bcf4c681dc0861983a5aec76e6c8b244112d64d489e84328974737394b83a39459011727162652b7aa793bfb1b71488b7dec96b");
	pts.emplace_back("f7e0f79cfddd15ed3600ab2d29c56ba3c8e96d1a896aff6dec773e6ea4710a77f2f4ec646b76efda6428c175d007c84aa9f4b18c5e1bac5f27f7307b737655eee813f7e1f5880a37ac63ad1666e7883083b648454d45786f53ea3db1b5129291138abe40c79fcb7ab7c6f6b9ea133b5f");
	pts.emplace_back("cfc155a3967de347f58fa2e8bbeb4183d6d32f7427155e6ab39cddf2e627c572acae02f1f243f3b784e73e21e7e520eacd3befafbee814867334c6ee8c2f0ee7376d3c72728cde7813173dbdfe3357deac41d3ae2a04229c0262f2d109d01f5d03e7f848fb50c28849146c02a2f4ebf7d7ffe3c9d40e31970bf151873672ef2b");
	pts.emplace_back("a0bb1d2fdeb7e6bf34c690fe7b72a5e9d65796aa57982fe340c286d6923dbddb426566ff58e9c0b3af52e4db446f6cc5daa5bfcf4e3c85db5a5638e670c370cce128db22c97542a64a63846f18a228d3462a11376dcb71f66ec52ebda474f7b6752915b0801797974bc51eb1218127fed60f1009430eb5089fb3ba5f28fad24c518ccddc2501393ceb6dffc46a159421");
	pts.emplace_back("bfe5c6354b7a3ff3e192e05775b9b75807de12e38a626b8bf0e12d5fff78e4f1775aa7d792d885162e66d88930f9c3b2cdf8654f56972504803190386270f0aa43645db187af41fcea639b1f8026ccdd0c23e0de37094a8b941ecb7602998a4b2604e69fc04219585d854600e0ad6f99a53b2504043c08b1c3e214d17cde053cbdf91daa999ed5b47c37983ba3ee254bc5c793837daaa8c85cfc12f7f54f699f");

	cts2.emplace_back("6acc04142e100a65f51b97adf5172c41");
	cts2.emplace_back("2fa0df722a9fd3b64cb18fb2b3db55ff2267422757289413f8f657507412a64c");
	cts2.emplace_back("608e82c7ab04007adb22e389a44797fed7de090c8c03ca8a2c5acd9e84df37fbc58ce8edb293e98f02b640d6d1d72464");
	cts2.emplace_back("05d5c77729421b08b737e41119fa4438d1f570cc772a4d6c3df7ffeda0384ef84288ce37fc4c4c7d1125a499b051364c389fd639bdda647daa3bdadab2eb5594");
	cts2.emplace_back("9c99e68236bb2e929db1089c7750f1b356d39ab9d0c40c3e2f05108ae9d0c30b04832ccdbdc08ebfa426b7f5efde986ed05784ce368193bb3699bc691065ac62e258b9aa4cc557e2b45b49ce05511e65");
	cts2.emplace_back("ac3d6dbafe2e0f740632fd9e820bf6044cd5b1551cbb9cc03c0b25c39ccb7f33b83aacfca40a3265f2bbff879153448acacb88fcfb3bb7b10fe463a68c0109f028382e3e557b1adf02ed648ab6bb895df0205d26ebbfa9a5fd8cebd8e4bee3dc");
	cts2.emplace_back("34df561bd2cfebbcb7af3b4b8d21ca5258312e7e2e4e538e35ad2490b6112f0d7f148f6aa8d522a7f3c61d785bd667db0e1dc4606c318ea4f26af4fe7d11d4dcff0456511b4aed1a0d91ba4a1fd6cd9029187bc5881a5a07fe02049d39368e83139b12825bae2c7be81e6f12c61bb5c5");
	cts2.emplace_back("90b7b9630a2378f53f501ab7beff039155008071bc8438e789932cfd3eb1299195465e6633849463fdb44375278e2fdb1310821e6492cf80ff15cb772509fb426f3aeee27bd4938882fd2ae6b5bd9d91fa4a43b17bb439ebbe59c042310163a82a5fe5388796eee35a181a1271f00be29b852d8fa759bad01ff4678f010594cd");
	cts2.emplace_back("0254b23463bcabec5a395eb74c8fb0eb137a07bc6f5e9f61ec0b057de305714f8fa294221c91a159c315939b81e300ee902192ec5f15254428d8772f79324ec43298ca21c00b370273ee5e5ed90e43efa1e05a5d171209fe34f9f29237dba2a6726650fd3b1321747d1208863c6c3c6b3e2d879ab5f25782f08ba8f2abbe63e0bedb4a227e81afb36bb6645508356d34");
	cts2.emplace_back("fc5873e50de8faf4c6b84ba707b0854e9db9ab2e9f7d707fbba338c6843a18fc6facebaf663d26296fb329b4d26f18494c79e09e779647f9bafa87489630d79f4301610c2300c19dbf3148b7cac8c4f4944102754f332e92b6f7c5e75bc6179eb877a078d4719009021744c14f13fd2a55a2b9c44d18000685a845a4f632c7c56a77306efa66a24d05d088dcd7c13fe24fc447275965db9e4d37fbc9304448cd");

	cts2.emplace_back("d51d19ded5ca4ae14b2b20b027ffb020");
	cts2.emplace_back("6cb4e2f4ddf79a8e08c96c7f4040e8a83266c07fc88dd0074ee25b00d445985a");
	cts2.emplace_back("e4046d05385ab789c6a72866e08350f93f583e2a005ca0faecc32b5cfc323d461c76c107307654db5566a5bd693e227c");
	cts2.emplace_back("6c24f19b9c0b18d7126bf68090cb8ae72db3ca7eabb594f506aae7a2493e5326a5afae4ec4d109375b56e2b6ff4c9cf639e72c63dc8114c796df95b3c6b62021");
	cts2.emplace_back("d50ea48c8962962f7c3d301fa9f877245026c204a7771292cddca1e7ffebbef00e86d72910b7d8a756dfb45c9f1040978bb748ca537edd90b670ecee375e15d98582b9f93b6355adc9f80f4fb2108fb9");
	cts2.emplace_back("31e4677a17aed120bd3af69fbb0e4b645b9e8c104e280b799ddd49f1e241c3ccb7d40e1c6ff226bf04f8049c51a86e2981cf1331c824d7d451746ccf77fc22fd3717001ee51913d81f7a06fb0037f309957579f695670f2c4c7397d2d990374e");
	cts2.emplace_back("d99771963b7ae5202e382ff8c06e035367909cd24fe5ada7f3d39bfaeb5de98b04eaf4989648e00112f0d2aadb8c5f2157b64581450359965140c141e5fb631e43469d65d1b7370eb3b396399fec32cced294a5eee46d6547f7bbd49dee148b4bc31d6c493cfd28f3908e36cb698629d");
	cts2.emplace_back("b32e2b171b63827034ebb0d1909f7ef1d51c5f82c1bb9bc26bc4ac4dccdee8357dca6154c2510ae1c87b1b422b02b621bb06cac280023894fcff3406af08ee9be1dd72419beccddff77c722d992cdcc87e9c7486f56ab406ea608d8c6aeb060c64cf2785ad1a159147567e39e303370da445247526d95942bf4d7e88057178b0");
	cts2.emplace_back("ba21db8ec170fa4d73cfc381687f3fa188dd2d012bef48007f3dc88329e22ba32fe235a315be362546468b9db6af6705c6e5d4d36822f42883c08d4a994cc454a7db292c4ca1f4b62ebf8e479a5d545d6af9978d2cfee7bc80999192c2c8662ce9b4be11af40bd68f3e2d5685bb28c0f3dc08017c0aba8263e6fdc45ed7f9893bf14fd3a86c418a35c5667e642d59985");
	cts2.emplace_back("5b97a9d423f4b97413f388d9a341e727bb339f8e18a3fac2f2fb85abdc8f135deb30054a1afdc9b6ed7da16c55eba6b0d4d10c74e1d9a7cf8edfaeaa684ac0bd9f9d24ba674955c79dc6be32aee1c260b558ff07e3a4d49d24162011ff254db8be078e8ad07e648e6bf5679376cb4321a5ef01afe6ad8816fcc7634669c8c4389295c9241e45fff39f3225f7745032daeebe99d4b19bcb215d1bfdb36eda2c24");

	unsigned char key[32];
	unsigned char iv[16];
	unsigned char pt[160];
	unsigned char pt2[160];
	unsigned char ct[160];
	unsigned char ct2[160];



	SECTION("aes encrypt")
	{
		for (size_t i = 0; i < 10; ++i) {
			Util::stringToUnsignedChar(keys[i], key);
			Util::stringToUnsignedChar(ivs[i], iv);
			Util::stringToUnsignedChar(pts[i], pt);
			Util::stringToUnsignedChar(cts2[i], ct2);

			Util::aes256encrypt(pt, 16 + i * 16, key, iv, ct, 0);

			REQUIRE(std::memcmp(ct, ct2, 16 + 16 * i) == 0);
		}

		for (size_t i = 0; i < 10; ++i) {
			Util::stringToUnsignedChar(keys[i + 10], key);
			Util::stringToUnsignedChar(ivs[i + 10], iv);
			Util::stringToUnsignedChar(pts[i + 10], pt);
			Util::stringToUnsignedChar(cts2[i + 10], ct2);

			Util::aes256encrypt(pt, 16 + i * 16, key, iv, ct, 0);

			REQUIRE(std::memcmp(ct, ct2, 16 + 16 * i) == 0);
		}
	}

	SECTION("aes invalid encryption")
	{
		for (size_t i = 0; i < 10; ++i) {
			Util::stringToUnsignedChar(keys[i], key);
			Util::stringToUnsignedChar(ivs[i], iv);
			Util::stringToUnsignedChar(pts[i], pt);
			Util::stringToUnsignedChar(cts2[i], ct2);

			REQUIRE(Util::aes256encrypt(pt, 15 + i * 16, key, iv, ct, 0) == 0);
		}

		for (size_t i = 0; i < 10; ++i) {
			Util::stringToUnsignedChar(keys[i + 10], key);
			Util::stringToUnsignedChar(ivs[i + 10], iv);
			Util::stringToUnsignedChar(pts[i + 10], pt);
			Util::stringToUnsignedChar(cts2[i + 10], ct2);

			REQUIRE(Util::aes256encrypt(pt, 15 + i * 16, key, iv, ct, 0) == 0);

		}
	}

	SECTION("aes decrypt")
	{
		for (size_t i = 0; i < 10; ++i) {
			Util::stringToUnsignedChar(keys[i], key);
			Util::stringToUnsignedChar(ivs[i], iv);
			Util::stringToUnsignedChar(pts[i], pt2);
			Util::stringToUnsignedChar(cts2[i], ct2);

			Util::aes256decrypt(ct2, 16 + 16 * i, key, iv, pt, 0);

			REQUIRE(std::memcmp(pt, pt2, 16 + 16 * i) == 0);
		}

		for (size_t i = 0; i < 10; ++i) {
			Util::stringToUnsignedChar(keys[i + 10], key);
			Util::stringToUnsignedChar(ivs[i + 10], iv);
			Util::stringToUnsignedChar(pts[i + 10], pt2);
			Util::stringToUnsignedChar(cts2[i + 10], ct2);

			Util::aes256decrypt(ct2, 16 + i * 16, key, iv, pt, 0);

			REQUIRE(std::memcmp(pt, pt2, 16 + 16 * i) == 0);
		}
	}

	SECTION("aes invalid decryption")
	{
		for (size_t i = 0; i < 10; ++i) {
			Util::stringToUnsignedChar(keys[i], key);
			Util::stringToUnsignedChar(ivs[i], iv);
			Util::stringToUnsignedChar(pts[i], pt2);
			Util::stringToUnsignedChar(cts2[i], ct2);

			REQUIRE(Util::aes256decrypt(ct2, 15 + 16 * i, key, iv, pt, 0) == 0);
		}

		for (size_t i = 0; i < 10; ++i) {
			Util::stringToUnsignedChar(keys[i + 10], key);
			Util::stringToUnsignedChar(ivs[i + 10], iv);
			Util::stringToUnsignedChar(pts[i + 10], pt2);
			Util::stringToUnsignedChar(cts2[i + 10], ct2);

			REQUIRE(Util::aes256decrypt(ct2, 15 + i * 16, key, iv, pt, 0) == 0);
		}
	}

	SECTION("aes encrypt == decrypt, without padding")
	{
		unsigned char buffer[64];
		size_t len = 0;
		for (size_t i = 0; i < 256; ++i) {
			RAND_bytes(buffer, 32);
			std::memcpy(key, buffer, 32);
			RAND_bytes(buffer, 16);
			std::memcpy(iv, buffer, 16);
			RAND_bytes(buffer, 64);
			std::memcpy(pt, buffer, 64);

			len = Util::aes256encrypt(pt, 64, key, iv, ct, 0);

			Util::aes256decrypt(ct, len, key, iv, pt2, 0);

			REQUIRE(std::memcmp(pt, pt2, 64) == 0);
		}
	}

	SECTION("aes encrypt == decrypt, with padding")
	{
		unsigned char buffer[64];
		size_t len = 0;
		for (size_t i = 0; i < 256; ++i) {
			RAND_bytes(buffer, 32);
			std::memcpy(key, buffer, 32);
			RAND_bytes(buffer, 16);
			std::memcpy(iv, buffer, 16);
			RAND_bytes(buffer, 64);
			std::memcpy(pt, buffer, 64);

			len = Util::aes256encrypt(pt, 64, key, iv, ct);

			Util::aes256decrypt(ct, len, key, iv, pt2);

			REQUIRE(std::memcmp(pt, pt2, 64) == 0);
		}
	}

	google::protobuf::ShutdownProtobufLibrary();
}

TEST_CASE("Util function tests")
{

	SECTION("xeddsa")
	{
		for (int i = 0; i < 128; ++i) {
                size_t len = 32;
                unsigned char message[] = "Toto je zpráva";
                unsigned char rnd[64];
                unsigned char sig[64];
                std::memset(rnd, 0, 64);

                Key k;
		k.generate();

                std::vector<uint8_t> sk = k.getPrivateKey();
                std::vector<uint8_t> pk = k.getPublicKey();

				Util::xeddsa_sign(&sk[0], message, sizeof(message), rnd, sig);

                REQUIRE(Util::xeddsa_verify(&pk[0], message, sizeof(message), sig) == 0);

		// change it a bit
		if (sig[12] != 0xff) {
			sig[12] = static_cast<unsigned>(sig[12]) + 1;
		} else {
			sig[12] = static_cast<unsigned>(sig[12]) - 1;
		}

                REQUIRE_FALSE(Util::xeddsa_verify(&pk[0], message, sizeof(message), sig) == 0);
		}
	}

	SECTION("xeddsa random")
	{
		unsigned char message[128];
                unsigned char rnd[64];
                unsigned char sig[64];

		RAND_bytes(message, 128);
		RAND_bytes(rnd, 64);

		Key k;
		k.generate();

                std::vector<uint8_t> sk = k.getPrivateKey();
                std::vector<uint8_t> pk = k.getPublicKey();

		Util::xeddsa_sign(&sk[0], message, sizeof(message), rnd, sig);

                REQUIRE(Util::xeddsa_verify(&pk[0], message, sizeof(message), sig) == 0);

		// change it a bit
		if (sig[12] != 0xff) {
			sig[12] = static_cast<unsigned>(sig[12]) + 1;
		} else {
			sig[12] = static_cast<unsigned>(sig[12]) - 1;
		}

                REQUIRE_FALSE(Util::xeddsa_verify(&pk[0], message, sizeof(message), sig) == 0);
	}

	SECTION("ecdh basic")
	{
		unsigned char raw_alice_priv[32] = {
				0x77, 0x07, 0x6d, 0x0a, 0x73, 0x18, 0xa5, 0x7d,
				0x3c, 0x16, 0xc1, 0x72, 0x51, 0xb2, 0x66, 0x45,
				0xdf, 0x4c, 0x2f, 0x87, 0xeb, 0xc0, 0x99, 0x2a,
				0xb1, 0x77, 0xfb, 0xa5, 0x1d, 0xb9, 0x2c, 0x2a};
		unsigned char raw_alice_pub[32] = {
				0x85, 0x20, 0xf0, 0x09, 0x89, 0x30, 0xa7, 0x54,
				0x74, 0x8b, 0x7d, 0xdc, 0xb4, 0x3e, 0xf7, 0x5a,
				0x0d, 0xbf, 0x3a, 0x0d, 0x26, 0x38, 0x1a, 0xf4,
				0xeb, 0xa4, 0xa9, 0x8e, 0xaa, 0x9b, 0x4e, 0x6a};

		unsigned char raw_bob_priv[32] = {
				0x5d, 0xab, 0x08, 0x7e, 0x62, 0x4a, 0x8a, 0x4b,
				0x79, 0xe1, 0x7f, 0x8b, 0x83, 0x80, 0x0e, 0xe6,
				0x6f, 0x3b, 0xb1, 0x29, 0x26, 0x18, 0xb6, 0xfd,
				0x1c, 0x2f, 0x8b, 0x27, 0xff, 0x88, 0xe0, 0xeb};

		unsigned char raw_bob_pub[32] = {
				0xde, 0x9e, 0xdb, 0x7d, 0x7b, 0x7d, 0xc1, 0xb4,
				0xd3, 0x5b, 0x61, 0xc2, 0xec, 0xe4, 0x35, 0x37,
				0x3f, 0x83, 0x43, 0xc8, 0x5b, 0x78, 0x67, 0x4d,
				0xad, 0xfc, 0x7e, 0x14, 0x6f, 0x88, 0x2b, 0x4f};

		unsigned char secret_right[32] = {
				0x4a, 0x5d, 0x9d, 0x5b, 0xa4, 0xce, 0x2d, 0xe1,
				0x72, 0x8e, 0x3b, 0xf4, 0x80, 0x35, 0x0f, 0x25,
				0xe0, 0x7e, 0x21, 0xc9, 0x47, 0xd1, 0x9e, 0x33,
				0x76, 0xf0, 0x9b, 0x3c, 0x1e, 0x16, 0x17, 0x42};


		Key alice(raw_alice_priv, false);
		Key bob(raw_bob_priv, false);

		size_t ssize;
		unsigned char secret1[32];
		unsigned char secret2[32];
		Util::ecdh(alice, bob, secret1, &ssize);
		Util::ecdh(bob, alice, secret2, &ssize);

		REQUIRE(std::memcmp(secret1, secret2, 32) == 0);
		REQUIRE(std::memcmp(secret1, secret_right, 32) == 0);
		REQUIRE(std::memcmp(secret2, secret_right, 32) == 0);

	}

	SECTION("ecdh basic 2")
	{
		size_t len;
		unsigned char priv[32];
		unsigned char pub[32];
		unsigned char out[32];
		unsigned char out_correct[32];
		Util::stringToUnsignedChar("a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4", priv);
		Util::stringToUnsignedChar("e6db6867583030db3594c1a424b15f7c726624ec26b3353b10a903a6d0ab1c4c", pub);
		Util::stringToUnsignedChar("c3da55379de9c6908e94ea4df28d084f32eccf03491c71f754b4075577a28552", out_correct);

		Key key(priv, false);
		Key peer(pub, true);

		Util::ecdh(key, peer, out, &len);
		REQUIRE(std::memcmp(out, out_correct, 32) == 0);

		Util::stringToUnsignedChar("4b66e9d4d1b4673c5ad22691957d6af5c11b6421e0ea01d42ca4169e7918ba0d", priv);
		Util::stringToUnsignedChar("e5210f12786811d3f4b7959d0538ae2c31dbe7106fc03c3efc4cd549c715a493", pub);
		Util::stringToUnsignedChar("95cbde9476e8907d7aade45cb4b873f88b595a68799fa152e6f8f7647aac7957", out_correct);

		key.setPrivate(priv);
		peer.setPublic(pub);

		Util::ecdh(key, peer, out, &len);
		REQUIRE(std::memcmp(out, out_correct, 32) == 0);

	}

	SECTION("ecdh advanced")
	{
		size_t len;
		unsigned char priv[32];
		unsigned char pub[32];
		unsigned char out[32];
		unsigned char out_correct_1[32];
		unsigned char out_correct_1000[32];
		unsigned char out_correct_1000000[32];
		Util::stringToUnsignedChar("0900000000000000000000000000000000000000000000000000000000000000", priv);
		Util::stringToUnsignedChar("0900000000000000000000000000000000000000000000000000000000000000", pub);
		Util::stringToUnsignedChar("422c8e7a6227d7bca1350b3e2bb7279f7897b87bb6854b783c60e80311ae3079", out_correct_1);
		Util::stringToUnsignedChar("684cf59ba83309552800ef566f2f4d3c1c3887c49360e3875f2eb94d99532c51", out_correct_1000);
		Util::stringToUnsignedChar("7c3911e0ab2586fd864497297e575e6f3bc601c0883c30df5f4dd2d24f665424", out_correct_1000000);

		Key key;
		Key peer;

		// set i <= 1 000 000, if you are brave enough (or patient)
		for (size_t i = 1; i <= 1000; ++i) {
			key.setPrivate(priv);
			peer.setPublic(pub);

			Util::ecdh(key, peer, out, &len);

			std::memcpy(pub, priv, 32);
			std::memcpy(priv, out, 32);

			if (i == 1)
				REQUIRE(std::memcmp(out, out_correct_1, 32) == 0);

			if (i == 1000)
				REQUIRE(std::memcmp(out, out_correct_1000, 32) == 0);

			if (i == 1000000)
				REQUIRE(std::memcmp(out, out_correct_1000000, 32) == 0);
		}
	}

	google::protobuf::ShutdownProtobufLibrary();
}

TEST_CASE("kdf")
{
	SECTION("kdf basic")
	{
		unsigned char secret[32] = "123456789qwertyuiopasdfghjklzxc";
		std::vector<unsigned char> key(512);
		std::memset(&key[0], 0, 512);
		size_t len = 0;

		for (int i = 0; i < 512; ++i) {
			std::memset(&key[0], 0, 512);
			REQUIRE(Util::kdf(secret, 32, &key[0], &len) == 0);
			REQUIRE(key[len] == 0);
			++len;
		}
	}

	SECTION("kdf secret length")
	{
		unsigned char secret[32] = "123456789qwertyuiopasdfghjklzxc";
		std::vector<unsigned char> key(32);
		std::memset(&key[0], 0, 32);
		size_t len = 32;

		for (int i = 0; i < 32; ++i) {
			std::memset(&key[0], 0, 32);
			REQUIRE(Util::kdf(secret, i, &key[0], &len) == 0);
		}
	}

	SECTION("kdf determinism")
	{
		unsigned char secret[32] = "123456789qwertyuiopasdfghjklzxc";
		std::vector<unsigned char> key(128);
		std::vector<unsigned char> tmp(128);
		std::memset(&key[0], 0, 128);
		std::memset(&tmp[0], 0, 128);
		size_t len = 128;
		REQUIRE(Util::kdf(secret, 32, &key[0], &len) == 0);

		for (int i = 0; i < 32; ++i) {
			REQUIRE(Util::kdf(secret, 32, &tmp[0], &len) == 0);
			REQUIRE(memcmp(&key[0], &tmp[0], 128) == 0);
			std::memcpy(&key[0], &tmp[0], 128);
		}
	}
}


TEST_CASE("ratchet")
{
    SECTION("ratchet init")
    {
	unsigned char bPrivKey[32];
	std::memset(bPrivKey, 1, 32);
	unsigned char SK[32];
	std::memset(SK, 2, 32);
	Ratchet bob;
	Ratchet alice;

	bob.InitB(SK, bPrivKey);
	alice.InitA(SK, &bob.DHs.getPublicKey()[0]);

	std::vector<unsigned char> test(32, 1);
	REQUIRE(bob.DHs.getPrivateKey() == test);
	REQUIRE(! std::memcmp(bob.RK, SK, 32));
	REQUIRE(alice.DHr.getPublicKey() == bob.DHs.getPublicKey());
    }
    SECTION("ratchet encrypt")
    {
	unsigned char bPrivKey[32];
	std::memset(bPrivKey, 1, 32);
	unsigned char SK[32];
	std::memset(SK, 2, 32);
	Ratchet bob;
	Ratchet alice;

	bob.InitB(SK, bPrivKey);
	alice.InitA(SK, &bob.DHs.getPublicKey()[0]);

	unsigned char AD[64];
	std::memset(AD, 3, 64);
	unsigned char message[32] = "123456789qwertyuiopasdfghjklzxc";
	// alice.RatchetEncrypt(message, AD);

	REQUIRE(1==1);
    }




}
