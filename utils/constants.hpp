#pragma once

#include <string>

enum REQUESTS {
    AUTH,
    READ_MSG,
    SEND_MSG,
};

const std::string REQEST_FILE_PATH = "tmp_files/req/req_master";

const std::string RESPONSE_FILE_PATH = "tmp_files/res/res_master";
