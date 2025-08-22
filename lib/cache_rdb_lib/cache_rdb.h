#pragma once

#include <sw/redis++/redis++.h>
#include <string>
#include <vector>
#include <memory>
#include <cpr/cpr.h>
#include <cctype>

int64_t ExtractTime(const std::string& headers);

const std::string cache_control_header = "cache-control";
const std::string max_age_key = "max-age";

class RedisConnection {
public:
    RedisConnection();

    void Cache(const std::string& key, const std::string& headers, const std::string& body, const cpr::Response& responce);
    void GetCache(const std::string& key, std::vector<std::string>& getter);
    bool CheckCache(const std::string& key);
private:
    std::unique_ptr<sw::redis::Redis> connection;
    bool opened = true;
};