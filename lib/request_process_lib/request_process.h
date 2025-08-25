#pragma once

#include <string>
#include <cpr/cpr.h>
#include <set>
#include <string_view>

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
int ExtractInf(int index, const std::string_view& request, std::string& method, std::string& content);
void ParseHeaders(cpr::Header& headers, const std::string_view& request, int& index, const std::string& origin);
bool ValidateByEtag(const std::string& etag, cpr::Response& responce, int client_fd, const std::string& origin, const std::string& method, const std::string& content);

const std::string get_header = "get";
const std::string head_header = "head";
const std::string post_header = "post";

const std::set<std::string> banned_headers {"referer", "proxy-authenticat", "proxy-authorization" , "keep-alive", "сonnection", \
                                            "te", "trailers", "upgrade"}; // TODO: check more headers
}
