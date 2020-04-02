#include <string>
#include <fstream>
#include <vector>
#include <string>


class Client
{
    std::string name_;
    std::vector<std::pair<std::string, std::string>> messages_;

public:

    Client(std::string &name) : name_(name)
    {
        std::ofstream req("tmp_files/req/" + name);
        std::ofstream res("tmp_files/res/" + name);
        req.close();
        res.close();
    }

    void develSendMessage();

    void develReadMessages();

    void develRunClient();

    bool develFileExists(const std::string &path);

    void printMessages();
};
