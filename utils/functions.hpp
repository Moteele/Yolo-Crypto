//#pragma once

#include <string>
#include <vector>


// File handlers
void lockFile(const std::string &path);

void unlockFile(const std::string &path);

bool isFileLocked(const std::string &path);

std::vector<std::string> getUnlockedFiles(const std::string &dirPath);

void initServer();

std::string hexToString(const std::string &input);

std::string stringToHex(const std::string &input);

std::string keyToHex(std::vector<uint8_t> &key);

std::vector<uint8_t> hexToKey(std::string &input);
