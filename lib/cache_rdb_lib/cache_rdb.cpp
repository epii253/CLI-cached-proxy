#include <iostream>

#include <lib/cache_rdb_lib/cache_rdb.h>

int64_t ExtractTime(const std::string& keys) {

    auto ind = keys.find(max_age_key); 
    if (ind == keys.npos)
        return -1;

    ind += keys.size() + 1;
    int64_t result = 0;

    while (ind < keys.size() && isdigit(keys[ind])) {
        result *= 10;
        result += keys[ind] - '0';
        ++ind;
    }
    return result;
}

RedisConnection::RedisConnection() {
    try {
        connection.reset(new sw::redis::Redis("tcp://127.0.0.1:6379"));

    } catch (const sw::redis::Error &e) {
        std::cerr << "Cannot open connection " << e.what() << std::endl; 
        opened = false;
    }
    
}

bool RedisConnection::CheckCache(const std::string& key) {
    if (!opened)
        return false;

    return connection->exists(key) > 0;
}

void RedisConnection::Cache(const std::string& key, const std::string& headers, const std::string& body, const cpr::Response& responce) {
    if (!opened || responce.header.find(cache_control_header) == responce.header.end())
        return;

    auto keys = responce.header.find(cache_control_header)->second;
    if (keys.find("private") != keys.npos) {
        return;
    }

    int64_t max_age = ExtractTime(keys);

    connection->del(key);
    
    connection->rpush(key, {headers, body});
    
    if (max_age != -1)
        connection->expire(key, max_age);
}

void RedisConnection::GetCache(const std::string& key, std::vector<std::string>& getter) { 
    if (!this->CheckCache(key)) 
        return;
    
    connection->lrange(key, 0, -1, std::back_inserter(getter));
}