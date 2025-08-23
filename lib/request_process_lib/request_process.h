#pragma once

#include <string>
#include <cpr/cpr.h>
#include <zlib.h>
#include <set>

namespace ReqestProcess {
enum class Requests {
    Get,
    Head,
    Post,
    Put,
    Delete,
    Connect,
    Options,
    Trace
};

void GzipCompress(std::string &output, const std::string &input, int level);

void RedirectRequest(const char* buffer, int client_fd, const std::string& origin, cpr::Response& responce);

const std::string get_header = "get";
const std::string head_header = "head";
const std::string post_header = "post";

const std::set<std::string> banned_headers {"referer", "connection", "origin"};
}