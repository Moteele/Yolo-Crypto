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

class Client
{
    std::string name_;
    std::vector<Mess> messages_;

    bool isAuthenticated_ = false;
    bool gotResponse_ = false;

public:

    void develAuth();

    void develCreateAcc();

    void develAwaitCreation();

    void readResponse();

    void writeToReq(const std::string &req);

    void develSendMessage();

    void develReadMessages();

    void develRunClient();

    bool develFileExists(const std::string &path);

    void printMessages();
};
