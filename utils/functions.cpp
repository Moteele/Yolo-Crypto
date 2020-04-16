#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "functions.hpp"
#include "constants.hpp"

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

// initializes the user database as new
void initServer() {
        std::ofstream reqFile(REQEST_FILE_PATH, std::ofstream::out | std::ofstream::trunc);
	reqFile.close();
        std::ofstream resFile(RESPONSE_FILE_PATH, std::ofstream::out | std::ofstream::trunc);
	resFile.close();
        std::ofstream dbFile("tmp_files/db/users", std::ofstream::out | std::ofstream::trunc);
	dbFile.close();
	std::string reqPath = "tmp_files/req";
	std::vector<std::string> reqFiles = getUnlockedFiles(reqPath);
	if (reqFiles.back() != "req_master") {
	    std::remove(("tmp_files/req/" + reqFiles.back()).c_str());
	}
	reqFiles.pop_back();
	std::string resPath = "tmp_files/res";
	std::vector<std::string> resFiles = getUnlockedFiles(resPath);
	while ( ! resFiles.empty() ) {
	    if (resFiles.back() != "res_master") {
		std::remove(("tmp_files/res/" + resFiles.back()).c_str());
	    }
	    resFiles.pop_back();
	}

}

/**
 * This function is not just any hexToString function
 * but is created specificaly for storing proto buffer stuff as string needs.
 *
 * Using for anything else is highly unrecommended.
 * */
std::string hexToString(const std::string &input) {
	std::string output;

	if (((input.length()) % 2) != 0) {
        throw std::runtime_error("String is not valid length ...");
    }

	size_t cnt = (input.length()) / 2;
    for (size_t i = 0; cnt > i; ++i) {
        uint32_t s = 0;
        std::stringstream ss;
        ss << std::hex << input.substr(i * 2, 2);
        ss >> s;

        output.push_back(static_cast<unsigned char>(s));
    }

	return output;
}


/**
 * This function is not just any stringToHex function
 * but is created specificaly for storing proto buffer stuff as string needs.
 *
 * Using for anything else is highly unrecommended.
 * */
std::string stringToHex(const std::string &input) {
	std::stringstream output;
	for (int i = 0; i < input.size(); ++i) {
		output << std::hex << std::setfill('0') << std::setw(2) << (int)input[i];
	}
	output << std::endl;
	return output.str();
}
