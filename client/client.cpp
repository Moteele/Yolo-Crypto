#include "client.hpp"

#include <bitset>


bool Client::develFileExists(const std::string &path) {
    std::ifstream file(path);
    return file.good();
}

void Client::writeToReq(const std::string &req) {
    while (isFileLocked(REQEST_FILE_PATH)) {
        //waiting
    }

    // lock requests
    lockFile(REQEST_FILE_PATH);

    std::ofstream reqFile(REQEST_FILE_PATH, std::ofstream::out | std::ofstream::app);
    reqFile << req << std::endl;
    reqFile.close();
    gotResponse_ = false;
    unlockFile(REQEST_FILE_PATH);
}

void Client::develAuth() {
    std::cout << "Login:" << std::endl;
    std::string name;
    std::getline(std::cin, name);
    std::cout << "Password:" << std::endl;
    std::string pwd;
    std::getline(std::cin, pwd);
    std::cout << std::endl;

    std::string reqText = name + ";auth;" + pwd + "\n";
    name_ = name;
    writeToReq(reqText);
}

void Client::develCreateAcc() {
    std::cout << "Username:" << std::endl;
    std::string name;
    std::getline(std::cin, name);
    std::cout << "Password:" << std::endl;
    std::string pwd;
    std::getline(std::cin, pwd);
    std::cout << std::endl;

    lockFile("tmp_files/req/" + name);
    std::ofstream req("tmp_files/req/" + name);
    req << "password:" << pwd << std::endl;
    createKeys(req);
    req.close();
    unlockFile("tmp_files/req/" + name);

    name_ = name;
}

void Client::develAwaitCreation() {
    std::string expectedResponsePath = "tmp_files/res/" + name_;
    while (!develFileExists(expectedResponsePath)) {
        // wait for the response
    }
    lockFile(expectedResponsePath);
    std::ifstream resFile(expectedResponsePath);
    std::string line;
    std::getline(resFile, line);
    if (line == "Success") {
        std::cout << "Account created\n" << std::endl;
        isAuthenticated_ = true;
    } else {
        std::cout << "Something went wrong: " << line << "\n" << std::endl;
    }
    resFile.close();
    std::remove(expectedResponsePath.c_str());
    unlockFile(expectedResponsePath);
}

void Client::readResponse() {
    while (isFileLocked(RESPONSE_FILE_PATH)) {
        // waiting
    }
    lockFile(RESPONSE_FILE_PATH);
    std::ifstream resFile(RESPONSE_FILE_PATH);
    std::string line;
    std::string newName = RESPONSE_FILE_PATH + "_new";
    std::ofstream newRes(newName);
    while (std::getline(resFile, line)) {
        int delim = line.find_first_of(';');
        std::string resRec = line.substr(0, delim);
        if (resRec == name_) {
            gotResponse_ = true;
            std::string tmp = line.substr(delim + 1);
            delim = tmp.find_first_of(';');
            std::string command = tmp.substr(0, delim);
            std::string message = tmp.substr(delim + 1);
            if (command == "auth") {
                if (message != "Wrong password") {
                    isAuthenticated_ = true;
                    std::string userStr = hexToString(message);
                    userAcc user;
                    user.ParseFromString(userStr);
                    //TODO:
                } else {
                    std::cout << "Authentication failed: " << message << std::endl;
                    break;
                }
            }
            if (command == "sendMessage") {
                std::cout << message << std::endl;
            }
            if (command == "fetchMessages") {
                std::string parsedFromHex = hexToString(message);

                Mess msg;
                msg.ParseFromString(parsedFromHex);
                messages_.push_back(msg);
            }
            if (command == "fetchKeys") {
                createSecretFromKeys(message);
            }
            if (command == "initialMessage") {
                readInitial(message);
            }
        } else {
            newRes << line << std::endl;
        }
    }
    resFile.close();
    newRes.close();
    std::remove(RESPONSE_FILE_PATH.c_str());
    std::rename(newName.c_str(), RESPONSE_FILE_PATH.c_str());
    std::remove(newName.c_str());
    unlockFile(RESPONSE_FILE_PATH);
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
    int delim = keys.find_first_of(';');
    std::string rec = keys.substr(0, delim);
    std::string tmp = keys.substr(delim + 1);
    delim = tmp.find_first_of(';');
    std::string arg = tmp.substr(0, delim);

    Key hisPublicIdentity = createKeyFromHex(arg, true);

    memcpy(helperKey, hisPublicIdentity.getPublicKey().data(), 32);

    tmp = tmp.substr(delim + 1);
    delim = tmp.find_first_of(';');
    arg = tmp.substr(0, delim);

    std::string signedPrekeyHex = arg;
    Key hisSignedPrekey = createKeyFromHex(arg, true);

    tmp = tmp.substr(delim + 1);
    delim = tmp.find_first_of(';');
    arg = tmp.substr(0, delim);
    std::string prekeySignatureString = arg;

    std::stringstream decode;
    for (int i = 0; i < prekeySignatureString.size(); i += 8) {
        std::bitset<8> bits(prekeySignatureString.substr(i, i+8));
        decode << static_cast<unsigned char>(bits.to_ulong());
    }
    std::string signature = decode.str();
    unsigned char sig[64];
    for (int i = 0; i < 64; ++i) {
        sig[i] = signature[i];
    }

    arg = tmp.substr(delim + 1);

    Key hisOneTimeKey = createKeyFromHex(arg, true);

    //--------------------------------------
    int verified = Util::xeddsa_verify(hisPublicIdentity, (const unsigned char*)signedPrekeyHex.c_str(), signedPrekeyHex.size(), sig);


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


    std::pair<std::string, std::string> newPair = std::make_pair(rec, "");
    for (int i = 0; i < 32; ++i) {
        newPair.second += sharedSecret[i];
    }
    sharedSecrets_.push_back(newPair);

//     //TODO: delete stuff not needed anymore
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
            std::this_thread::sleep_for(1s);
            // here shared secret is created
        }
    }

    std::stringstream initialMessageSS;
    initialMessageSS << name_ << ";initialMessage;" << reciever << ";";
    auto myPublicId = identityKey.getPublicKey();
    std::string myPublicIdHex = keyToHex(myPublicId);
    initialMessageSS << myPublicIdHex << ";";

    auto myPublicEphemeral = ephemeral.getPublicKey();
    std::string myPublicEphemHex = keyToHex(myPublicEphemeral);
    initialMessageSS << myPublicEphemHex << ";";

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

    initialMessageSS << cipheredAd;


    writeToReq(initialMessageSS.str());

    std::cout << "Message:" << std::endl;
    std::string message;
    std::getline(std::cin, message);
    std::cout << std::endl;

    std::string reqText = name_ + ";sendMessage;" + reciever + ";" + message; //TODO: the message should be encrypted here
    writeToReq(reqText);
}


void Client::createKeys(std::ofstream &output)
{
    identityKey.generate();
    auto privIdenPreStr = identityKey.getPrivateKey();
    std::string privateIdentityKeyString = keyToHex(privIdenPreStr);
    auto pubIdenPreStr = identityKey.getPublicKey();
    std::string publicIdentityKeyString = keyToHex(pubIdenPreStr);

    signedPrekey.generate();
    auto privSigPreStr = signedPrekey.getPrivateKey();
    std::string privateSignedPrekeyString = keyToHex(privSigPreStr);
    auto pubSigPreStr = signedPrekey.getPublicKey();
    std::string publicSignedPrekeyString = keyToHex(pubSigPreStr);

    // Prekey signarute
    unsigned char rnd[64];
    unsigned char prekeySignature[64];

    Util::xeddsa_sign(&privIdenPreStr[0], (const unsigned char*)publicSignedPrekeyString.c_str(), publicSignedPrekeyString.size(), rnd, prekeySignature);

    std::stringstream encoded;
    for(int i = 0; i < 64; ++i) {
        encoded << std::bitset<8>(prekeySignature[i]).to_string();
    }
    std::string prekeySignatureString = encoded.str();

    oneTimeKey1.generate();
    auto privOne1PreStr = oneTimeKey1.getPrivateKey();
    std::string privateOneTime1String = keyToHex(privOne1PreStr);
    auto pubOne1PreStr = oneTimeKey1.getPublicKey();
    std::string publicOneTime1String = keyToHex(pubOne1PreStr);

    oneTimeKey2.generate();
    auto privOne2PreStr = oneTimeKey2.getPrivateKey();
    std::string privateOneTime2String = keyToHex(privOne2PreStr);
    auto pubOne2PreStr = oneTimeKey2.getPublicKey();
    std::string publicOneTime2String = keyToHex(pubOne2PreStr);

    oneTimeKey3.generate();
    auto privOne3PreStr = oneTimeKey3.getPrivateKey();
    std::string privateOneTime3String = keyToHex(privOne3PreStr);
    auto pubOne3PreStr = oneTimeKey3.getPublicKey();
    std::string publicOneTime3String = keyToHex(pubOne3PreStr);

    output << "privateId:" << privateIdentityKeyString << std::endl;
    output << "publicId:" << publicIdentityKeyString << std::endl;
    output << "privateSignedPrekey:" << privateSignedPrekeyString << std::endl;
    output << "publicSignedPrekey:" << publicSignedPrekeyString << std::endl;
    output << "prekeySignature:" << prekeySignatureString << std::endl;

    output << "privateOnetime:" << privateOneTime1String << std::endl;
    output << "publicOnetime:" << publicOneTime1String << std::endl;
    output << "privateOnetime:" << privateOneTime2String << std::endl;
    output << "publicOnetime:" << publicOneTime2String << std::endl;
    output << "privateOnetime:" << privateOneTime3String << std::endl;
    output << "publicOnetime:" << publicOneTime3String << std::endl;
}

void Client::readInitial(const std::string &message) {
    int delim = message.find_first_of(';');
    std::string name = message.substr(0, delim);
    std::string tmp = message.substr(delim + 1);
    delim = tmp.find_first_of(';');

    std::string hisPublicHex = tmp.substr(0, delim);

    Key hisPublicId = createKeyFromHex(hisPublicHex, true);

    tmp = tmp.substr(delim + 1);
    delim = tmp.find_first_of(';');
    std::string hisEphHex = tmp.substr(0, delim);
    std::string encodedEncryptedAd = tmp.substr(delim + 1);

    std::stringstream decode;
    for (int i = 0; i < encodedEncryptedAd.size(); i += 8) {
        std::bitset<8> bits(encodedEncryptedAd.substr(i, i+8));
        decode << static_cast<unsigned char>(bits.to_ulong());
    }
    std::string encryptedAd = decode.str();

    unsigned char ad[64];
    auto myIdPub = identityKey.getPublicKey();
    auto hisIdPub = hisPublicId.getPublicKey();
    for (int i = 0; i < 32; ++i) {
        ad[i] = hisIdPub[i];
    }
    for (int i = 0; i < 32; ++i) {
        ad[32 + i] = myIdPub[i];
    }

    Key hisPublicEphemeral = createKeyFromHex(hisEphHex, true);

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

    if (memcmp(decryptedAd, ad, 64))
	std::cout << "Authentication failed!" << std::endl;
	//return;
    std::cout << "Authentication successful" << std::endl;

    for (int i = 0; i < sharedSecrets_.size(); ++i) {
        if (sharedSecrets_[i].first == name) {
            std::cout << "Weird stuff happening, already have shared secret with this person" << std::endl;
            std::cout << "Possible error, do not use this app anymore" << std::endl;
            return;
        }
    }
    std::pair<std::string, std::string> newPair = std::make_pair(name, "");
    for (int i = 0; i < 32; ++i) {
        newPair.second += sharedSecret[i];
    }
    sharedSecrets_.push_back(newPair);
}

void Client::fetchBundleForInitMsg(const std::string &reciever)
{
    std::string reqText = name_ + ";fetchKeys;" + reciever;
    writeToReq(reqText);
}

void Client::develReadMessages()
{
    std::string reqText = name_ + ";fetchMessages";
    writeToReq(reqText);
}

void Client::printMessages()
{
    for (const auto & message : messages_) {
        std::cout << "\nMessage from: " << message.sender() << " to: " << message.reciever() << std::endl;
        std::cout << message.text() << "\n" << std::endl;
    }

    messages_.clear();
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


void Client::develRunClient()
{
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
                develAwaitCreation();
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
		    // Sleep for 5 seconds so the server has time to give you messages with the initial One
		    std::cout << "Reading messages" << std::endl;
		    for (int i = 0; i < 5; ++i) {
			std::cout << "." << std::flush;
			std::this_thread::sleep_for(1s);
		    }
		    std::cout << std::endl;
		    while (!gotResponse_) {
			readResponse();
			std::this_thread::sleep_for(1s);
		    }
		    printMessages();
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
