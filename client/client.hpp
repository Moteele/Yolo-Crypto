#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <string>

#include "../utils/userAcc.pb.h"
#include "../utils/mess.pb.h"

#include <thread>
#include <chrono>
#include "../utils/functions.hpp"
#include "../utils/constants.hpp"
#include "../utils/key.hpp"
#include "../utils/util.hpp"

class Client
{
    std::string name_;
    std::vector<Mess> messages_;

    bool isAuthenticated_ = false;
    bool gotResponse_ = false;

    Key identityKey = {};
    Key signedPrekey = {};

    Key oneTimeKey1 = {};
    Key oneTimeKey2 = {};
    Key oneTimeKey3 = {};

    Key ephemeral = {};

    // !!! the second string is a shared secret and it's a BINARY string !!!
    std::vector<std::pair<std::string, std::string>> sharedSecrets_ = {};

    unsigned char helperKey[32];

public:

    Client() = default;

    int getIndexOfSharedSecret(const std::string &name);

    //void createKeys(std::ofstream &output);

    void createKeys(Mess &message);

    void develAuth();

    void develAwaitCreation();

    void develCreateAcc();

    void fetchBundleForInitMsg(const std::string &reciever);

    void createSecretFromKeys(const std::string &keys);

    void readResponse();

    void writeToReq(const std::string &req);

    void develSendMessage();

    void develReadMessages();

    void develRunClient();

    bool develFileExists(const std::string &path);

    void printMessages();

    void readInitial(const std::string &req);

    Key createKeyFromHex(std::string &hexKey, bool isPublic);

    void printSharedSecrets(); // only for devel purposes, should be deleted later
};
