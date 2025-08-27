#pragma once

#include <cpr/cpr.h>
#include <sw/redis++/redis++.h>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <set>
#include <cctype>
#include <utility>

int64_t ExtractTime(const std::string& headers);

std::string MakeHeader(const cpr::Response& responce);

const std::string cache_control_header = "cache-control";
const std::string max_age_key = "max-age";
const std::set<std::string> headers_to_remove{"strict-transport-security", "alt-svc", "upgrade-insecure-requests", \
     "set-cookie", "content-security-policy", "x-powered-by", "cookie"}; 

const std::set<int> cached_satus{200, 203, 204, 206, 300, 301, 404, 405, 410, 414, 501};


class RedisConnection {
public:
    RedisConnection(std::string origin, int port) :
    origin_(origin)
    {
        try {
            connection_.reset(new sw::redis::Redis("tcp://127.0.0.1:" + std::to_string(port)));

        } catch (const sw::redis::Error &e) {
            std::cerr << "Cannot open connection_ " << e.what() << std::endl; 
            opened = false;
        }
    }

    void LookUpCache(std::string_view request, std::string& header, std::string& body, int fd);
    void GetCache(const std::string& key, std::vector<std::string>& getter);
    bool CheckCache(const std::string& key);

private:
    std::string GetEtagByKey(const std::string& key);
    std::pair<std::string, std::string> GetMethodAndContent(const char* buff);
    void WriteToCache(const std::string& key, const std::string& head, const std::string& body, const std::string& etag, const cpr::Response& responce);

    std::unique_ptr<sw::redis::Redis> connection_;
    bool opened = true;
    std::string origin_;
};
