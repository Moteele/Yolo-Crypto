//#pragma once

#include <string>
#include <vector>


// File handlers
void lockFile(const std::string &path);

void unlockFile(const std::string &path);

bool isFileLocked(const std::string &path);

std::vector<std::string> getUnlockedFiles(const std::string &dirPath);
