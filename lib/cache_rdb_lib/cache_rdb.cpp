#include <iostream>

#include <lib/cache_rdb_lib/cache_rdb.h>

int64_t ExtractTime(const std::string& headers) {

    auto ind = headers.find(age_header); 
    if (ind == headers.npos)
        return -1;
    ind += age_header.size() + 1;
    int64_t result = 0;

    while (ind < headers.size() && isdigit(headers[ind])) {
        result *= 10;
        result += headers[ind] - '0';
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

void RedisConnection::Cache(const std::string& key, const std::string& headers, const std::string& body) {
    if (!opened)
        return;

    connection->del(key);
    
    connection->rpush(key, {headers, body});
    int64_t max_age = ExtractTime(headers);
    
    if (max_age != -1)
        connection->expire(key, max_age);
}

void RedisConnection::GetCache(const std::string& key, std::vector<std::string>& getter) { 
    if (!this->CheckCache(key)) 
        return;
    
    connection->lrange(key, 0, -1, std::back_inserter(getter));
}