#include "client.hpp"




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
                std::cout << "Got following keys from BE:" << std::endl;
                std::cout << message << std::endl;
                createSecretFromKeys(message);
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

void Client::createSecretFromKeys(const std::string &keys)
{
    //TODO: sometimes it is a signature
    int delim = keys.find_first_of(';');
    std::string publicIdString = hexToKey(keys.substr(0, delim));
    std::string tmp = keys.substr(delim + 1);
    delim = tmp.find_first_of(';');
    std::string signedPrekeyString = hexToKey(tmp.substr(0, delim));
    tmp = tmp.substr(delim + 1);
    delim = tmp.find_first_of(';');
    std::string prekeySignatureString = hexToKey(tmp.substr(0, delim));
    std::string oneTimeString = hexToKey(tmp.substr(delim + 1));
}

void Client::develSendMessage()
{
    using namespace std::chrono_literals;
    std::cout << "\nSend message to:" << std::endl;
    std::string reciever;
    std::getline(std::cin, reciever);

    //TODO: only if not done yet
    fetchBundleForInitMsg(reciever);

    while (!gotResponse_) {
        readResponse();
        std::this_thread::sleep_for(1s);
    }




    std::cout << "Message:" << std::endl;
    std::string message;
    std::getline(std::cin, message);
    std::cout << std::endl;

    std::string reqText = name_ + ";sendMessage;" + reciever + ";" + message;
    writeToReq(reqText);
}

void Client::createKeys(std::ofstream &output)
{
    // Identity Key
    Key identity;
    std::vector<uint8_t> privateIdentity = identity.getPrivateKey();
    std::vector<uint8_t> publicIdentity = identity.getPublicKey();

    std::string privateIdentityKeyString = keyToHex(privateIdentity);
    std::string publicKeyIdentityString = keyToHex(publicIdentity);

    // Signed prekey
    Key signedPrekey;
    std::vector<uint8_t> privateSignedPrekey = signedPrekey.getPrivateKey();
    std::vector<uint8_t> publicSignedPrekey = signedPrekey.getPublicKey();

    std::string privateSignedPrekeyString = keyToHex(privateSignedPrekey);
    std::string publicSignedPrekeyString = keyToHex(publicSignedPrekey);

    // Prekey signarute
    unsigned char rnd[64];
    unsigned char prekeySignature[64];

    std::string prekeySignatureString = stringToHex((char *)prekeySignature);

    Util::xeddsa_sign(&privateIdentity[0], (const unsigned char*)publicSignedPrekeyString.c_str(), publicSignedPrekeyString.size(), rnd, prekeySignature);

    // One time prekeys
    Key oneTime1;
    std::vector<uint8_t> privateOneTime1 = oneTime1.getPrivateKey();
    std::vector<uint8_t> publicOneTime1 = oneTime1.getPublicKey();
    std::string privateOneTime1String = keyToHex(privateOneTime1);
    std::string publicOneTime1String = keyToHex(publicOneTime1);

    Key oneTime2;
    std::vector<uint8_t> privateOneTime2 = oneTime2.getPrivateKey();
    std::vector<uint8_t> publicOneTime2 = oneTime2.getPublicKey();
    std::string privateOneTime2String = keyToHex(privateOneTime2);
    std::string publicOneTime2String = keyToHex(publicOneTime2);

    Key oneTime3;
    std::vector<uint8_t> privateOneTime3 = oneTime3.getPrivateKey();
    std::vector<uint8_t> publicOneTime3 = oneTime3.getPublicKey();
    std::string privateOneTime3String = keyToHex(privateOneTime3);
    std::string publicOneTime3String = keyToHex(publicOneTime3);

    output << "privateId:" << privateIdentityKeyString << std::endl;
    output << "publicId:" << publicKeyIdentityString << std::endl;
    output << "privateSignedPrekey:" << privateSignedPrekeyString << std::endl;
    output << "publicSignedPrekey:" << publicSignedPrekeyString << std::endl;
    output << "prekeySignature:" << prekeySignatureString << std::endl;
    output << "privateOnetime:" << privateOneTime1String << std::endl;
    output << "publicOneTime:" << publicOneTime1String << std::endl;
    output << "privateOnetime:" << privateOneTime2String << std::endl;
    output << "publicOneTime:" << publicOneTime2String << std::endl;
    output << "privateOnetime:" << privateOneTime3String << std::endl;
    output << "publicOneTime:" << publicOneTime3String << std::endl;

    std::cout << "MyPublicId:" << publicKeyIdentityString << std::endl;
    std::cout << "MySignedPrekey:" << publicSignedPrekeyString << std::endl;
    std::cout << "signature:" << prekeySignatureString << std::endl;
    std::cout << "MyOneTime:" << publicOneTime1String << std::endl;
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
                while (!gotResponse_) {
                    readResponse();
                    std::this_thread::sleep_for(1s);
                }
                printMessages();
		        break;
	        default:
		        std::cout << "Invalid choice" << std::endl;
            }
        }
    }
}
