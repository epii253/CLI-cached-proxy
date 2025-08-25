#include <iostream>
#include <optional>

#include <lib/cache_rdb_lib/cache_rdb.h>
#include <lib/request_process_lib/request_process.h>

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

std::string MakeHeader(const cpr::Response& responce) {
    std::string to_responce = "HTTP/1.1 " + std::to_string(responce.status_code) + responce.reason + "\r\n";
            
    for (auto& h : responce.header) {
        if (headers_to_remove.find(h.first) == headers_to_remove.end())
            to_responce += h.first + ": " + h.second + "\r\n";
    }

    return to_responce + "\r\n";
}

bool RedisConnection::CheckCache(const std::string& key) {
    if (!opened)
        return false;

    return connection_->exists(key) > 0;
}

std::string RedisConnection::GetEtagByKey(const std::string& key) {
    std::optional<std::string> result;

    if (this->CheckCache(key))
        result = connection_->lindex(key, 2); 

    return result.has_value() ? *result : "";
}

void RedisConnection::LookUpCache(std::string_view request, std::string& header, std::string& body, int fd) {
    auto inf = GetMethodAndContent(request.data());
    std::string etag = GetEtagByKey(inf.second);

    cpr::Response responce;
    if ((inf.first == "get" || inf.first == "head") && CheckCache(inf.second) && \
            ReqestProcess::ValidateByEtag(etag, responce, fd, origin_, inf.first, inf.second)) {
        std::vector<std::string> vec;
        this->GetCache(inf.second, vec);
        vec[0] = header;
        vec[1] = body;
        
    } else {
        ReqestProcess::RedirectRequest(request.data(), fd, origin_, responce);
        responce.header["content-length"] = std::to_string(responce.text.size());

        header = MakeHeader(responce);
        body = responce.text;
        
        if ((inf.first == "get" || inf.first == "head") && cached_satus.contains(responce.status_code)) {
            this->WriteToCache(inf.second, header, body, etag, responce);
        }
    }
}

void RedisConnection::WriteToCache(const std::string& key, const std::string& head, const std::string& body, const std::string& etag, const cpr::Response& responce) {
    if (!opened || responce.header.find(cache_control_header) == responce.header.end())
        return;

    auto keys = responce.header.find(cache_control_header)->second;
    if (keys.find("private") != keys.npos) {
        return;
    }

    int64_t max_age = ExtractTime(keys);

    connection_->del(key);
    
    connection_->rpush(key, {head, body, etag});
    
    if (max_age != -1)
        connection_->expire(key, max_age);
}


void RedisConnection::GetCache(const std::string& key, std::vector<std::string>& getter) { 
    if (!this->CheckCache(key)) 
        return;
    
    connection_->lrange(key, 0, -1, std::back_inserter(getter));
}

std::pair<std::string, std::string> RedisConnection::GetMethodAndContent(const char* buff) {
    std::pair<std::string, std::string> result;
    std::string_view str(buff);

    int ind = 0;
    while (ind < str.size() && str[ind] != ' ') {
        result.first += std::tolower(str[ind]);
        ++ind;
    }

    if (str[ind] != ' ')
        return {"", ""};
    ++ind;

    while (ind < str.size() && str[ind] != ' ') {
        result.second += str[ind++];
    }

    return result;
}
