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


    // std::string publicPk_;
    // std::string privatePk_;

    // std::vector<std::string> publicOneTime_;
    // std::vector<std::string> privateOneTime_;

public:

    void createKeys(std::ofstream &output);

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
};
