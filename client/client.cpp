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
    req << pwd << std::endl;
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
                if (message == "Success") {
                    isAuthenticated_ = true;
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

void Client::develSendMessage()
{
    std::cout << "\nSend message to:" << std::endl;
    std::string reciever;
    std::getline(std::cin, reciever);
    std::cout << "Message:" << std::endl;
    std::string message;
    std::getline(std::cin, message);
    std::cout << std::endl;

    std::string reqText = name_ + ";sendMessage;" + reciever + ";" + message;
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
