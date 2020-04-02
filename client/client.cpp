#include "client.hpp"
#include <iostream>


bool Client::develFileExists(const std::string &path) {
    std::ifstream file(path);
    return file.good();
}

void Client::develSendMessage()
{
    std::cout << "\nSend message to:" << std::endl;
    std::string reciever;
    std::getline(std::cin, reciever);
    std::cout << "Message:" << std::endl;
    std::string message;
    std::getline(std::cin, message);
    std::cout << std::endl;

    std::string reqPath = "tmp_files/req/" + name_;
    std::string lockPath = reqPath + ".lock";

    while (develFileExists(lockPath)) {
        //waiting
    }

    // lock requests
    std::ifstream lock(lockPath);
    lock.close();

    std::ofstream req(reqPath, std::ofstream::out | std::ofstream::app);
    req << "sendMessage;" << name_ << ";" << reciever << ";" << message << "\n";
    req.close();

    std::remove(lockPath.c_str());
}

void Client::develReadMessages()
{
    std::string resPath = "tmp_files/res/" + name_;
    std::string lockPath = resPath + ".lock";

    while (develFileExists(lockPath)) {
        //waiting
    }

    std::ifstream lock(lockPath);
    lock.close();

    std::ifstream res(resPath);
    std::string line;
    while (std::getline(res, line)) {
        int delim = line.find_first_of(';');
        std::string command = line.substr(0, delim);

        std::string senderAndMessage = line.substr(delim + 1, line.size() - 1 - delim);
        delim = senderAndMessage.find_first_of(';');

        std::string sender = senderAndMessage.substr(0, delim);
        std::string message = senderAndMessage.substr(delim + 1, senderAndMessage.size() - 1 - delim);

        if (command == "recieveMessage") {
            messages_.emplace_back(sender, message);
        }
    }
    res.close();
    std::ofstream eraseRes(resPath, std::ofstream::out | std::ofstream::trunc);
    eraseRes.close();

    std::remove(lockPath.c_str());

    printMessages();
}

void Client::printMessages()
{
    if (messages_.size() == 0) {
        std::cout << "\nNo new messages\n" << std::endl;
        return;
    }

    for (const auto & pair : messages_) {
        std::cout << "\nMessage from: " << pair.first << std::endl;
        std::cout << pair.second << "\n" << std::endl;
    }

    messages_.clear();
}


void Client::develRunClient()
{
    while (true) {

        std::string chosen;

        std::cout << "Choose an action:" << std::endl;
        std::cout << "1 - send a message" << std::endl;
        std::cout << "2 - read messages" << std::endl;
        std::getline(std::cin, chosen);

        if (chosen == "1") {
            develSendMessage();
        } else if (chosen == "2") {
            develReadMessages();
        }
    }
}
