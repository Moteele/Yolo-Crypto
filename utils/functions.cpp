#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include "functions.h"

void lockFile(const std::string &path) {
    std::ofstream lockFile(path + ".lock");
    lockFile.close();
};

void unlockFile(const std::string &path) {
    std::string file = path + ".lock";
    std::remove(file.c_str());
};

bool isFileLocked(const std::string &path) {
    std::ifstream file(path + ".lock");
    return file.good();
}

std::vector<std::string> getUnlockedFiles(const std::string &dirPath) {
    std::vector<std::string> result;

	struct dirent *entry;
    DIR *dir = opendir(dirPath.c_str());
    if (dir == NULL) {
        return result;
    }
    while ((entry = readdir(dir)) != NULL) {
		std::string fname = entry->d_name;

		if (fname == "." || fname == ".." || fname == "_test") {
			continue;
		}
		if (fname.size() < 5) {
			result.push_back(fname);
			continue;
		}
		if (fname.substr(fname.size() - 4, 4) != "lock") {
			result.push_back(fname);
		}
    }
    closedir(dir);
    return result;
}
