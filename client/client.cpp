#include "client.hpp"

#include <bitset>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


bool Client::develFileExists(const std::string &path) {
    std::ifstream file(path);
    return file.good();
}

void Client::writeToReq(const std::string &req) {
    send(socket_ , req.c_str() , req.size() , 0 );
    gotResponse_ = false;
}

void Client::develAuth() {
    Mess message;
    message.set_type(Mess::AUTH);
    std::cout << "Login:" << std::endl;
    std::string name;
    std::getline(std::cin, name);
    std::cout << "Password:" << std::endl;
    std::string pwd;
    std::getline(std::cin, pwd);
    std::cout << std::endl;

    // fill request
    message.set_username(name);
    message.set_password(pwd);

    std::string serialized;
    message.SerializeToString(&serialized);

    name_ = name;
    // write request
    writeToReq(stringToHex(serialized));
}

void Client::develCreateAcc() {
    Mess message;
    message.set_type(Mess::CREATE_ACCOUNT);

    std::cout << "Username:" << std::endl;
    std::string name;
    std::getline(std::cin, name);
    std::cout << "Password:" << std::endl;
    std::string pwd;
    std::getline(std::cin, pwd);
    std::cout << std::endl;

    // fill request
    message.set_username(name);
    message.set_sender(name);
    message.set_password(pwd);

    // create prekey bundle
    createKeys(message);

    std::string serialized;
    message.SerializeToString(&serialized);
    // write request
    writeToReq(stringToHex(serialized));

    name_ = name;
}

void Client::readResponse() {
    char buffer[2048];
    std::memset(buffer, 0, 2048);
    int valread = read(socket_, buffer, 2048);
    buffer[valread] = '\0';
    std::stringstream response;
    for (int i = 0; i < valread; ++i) {
        response << buffer[i];
    }

    std::string line;

    Mess message;
    // go through all responses
    while (std::getline(response, line)) {
        if (line == "") {
            continue;
        }

        message.ParseFromString(hexToString(line));
    #ifdef DEBUG
        message.PrintDebugString();
    #endif // DEBUG

        if (message.reciever() == name_) {
            gotResponse_ = true;
            switch (message.type()) {
            case Mess::AUTH:
                if (message.error()) {
                    std::cout << "Authentication failed" << std::endl;
                } else {
                    std::cout << "Authentiacion successful" << std::endl;
                    isAuthenticated_ = true;
                    }
                break;
            case Mess::MESSAGE:
                if (message.error()) {
                    std::cout << "Message was not sent" << std::endl;
                } else {
                    std::cout << "Message sent successfully" << std::endl;
                }
                break;
            case Mess::FETCH_MESSAGES:
                messages_.emplace_back(message);
                break;
            case Mess::FETCH_KEYS:
                if (!message.error()) {
                    createSecretFromKeys(line);
                } else {
                    std::cout << "User not found" << std::endl;
                }
                break;
            case Mess::INIT_MESSAGE:
                readInitial(line);
                break;
            case Mess::CREATE_ACCOUNT:
                if (message.error()) {
                    std::cout << "Account creation failed" << std::endl;
                } else {
                    std::cout << "Account creation successful" << std::endl;
                    isAuthenticated_ = true;
                    }
                break;
            default:
                break;
            }
        // if not for me, ignore it
        } else {
            //std::cerr << "Authentication failed: " << message << std::endl;
            //std::cerr << "Authentication failed: " << std::endl;
            //exit(1);
        }
    }
}

Key Client::createKeyFromHex(std::string &hexKey, bool isPublic) {
    unsigned char keyArr[32];
    auto keyVec = hexToKey(hexKey);
    for (int i = 0; i < 32; ++i) {
        keyArr[i] = keyVec[i];
    }
    Key theKey(keyArr, isPublic);
    return theKey;
}

void Client::createSecretFromKeys(const std::string &keys)
{
    Mess message;
    message.ParseFromString(hexToString(keys));

    // populate peer identity key
    Key hisPublicIdentity(&std::vector<unsigned char>(message.reqidkey().begin(), message.reqidkey().end())[0], true);

    memcpy(helperKey, hisPublicIdentity.getPublicKey().data(), 32);

    // populate peer prekey
    Key hisSignedPrekey(&std::vector<unsigned char>(message.reqprekey().begin(), message.reqprekey().end())[0], true);


    // populate signature
    unsigned char sig[64];
    std::memcpy(sig, &message.reqprekeysig().c_str()[0], 64);


    // populate one-time prekey
    Key hisOneTimeKey(&std::vector<unsigned char>(message.senotp()[0].begin(), message.senotp()[0].end())[0], true);

    //--------------------------------------
    int verified = Util::xeddsa_verify(hisPublicIdentity, &hisSignedPrekey.getPublicKey()[0], hisSignedPrekey.getPublicKey().size(), sig);


    if (verified != 0) {
        throw std::runtime_error("verification of the signature has failed");
    }

    ephemeral.generate();

    // DH1
    size_t ssize;
    unsigned char DH1[32];
    Util::ecdh(identityKey, hisSignedPrekey, DH1, &ssize);

    // DH2
    size_t ssize2;
    unsigned char DH2[32];
    Util::ecdh(ephemeral, hisPublicIdentity, DH2, &ssize2);

    // DH3
    size_t ssize3;
    unsigned char DH3[32];
    Util::ecdh(ephemeral, hisSignedPrekey, DH3, &ssize3);

    // DH4
    size_t ssize4;
    unsigned char DH4[32];
    Util::ecdh(ephemeral, hisOneTimeKey, DH4, &ssize4);

    // KDF
    unsigned char concat[128];
    for (int i = 0; i < 32; ++i) {
        concat[i] = DH1[i];
    }
    for (int i = 0; i < 32; ++i) {
        concat[32 + i] = DH2[i];
    }
    for (int i = 0; i < 32; ++i) {
        concat[64 + i] = DH3[i];
    }
    for (int i = 0; i < 32; ++i) {
        concat[96 + i] = DH4[i];
    }

    unsigned char sharedSecret[32];
    size_t sharedSecretLen = 32;

    Util::kdf(concat, 128, sharedSecret, &sharedSecretLen);


    // store shared secret
    std::pair<std::string, std::string> newPair = std::make_pair(message.username(), "");
    for (int i = 0; i < 32; ++i) {
        newPair.second += sharedSecret[i];
    }
    sharedSecrets_.push_back(newPair);

    // clear the DHs from memory
    for (int i = 0; i < 32; ++i) {
        DH1[i] = '\0';
        DH2[i] = '\0';
        DH3[i] = '\0';
        DH4[i] = '\0';
    }
}

int Client::getIndexOfSharedSecret(const std::string &name) {
    for (int i = 0; i < sharedSecrets_.size(); ++i) {
        if (sharedSecrets_[i].first == name) {
            return i;
        }
    }
    return -1;
}

void Client::develSendMessage()
{
    using namespace std::chrono_literals;
    std::cout << "\nSend message to:" << std::endl;
    std::string reciever;
    std::getline(std::cin, reciever);
    int sharedSecretIndex = getIndexOfSharedSecret(reciever);
    if (sharedSecretIndex == -1) {
        fetchBundleForInitMsg(reciever);
        while (!gotResponse_) {
            readResponse();
            std::cout << "Creating shared secret..." << std::endl;
            std::this_thread::sleep_for(1s);
            // here shared secret is created
        }
    }

    sharedSecretIndex = getIndexOfSharedSecret(reciever);

    if (sharedSecretIndex == -1) {
	    std::cout << "user \'" << reciever << "\' not found" << std::endl;
	    return;
    }


    Mess message;
    std::string serialized;
    message.set_type(Mess::INIT_MESSAGE);
    message.set_sender(name_);
    message.set_reciever(reciever);

    // populate keys
    message.set_senidkey(&identityKey.getPublicKey()[0], 32);
    message.set_senepkey(&ephemeral.getPublicKey()[0], 32);

    // AD
    sharedSecretIndex = getIndexOfSharedSecret(reciever);
    unsigned char key[32];
    for (int i = 0; i < 32; ++i) {
        key[i] = sharedSecrets_[sharedSecretIndex].second[i];
    }
    unsigned char iv[16];
    memset(iv, 0, 16);
    unsigned char ciphered[64];
    unsigned char ad[64];
    auto myIdentity = identityKey.getPublicKey();

    memcpy(ad, myIdentity.data(), 32);
    memcpy(ad + 32, helperKey, 32);

    int len = Util::aes256encrypt(ad, 64, key, iv, ciphered, 0);
    // TODO: make sure about the ciphered text len

    std::stringstream encodedCiphered;
    for(int i = 0; i < 64; ++i) {
        encodedCiphered << std::bitset<8>(ciphered[i]).to_string();
    }
    std::string cipheredAd = encodedCiphered.str();

    message.set_cipherad(cipheredAd);
    message.SerializeToString(&serialized);
    // write init key bundle
    writeToReq(stringToHex(serialized));

    // the message itself
    message.Clear();
    message.set_type(Mess::MESSAGE);
    std::cout << "Message:" << std::endl;
    std::string msg;
    std::getline(std::cin, msg);
    std::cout << std::endl;

    message.set_sender(name_);
    message.set_reciever(reciever);
    message.set_textcontent(msg);

    message.SerializeToString(&serialized);
    //std::cout << "DEBUG: sending message" << std::endl;
    writeToReq(stringToHex(serialized));
}


void Client::createKeys(Mess &message)
{
    identityKey.generate();
    signedPrekey.generate();

    // Prekey signarute
    unsigned char rnd[64];
    std::memset(rnd, 0, 64);
    unsigned char prekeySignature[64];
    std::memset(prekeySignature, 0, 64);

    Util::xeddsa_sign(identityKey, &signedPrekey.getPublicKey()[0], signedPrekey.getPublicKey().size(), rnd, prekeySignature);

    // generate one-time prekeys
    oneTimeKey1.generate();
    oneTimeKey2.generate();
    oneTimeKey3.generate();

    // populate message
    message.set_senidkey(&identityKey.getPublicKey()[0], 32);
    message.set_senprekey(&signedPrekey.getPublicKey()[0], 32);
    message.set_senprekeysig(prekeySignature, 64);
    message.add_senotp(&oneTimeKey1.getPublicKey()[0], 32);
    message.add_senotp(&oneTimeKey2.getPublicKey()[0], 32);
    message.add_senotp(&oneTimeKey3.getPublicKey()[0], 32);
}

void Client::readInitial(const std::string &req) {

    //std::cout << "DEBUG: reading initial message" << std::endl;
    Mess message;
    message.ParseFromString(hexToString(req));


    Key hisPublicId(&std::vector<unsigned char>(message.senidkey().begin(), message.senidkey().end())[0], true);

    std::string encodedEncryptedAd = message.cipherad();

    std::stringstream decode;
    for (int i = 0; i < encodedEncryptedAd.size(); i += 8) {
        std::bitset<8> bits(encodedEncryptedAd.substr(i, i+8));
        decode << static_cast<unsigned char>(bits.to_ulong());
    }
    std::string encryptedAd = decode.str();

    unsigned char ad[64];
    auto myIdPub = identityKey.getPublicKey();
    auto hisIdPub = hisPublicId.getPublicKey();

    // create AD
    for (int i = 0; i < 32; ++i) {
        ad[i] = hisIdPub[i];
    }
    for (int i = 0; i < 32; ++i) {
        ad[32 + i] = myIdPub[i];
    }

    Key hisPublicEphemeral(&std::vector<unsigned char>(message.senepkey().begin(), message.senepkey().end())[0], true);

    // DH1
    size_t ssize;
    unsigned char DH1[32];
    Util::ecdh(signedPrekey, hisPublicId, DH1, &ssize);

    // DH2
    size_t ssize2;
    unsigned char DH2[32];
    Util::ecdh(identityKey, hisPublicEphemeral, DH2, &ssize2);

    // DH3
    size_t ssize3;
    unsigned char DH3[32];
    Util::ecdh(signedPrekey, hisPublicEphemeral, DH3, &ssize3);

    // DH4
    size_t ssize4;
    unsigned char DH4[32];
    Util::ecdh(oneTimeKey1, hisPublicEphemeral, DH4, &ssize4);

    // KDF
    unsigned char concat[128];
    for (int i = 0; i < 32; ++i) {
        concat[i] = DH1[i];
    }
    for (int i = 0; i < 32; ++i) {
        concat[32 + i] = DH2[i];
    }
    for (int i = 0; i < 32; ++i) {
        concat[64 + i] = DH3[i];
    }
    for (int i = 0; i < 32; ++i) {
        concat[96 + i] = DH4[i];
    }

    unsigned char sharedSecret[32];
    size_t sharedSecretLen = 32;

    Util::kdf(concat, 128, sharedSecret, &sharedSecretLen);

    // authenticate with AD
    unsigned char decryptedAd[64];
    unsigned char iv[16];
    memset(iv, 0, 16);
    unsigned char encryptedAdArr[64];
    for (int i = 0; i < 64; ++i) {
        encryptedAdArr[i] = encryptedAd[i];
    }

    int len = Util::aes256decrypt(encryptedAdArr, 64, sharedSecret, iv, decryptedAd, 0);
    //TODO: make something with the len

#ifdef DEBUG
    std::cout << "DecryptedAd" << std::endl;
    Util::printUnsignedChar(decryptedAd, 64);
    std::cout << "FetchedAd" << std::endl;
    Util::printUnsignedChar(ad, 64);
#endif // DEBUG

    if (memcmp(decryptedAd, ad, 64)) {
	std::cout << "Authentication failed!" << std::endl;
	return;
    }

    std::cout << "Authentication successful" << std::endl;

    for (int i = 0; i < sharedSecrets_.size(); ++i) {
        if (sharedSecrets_[i].first == message.username()) {
            std::cout << "Weird stuff happening, already have shared secret with this person" << std::endl;
            std::cout << "Possible error, do not use this app anymore" << std::endl;
            return;
        }
    }
    std::pair<std::string, std::string> newPair = std::make_pair(message.username(), "");
    for (int i = 0; i < 32; ++i) {
        newPair.second += sharedSecret[i];
    }
    sharedSecrets_.push_back(newPair);

    // clear DHs from memory
    for (int i = 0; i < 32; ++i) {
        DH1[i] = '\0';
        DH2[i] = '\0';
        DH3[i] = '\0';
        DH4[i] = '\0';
    }
}

void Client::fetchBundleForInitMsg(const std::string &reciever)
{
    Mess message;
    std::string serialized;

    message.set_type(Mess::FETCH_KEYS);
    message.set_sender(name_);
    message.set_username(reciever);

    message.SerializeToString(&serialized);
    writeToReq(stringToHex(serialized));
}

void Client::develReadMessages()
{
    std::string serialized;
    Mess message;
    message.set_type(Mess::FETCH_MESSAGES);
    message.set_sender(name_);

    message.SerializeToString(&serialized);
    writeToReq(stringToHex(serialized));
}

void Client::printMessages()
{
    for (const auto & message : messages_) {
        std::cout << "\nMessage from: " << message.sender() << " to: " << message.reciever() << std::endl;
        std::cout << message.textcontent() << "\n" << std::endl;
    }

    // this is weird, messages_.clear() was giving me segfaults
    for (int i = 0; i <= messages_.size(); ++i) {
	    messages_.pop_back();
    }
}

void Client::printSharedSecrets() {
    std::cout << "sharedSecrets len " << sharedSecrets_.size() << std::endl;
    for (int i = 0; i < sharedSecrets_.size(); ++i) {
        std::cout << "Shared secret with " + sharedSecrets_[i].first << ":" << std::endl;
        for (int j = 0; j < 32; ++j) {
            std::cout << sharedSecrets_[i].second[j];
        }
        std::cout << std::endl;
    }
}

void Client::initConnection() {
    int port = 8080;
    struct sockaddr_in serv_addr;
    char buffer[2048] = {0};
    if ((socket_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "\n Socket creation error \n" << std::endl;
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        std::cerr << "\nInvalid address/ Address not supported \n" << std::endl;
        exit(1);
    }

    if (connect(socket_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "\nConnection Failed \n" << std::endl;
        exit(1);
    }
}


void Client::develRunClient()
{
    initConnection();
    using namespace std::chrono_literals;
    std::string chosen;
    while (true) {
        if (!isAuthenticated_) {

            std::cout << "Welcome, choose action:" << std::endl;
            std::cout << "1 - Create account" << std::endl;
            std::cout << "2 - Sign in" << std::endl;
            std::getline(std::cin, chosen);


            try {
                stoi(chosen);
            } catch(...) {
                std::cout << "Invalid choice" << std::endl;
                continue;
            }

            switch (std::stoi(chosen)) {
            case 1:
                develCreateAcc();
                std::this_thread::sleep_for(2s);
		readResponse();
                break;
            case 2:
                develAuth();
                while (!gotResponse_) {
                    std::cout << "waiting.." << std::endl;
                    readResponse();
                    std::this_thread::sleep_for(1s);
                    std::cout << "waiting.." << std::endl;
                }
                break;
            default:
                std::cout << "Invalid choice" << std::endl;

            }
        } else {
            std::cout << "Choose an action:" << std::endl;
            std::cout << "1 - send a message" << std::endl;
            std::cout << "2 - read messages" << std::endl;
            std::getline(std::cin, chosen);

            try {
                stoi(chosen);
            } catch(...) {
                std::cout << "Invalid choice" << std::endl;
                continue;
            }

	    switch (stoi(chosen)) {
	        case 1:
                develSendMessage();
                while (!gotResponse_) {
                    readResponse();
                    std::this_thread::sleep_for(1s);
                }
		        break;
	        case 2:
                develReadMessages();
                // Sleep for 1 second so the server has time to give you messages with the initial One
                std::cout << "Reading messages" << std::endl;
                std::this_thread::sleep_for(1s);
                std::cout << std::endl;
                for (int i = 0; i < 10 && !gotResponse_; ++i) {
                    readResponse();
                    std::this_thread::sleep_for(1s);
                }
		if (gotResponse_) {
			printMessages();
		} else {
			std::cout << "No messages found" << std::endl;
		}
		break;
            case 3:
                printSharedSecrets();
                break;
	        default:
		        std::cout << "Invalid choice" << std::endl;
            }
        }
    }
}
