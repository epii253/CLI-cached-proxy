#pragma once

#include <sw/redis++/redis++.h>
#include <string>
#include <vector>
#include <memory>
#include <cctype>

int64_t ExtractTime(const std::string& headers);

const std::string age_header = "age:";



class RedisConnection {
public:
    RedisConnection();

    void Cache(const std::string& key, const std::string& headers, const std::string& body);
    void GetCache(const std::string& key, std::vector<std::string>& getter);
    bool CheckCache(const std::string& key);
private:
    std::unique_ptr<sw::redis::Redis> connection;
    bool opened = true;
};