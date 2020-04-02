#include "client.hpp"
#include <iostream>

int main()
{
    std::cout << "Client name:" << std::endl;
    std::string name;
    std::cin >> name;

    Client client(name);

    client.develRunClient();
}
