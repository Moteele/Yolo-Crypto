# Static analysis
There are many static analysis tools to choose from. I have chosen the _-Wall -Wextra_ flags for gcc and cppcheck. 

## gcc -Wall -Wextra
Using this method, I have found mainly four types of errors:

* unused variable
* unused parameter
* unused function
* comparision of integers with different sign

The unused fuctions are mainly low level curve/point arithmetic functions. The unused parameter/variable are usually in the X3DH, XEdDSA and Double ratchet functions. And the integer comparision errors are mainly in the for loop statement. 

## cppcheck
Cppcheck found many of the same errors (unused functions), but added a few new ones:

* shadowing of variables
* assigning value, that is never used
* index out of bounds

## Raw output

### gcc -Wall -Wextra raw output

```
utils/functions.cpp: In function ‘std::string stringToHex(const string&)’:
utils/functions.cpp:99:20: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::__cxx11::basic_string<char>::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
   99 |  for (int i = 0; i < input.size(); ++i) {
      |                  ~~^~~~~~~~~~~~~~
utils/functions.cpp: In function ‘std::string keyToHex(std::vector<unsigned char>&)’:
utils/functions.cpp:110:20: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<unsigned char>::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  110 |  for (int i = 0; i < key.size(); ++i) {
      |                  ~~^~~~~~~~~~~~
In file included from utils/util.hpp:20,
                 from utils/key.cpp:11:
utils/../libs/openssl_internals/curve25519.h:4972:13: warning: ‘void sc_muladd(uint8_t*, const uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
 4972 | static void sc_muladd(uint8_t *s, const uint8_t *a, const uint8_t *b,
      |             ^~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:4628:13: warning: ‘void x25519_sc_reduce(uint8_t*)’ defined but not used [-Wunused-function]
 4628 | static void x25519_sc_reduce(uint8_t *s)
      |             ^~~~~~~~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:4544:13: warning: ‘void ge_double_scalarmult_vartime(ge_p2*, const uint8_t*, const ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4544 | static void ge_double_scalarmult_vartime(ge_p2 *r, const uint8_t *a,
      |             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:4239:13: warning: ‘void ge_scalarmult_base(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4239 | static void ge_scalarmult_base(ge_p3 *h, const uint8_t *a)
      |             ^~~~~~~~~~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:1868:12: warning: ‘int ge_frombytes_vartime(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 1868 | static int ge_frombytes_vartime(ge_p3 *h, const uint8_t *s)
      |            ^~~~~~~~~~~~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:1845:13: warning: ‘void ge_p3_tobytes(uint8_t*, const ge_p3*)’ defined but not used [-Wunused-function]
 1845 | static void ge_p3_tobytes(uint8_t *s, const ge_p3 *h)
      |             ^~~~~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:1832:13: warning: ‘void ge_tobytes(uint8_t*, const ge_p2*)’ defined but not used [-Wunused-function]
 1832 | static void ge_tobytes(uint8_t *s, const ge_p2 *h)
      |             ^~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:697:13: warning: ‘void x25519_scalar_mult(uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
  697 | static void x25519_scalar_mult(uint8_t out[32], const uint8_t scalar[32],
      |             ^~~~~~~~~~~~~~~~~~
utils/util.cpp: In static member function ‘static int Util::kdf(unsigned char*, size_t, unsigned char*, size_t*, unsigned char*, size_t)’:
utils/util.cpp:88:16: warning: unused variable ‘derived’ [-Wunused-variable]
   88 |  unsigned char derived[64];
      |                ^~~~~~~
utils/util.cpp: In static member function ‘static void Util::calculate_key_pair(unsigned char*, unsigned char*, unsigned char*)’:
utils/util.cpp:283:16: warning: unused variable ‘A’ [-Wunused-variable]
  283 |  unsigned char A[32];
      |                ^
utils/util.cpp:284:16: warning: unused variable ‘test’ [-Wunused-variable]
  284 |  unsigned char test[32];
      |                ^~~~
utils/util.cpp: In static member function ‘static void Util::xeddsa_sign(unsigned char*, const unsigned char*, size_t, unsigned char*, unsigned char*)’:
utils/util.cpp:314:16: warning: unused variable ‘hash’ [-Wunused-variable]
  314 |  unsigned char hash[64];
      |                ^~~~
utils/util.cpp: In member function ‘void Ratchet::Encrypt(unsigned char*, unsigned char*, unsigned int, unsigned char*, unsigned char*)’:
utils/util.cpp:531:38: warning: unused parameter ‘mk’ [-Wunused-parameter]
  531 | void Ratchet::Encrypt(unsigned char *mk, unsigned char *plaintext, unsigned int pt_size, unsigned char *ad, unsigned char *ciphertext) {
      |                       ~~~~~~~~~~~~~~~^~
utils/util.cpp:531:105: warning: unused parameter ‘ad’ [-Wunused-parameter]
  531 | void Ratchet::Encrypt(unsigned char *mk, unsigned char *plaintext, unsigned int pt_size, unsigned char *ad, unsigned char *ciphertext) {
      |                                                                                          ~~~~~~~~~~~~~~~^~
utils/util.cpp: In member function ‘void Ratchet::Decrypt(unsigned char*, unsigned char*, unsigned int, unsigned char*, Header, unsigned char*)’:
utils/util.cpp:563:39: warning: unused parameter ‘mk’ [-Wunused-parameter]
  563 | void Ratchet:: Decrypt(unsigned char* mk, unsigned char *ciphertext, unsigned int ct_len, unsigned char *AD, Header header, unsigned char *plaintext) {
      |                        ~~~~~~~~~~~~~~~^~
utils/util.cpp:563:106: warning: unused parameter ‘AD’ [-Wunused-parameter]
  563 | void Ratchet:: Decrypt(unsigned char* mk, unsigned char *ciphertext, unsigned int ct_len, unsigned char *AD, Header header, unsigned char *plaintext) {
      |                                                                                           ~~~~~~~~~~~~~~~^~
utils/util.cpp:563:117: warning: unused parameter ‘header’ [-Wunused-parameter]
  563 | void Ratchet:: Decrypt(unsigned char* mk, unsigned char *ciphertext, unsigned int ct_len, unsigned char *AD, Header header, unsigned char *plaintext) {
      |                                                                                                              ~~~~~~~^~~~~~
In file included from utils/util.hpp:20,
                 from utils/util.cpp:1:
utils/../libs/openssl_internals/curve25519.h: At global scope:
utils/../libs/openssl_internals/curve25519.h:697:13: warning: ‘void x25519_scalar_mult(uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
  697 | static void x25519_scalar_mult(uint8_t out[32], const uint8_t scalar[32],
      |             ^~~~~~~~~~~~~~~~~~
In file included from client/../utils/util.hpp:20,
                 from client/client.hpp:6,
                 from client/main.cpp:1:
client/../utils/../libs/openssl_internals/curve25519.h:4972:13: warning: ‘void sc_muladd(uint8_t*, const uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
 4972 | static void sc_muladd(uint8_t *s, const uint8_t *a, const uint8_t *b,
      |             ^~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:4628:13: warning: ‘void x25519_sc_reduce(uint8_t*)’ defined but not used [-Wunused-function]
 4628 | static void x25519_sc_reduce(uint8_t *s)
      |             ^~~~~~~~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:4544:13: warning: ‘void ge_double_scalarmult_vartime(ge_p2*, const uint8_t*, const ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4544 | static void ge_double_scalarmult_vartime(ge_p2 *r, const uint8_t *a,
      |             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:4239:13: warning: ‘void ge_scalarmult_base(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4239 | static void ge_scalarmult_base(ge_p3 *h, const uint8_t *a)
      |             ^~~~~~~~~~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:1868:12: warning: ‘int ge_frombytes_vartime(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 1868 | static int ge_frombytes_vartime(ge_p3 *h, const uint8_t *s)
      |            ^~~~~~~~~~~~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:1845:13: warning: ‘void ge_p3_tobytes(uint8_t*, const ge_p3*)’ defined but not used [-Wunused-function]
 1845 | static void ge_p3_tobytes(uint8_t *s, const ge_p3 *h)
      |             ^~~~~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:1832:13: warning: ‘void ge_tobytes(uint8_t*, const ge_p2*)’ defined but not used [-Wunused-function]
 1832 | static void ge_tobytes(uint8_t *s, const ge_p2 *h)
      |             ^~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:697:13: warning: ‘void x25519_scalar_mult(uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
  697 | static void x25519_scalar_mult(uint8_t out[32], const uint8_t scalar[32],
      |             ^~~~~~~~~~~~~~~~~~
client/client.cpp: In member function ‘int Client::getIndexOfSharedSecret(const string&)’:
client/client.cpp:251:23: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<std::pair<Ratchet*, std::__cxx11::basic_string<char> > >::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  251 |     for (int i = 0; i < sharedSecrets_.size(); ++i) {
      |                     ~~^~~~~~~~~~~~~~~~~~~~~~~
client/client.cpp: In member function ‘void Client::develSendMessage()’:
client/client.cpp:311:9: warning: unused variable ‘len’ [-Wunused-variable]
  311 |     int len = Util::aes256encrypt(ad, 64, key, iv, ciphered, 0);
      |         ^~~
client/client.cpp: In member function ‘void Client::readInitial(const string&)’:
client/client.cpp:392:23: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::__cxx11::basic_string<char>::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  392 |     for (int i = 0; i < encodedEncryptedAd.size(); i += 8) {
      |                     ~~^~~~~~~~~~~~~~~~~~~~~~~~~~~
client/client.cpp:478:23: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<std::pair<Ratchet*, std::__cxx11::basic_string<char> > >::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  478 |     for (int i = 0; i < sharedSecrets_.size(); ++i) {
      |                     ~~^~~~~~~~~~~~~~~~~~~~~~~
client/client.cpp:461:9: warning: unused variable ‘len’ [-Wunused-variable]
  461 |     int len = Util::aes256decrypt(encryptedAdArr, 64, sharedSecret, iv, decryptedAd, 0);
      |         ^~~
client/client.cpp: In member function ‘void Client::printMessages()’:
client/client.cpp:549:6: warning: unused variable ‘len’ [-Wunused-variable]
  549 |  int len = Util::aes256decrypt(encryptedAdArr, 64, const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>
      |      ^~~
client/client.cpp: In member function ‘void Client::printSharedSecrets()’:
client/client.cpp:568:23: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<std::pair<Ratchet*, std::__cxx11::basic_string<char> > >::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  568 |     for (int i = 0; i < sharedSecrets_.size(); ++i) {
      |                     ~~^~~~~~~~~~~~~~~~~~~~~~~
client/client.cpp: In member function ‘void Client::initConnection()’:
client/client.cpp:584:10: warning: unused variable ‘buffer’ [-Wunused-variable]
  584 |     char buffer[2048] = {0};
      |          ^~~~~~
In file included from server/server.cpp:1:
server/server.hpp:179:2: warning: type qualifiers ignored on function return type [-Wignored-qualifiers]
  179 |  const unsigned int getId() const { return id_; }
      |  ^~~~~
server/server.cpp: In member function ‘void Server::processRequests(int)’:
server/server.cpp:119:20: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<std::__cxx11::basic_string<char> >::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  119 |  for (int i = 0; i < requests_.size(); ++i) {
      |                  ~~^~~~~~~~~~~~~~~~~~
server/server.cpp: In member function ‘void Server::performSendMessage(int, const string&)’:
server/server.cpp:164:20: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<userAcc>::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  164 |  for (int i = 0; i < users_.size(); ++i) {
      |                  ~~^~~~~~~~~~~~~~~
server/server.cpp:178:20: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<userAcc>::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  178 |  for (int i = 0; i < users_.size(); ++i) {
      |                  ~~^~~~~~~~~~~~~~~
server/server.cpp: In member function ‘void Server::performAuth(int, const string&)’:
server/server.cpp:216:20: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<userAcc>::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  216 |  for (int i = 0; i < users_.size(); ++i) {
      |                  ~~^~~~~~~~~~~~~~~
server/server.cpp: In member function ‘void Server::performFetchMessages(int, const string&)’:
server/server.cpp:243:20: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<userAcc>::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  243 |  for (int i = 0; i < users_.size(); ++i) {
      |                  ~~^~~~~~~~~~~~~~~
server/server.cpp: In member function ‘void Server::performSendInitialMsg(int, const string&)’:
server/server.cpp:277:20: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<userAcc>::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  277 |  for (int i = 0; i < users_.size(); ++i) {
      |                  ~~^~~~~~~~~~~~~~~
server/server.cpp: In member function ‘void Server::performFetchKeys(int, const string&)’:
server/server.cpp:301:20: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<userAcc>::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  301 |  for (int i = 0; i < users_.size(); ++i) {
      |                  ~~^~~~~~~~~~~~~~~
server/server.cpp: In member function ‘void Server::performCreateAccount(int, const string&)’:
server/server.cpp:338:20: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<userAcc>::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  338 |  for (int i = 0; i < users_.size(); ++i) {
      |                  ~~^~~~~~~~~~~~~~~
server/server.cpp: In member function ‘void Server::writeUsers()’:
server/server.cpp:393:20: warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::vector<userAcc>::size_type’ {aka ‘long unsigned int’} [-Wsign-compare]
  393 |  for (int i = 0; i < users_.size(); ++i) {
      |                  ~~^~~~~~~~~~~~~~~
In file included from client/../utils/util.hpp:20,
                 from client/client.hpp:6,
                 from client/client.cpp:1:
client/../utils/../libs/openssl_internals/curve25519.h: At global scope:
client/../utils/../libs/openssl_internals/curve25519.h:4972:13: warning: ‘void sc_muladd(uint8_t*, const uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
 4972 | static void sc_muladd(uint8_t *s, const uint8_t *a, const uint8_t *b,
      |             ^~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:4628:13: warning: ‘void x25519_sc_reduce(uint8_t*)’ defined but not used [-Wunused-function]
 4628 | static void x25519_sc_reduce(uint8_t *s)
      |             ^~~~~~~~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:4544:13: warning: ‘void ge_double_scalarmult_vartime(ge_p2*, const uint8_t*, const ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4544 | static void ge_double_scalarmult_vartime(ge_p2 *r, const uint8_t *a,
      |             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:4239:13: warning: ‘void ge_scalarmult_base(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4239 | static void ge_scalarmult_base(ge_p3 *h, const uint8_t *a)
      |             ^~~~~~~~~~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:1868:12: warning: ‘int ge_frombytes_vartime(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 1868 | static int ge_frombytes_vartime(ge_p3 *h, const uint8_t *s)
      |            ^~~~~~~~~~~~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:1845:13: warning: ‘void ge_p3_tobytes(uint8_t*, const ge_p3*)’ defined but not used [-Wunused-function]
 1845 | static void ge_p3_tobytes(uint8_t *s, const ge_p3 *h)
      |             ^~~~~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:1832:13: warning: ‘void ge_tobytes(uint8_t*, const ge_p2*)’ defined but not used [-Wunused-function]
 1832 | static void ge_tobytes(uint8_t *s, const ge_p2 *h)
      |             ^~~~~~~~~~
client/../utils/../libs/openssl_internals/curve25519.h:697:13: warning: ‘void x25519_scalar_mult(uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
  697 | static void x25519_scalar_mult(uint8_t out[32], const uint8_t scalar[32],
      |             ^~~~~~~~~~~~~~~~~~
In file included from server/../utils/util.hpp:20,
                 from server/server.hpp:12,
                 from server/server.cpp:1:
server/../utils/../libs/openssl_internals/curve25519.h: At global scope:
server/../utils/../libs/openssl_internals/curve25519.h:4972:13: warning: ‘void sc_muladd(uint8_t*, const uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
 4972 | static void sc_muladd(uint8_t *s, const uint8_t *a, const uint8_t *b,
      |             ^~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:4628:13: warning: ‘void x25519_sc_reduce(uint8_t*)’ defined but not used [-Wunused-function]
 4628 | static void x25519_sc_reduce(uint8_t *s)
      |             ^~~~~~~~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:4544:13: warning: ‘void ge_double_scalarmult_vartime(ge_p2*, const uint8_t*, const ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4544 | static void ge_double_scalarmult_vartime(ge_p2 *r, const uint8_t *a,
      |             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:4239:13: warning: ‘void ge_scalarmult_base(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4239 | static void ge_scalarmult_base(ge_p3 *h, const uint8_t *a)
      |             ^~~~~~~~~~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:1868:12: warning: ‘int ge_frombytes_vartime(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 1868 | static int ge_frombytes_vartime(ge_p3 *h, const uint8_t *s)
      |            ^~~~~~~~~~~~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:1845:13: warning: ‘void ge_p3_tobytes(uint8_t*, const ge_p3*)’ defined but not used [-Wunused-function]
 1845 | static void ge_p3_tobytes(uint8_t *s, const ge_p3 *h)
      |             ^~~~~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:1832:13: warning: ‘void ge_tobytes(uint8_t*, const ge_p2*)’ defined but not used [-Wunused-function]
 1832 | static void ge_tobytes(uint8_t *s, const ge_p2 *h)
      |             ^~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:697:13: warning: ‘void x25519_scalar_mult(uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
  697 | static void x25519_scalar_mult(uint8_t out[32], const uint8_t scalar[32],
      |             ^~~~~~~~~~~~~~~~~~
In file included from server/test-server.cpp:4:
server/server.hpp:179:2: warning: type qualifiers ignored on function return type [-Wignored-qualifiers]
  179 |  const unsigned int getId() const { return id_; }
      |  ^~~~~
In file included from server/../utils/util.hpp:20,
                 from server/test-server.cpp:3:
server/../utils/../libs/openssl_internals/curve25519.h:4972:13: warning: ‘void sc_muladd(uint8_t*, const uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
 4972 | static void sc_muladd(uint8_t *s, const uint8_t *a, const uint8_t *b,
      |             ^~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:4628:13: warning: ‘void x25519_sc_reduce(uint8_t*)’ defined but not used [-Wunused-function]
 4628 | static void x25519_sc_reduce(uint8_t *s)
      |             ^~~~~~~~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:4544:13: warning: ‘void ge_double_scalarmult_vartime(ge_p2*, const uint8_t*, const ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4544 | static void ge_double_scalarmult_vartime(ge_p2 *r, const uint8_t *a,
      |             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:4239:13: warning: ‘void ge_scalarmult_base(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4239 | static void ge_scalarmult_base(ge_p3 *h, const uint8_t *a)
      |             ^~~~~~~~~~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:1868:12: warning: ‘int ge_frombytes_vartime(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 1868 | static int ge_frombytes_vartime(ge_p3 *h, const uint8_t *s)
      |            ^~~~~~~~~~~~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:1845:13: warning: ‘void ge_p3_tobytes(uint8_t*, const ge_p3*)’ defined but not used [-Wunused-function]
 1845 | static void ge_p3_tobytes(uint8_t *s, const ge_p3 *h)
      |             ^~~~~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:1832:13: warning: ‘void ge_tobytes(uint8_t*, const ge_p2*)’ defined but not used [-Wunused-function]
 1832 | static void ge_tobytes(uint8_t *s, const ge_p2 *h)
      |             ^~~~~~~~~~
server/../utils/../libs/openssl_internals/curve25519.h:697:13: warning: ‘void x25519_scalar_mult(uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
  697 | static void x25519_scalar_mult(uint8_t out[32], const uint8_t scalar[32],
      |             ^~~~~~~~~~~~~~~~~~
utils/test-util.cpp: In function ‘void ____C_A_T_C_H____T_E_S_T____8()’:
utils/test-util.cpp:255:24: warning: unused variable ‘len’ [-Wunused-variable]
  255 |                 size_t len = 32;
      |                        ^~~
utils/test-util.cpp:318:17: warning: unused variable ‘raw_alice_pub’ [-Wunused-variable]
  318 |   unsigned char raw_alice_pub[32] = {
      |                 ^~~~~~~~~~~~~
utils/test-util.cpp:330:17: warning: unused variable ‘raw_bob_pub’ [-Wunused-variable]
  330 |   unsigned char raw_bob_pub[32] = {
      |                 ^~~~~~~~~~~
In file included from utils/util.hpp:20,
                 from utils/test-util.cpp:3:
utils/../libs/openssl_internals/curve25519.h: At global scope:
utils/../libs/openssl_internals/curve25519.h:4972:13: warning: ‘void sc_muladd(uint8_t*, const uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
 4972 | static void sc_muladd(uint8_t *s, const uint8_t *a, const uint8_t *b,
      |             ^~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:4628:13: warning: ‘void x25519_sc_reduce(uint8_t*)’ defined but not used [-Wunused-function]
 4628 | static void x25519_sc_reduce(uint8_t *s)
      |             ^~~~~~~~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:4544:13: warning: ‘void ge_double_scalarmult_vartime(ge_p2*, const uint8_t*, const ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4544 | static void ge_double_scalarmult_vartime(ge_p2 *r, const uint8_t *a,
      |             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:4239:13: warning: ‘void ge_scalarmult_base(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 4239 | static void ge_scalarmult_base(ge_p3 *h, const uint8_t *a)
      |             ^~~~~~~~~~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:1868:12: warning: ‘int ge_frombytes_vartime(ge_p3*, const uint8_t*)’ defined but not used [-Wunused-function]
 1868 | static int ge_frombytes_vartime(ge_p3 *h, const uint8_t *s)
      |            ^~~~~~~~~~~~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:1845:13: warning: ‘void ge_p3_tobytes(uint8_t*, const ge_p3*)’ defined but not used [-Wunused-function]
 1845 | static void ge_p3_tobytes(uint8_t *s, const ge_p3 *h)
      |             ^~~~~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:1832:13: warning: ‘void ge_tobytes(uint8_t*, const ge_p2*)’ defined but not used [-Wunused-function]
 1832 | static void ge_tobytes(uint8_t *s, const ge_p2 *h)
      |             ^~~~~~~~~~
utils/../libs/openssl_internals/curve25519.h:697:13: warning: ‘void x25519_scalar_mult(uint8_t*, const uint8_t*, const uint8_t*)’ defined but not used [-Wunused-function]
  697 | static void x25519_scalar_mult(uint8_t out[32], const uint8_t scalar[32],
      |             ^~~~~~~~~~~~~~~~~~
```

### cppcheck
```
[utils/util.cpp:405]: (style) The scope of the variable 'i' can be reduced.
[utils/util.cpp:563]: (performance) Function parameter 'header' should be passed by const reference.
[libs/openssl_internals/curve25519.h:4678]: (style) Variable 's23' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:4686]: (style) Variable 's22' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:4694]: (style) Variable 's21' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:4702]: (style) Variable 's20' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:4710]: (style) Variable 's19' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:4718]: (style) Variable 's18' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:4761]: (style) Variable 's17' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:4769]: (style) Variable 's16' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:4777]: (style) Variable 's15' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:4785]: (style) Variable 's14' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:4793]: (style) Variable 's13' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:4892]: (style) Variable 's12' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5161]: (style) Variable 's23' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5169]: (style) Variable 's22' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5177]: (style) Variable 's21' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5185]: (style) Variable 's20' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5193]: (style) Variable 's19' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5201]: (style) Variable 's18' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5244]: (style) Variable 's17' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5252]: (style) Variable 's16' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5260]: (style) Variable 's15' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5268]: (style) Variable 's14' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5276]: (style) Variable 's13' is assigned a value that is never used.
[libs/openssl_internals/curve25519.h:5375]: (style) Variable 's12' is assigned a value that is never used.
[utils/util.cpp:314]: (style) Unused variable: hash
[server/server.hpp:155]: (warning) Member variable 'Server::cryptoLib_' is not initialized in the constructor.
[server/server.hpp:155]: (style) Class 'Server' has a constructor with 1 argument that is not explicit.
[server/server.cpp:416]: (style) The scope of the variable 'activity' can be reduced.
[server/server.cpp:417]: (style) The scope of the variable 'maxSocketDescriptor' can be reduced.
[libs/openssl_internals/curve25519.h:1858] -> [/usr/local/include/google/protobuf/stubs/fastmem.h:116]: (style) Local variable 'd' shadows outer variable
[libs/openssl_internals/curve25519.h:1858] -> [/usr/local/include/google/protobuf/map_entry_lite.h:340]: (style) Local variable 'd' shadows outer variable
[server/server.cpp:416] -> [server/server.cpp:552]: (style) Local variable 'i' shadows outer variable
[server/server.cpp:16]: (style) Consider using std::any_of algorithm instead of a raw loop.
[server/server.cpp:46]: (style) Consider using std::find_if algorithm instead of a raw loop.
[server/server.cpp:251]: (error) Array index -1 is out of bounds.
[server/server.cpp:284]: (error) Array index -1 is out of bounds.
```
