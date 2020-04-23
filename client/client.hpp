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

    std::vector<uint8_t> publicIk_;
    std::vector<uint8_t> privateIk_;

    unsigned char pub_identity[32];
    unsigned char pri_identity[32];

    unsigned char pub_signedPre[32];
    unsigned char pri_signedPre[32];

    unsigned char pub_empheral[32];
    unsigned char pri_empheral[32];

    unsigned char pub_oneTime[32];
    unsigned char pri_oneTime[32];

    unsigned char AD[32];
    unsigned char sharedSecret[32];
    size_t sharedSecretLen;
public:

    void createKeys(std::ofstream &output);

    void setupKey(std::string name, std::vector<uint8_t> &key);

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

    void checkInitial(const std::string &message);
};
