#include <string_view>
#include <memory.h>
#include <iostream>
#include <cctype>
#include <cstring>
#include <chrono>

#include <lib/request_process_lib/request_process.h>

void ReqestProcess::GzipCompress(std::string &output, const std::string &input, int level = Z_BEST_COMPRESSION) {
    if (input.empty()) 
        return;

    z_stream zs;
    std::memset(&zs, 0, sizeof(zs));

    int windowBits = 15 + 16;
    if (deflateInit2(&zs, level, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        throw std::runtime_error("deflateInit2 failed");
    }

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
    zs.avail_in = static_cast<uInt>(input.size());

    const size_t chunkSize = 5242880;
    output.reserve(input.size() / 2);

    unsigned char outbuf[chunkSize];

    int ret;
    do {
        zs.next_out = outbuf;
        zs.avail_out = sizeof(outbuf);

        ret = deflate(&zs, zs.avail_in ? Z_NO_FLUSH : Z_FINISH);
        if (ret == Z_STREAM_ERROR) {
            deflateEnd(&zs);
            throw std::runtime_error("deflate failed (Z_STREAM_ERROR)");
        }

        size_t have = sizeof(outbuf) - zs.avail_out;
        output.append(reinterpret_cast<char*>(outbuf), have);
    } while (ret != Z_STREAM_END);

    deflateEnd(&zs);
}

void ReqestProcess::RedirectRequest(const char* buffer, int client_fd, const std::string& origin, cpr::Response& responce) {
    std::string_view req(buffer);

    int ind = 0;
    std::string method;
    std::string content;

    while (ind < req.size() && std::isalpha(req[ind]) ) {
        method.push_back(std::tolower(req[ind++]));
    }
    ++ind;
    
    while (ind < req.size() && req[ind] != ' ') {
        content.push_back(std::tolower(req[ind++]));
    }
    ++ind;

    while (ind < req.size() && std::isprint(req[ind]) && !std::isspace(req[ind])) {
        ++ind;
    }
    while (ind < req.size() && !(std::isprint(req[ind]) && !std::isspace(req[ind]))) {
        ++ind;
    }   

    cpr::Url full_url{origin + content};
    cpr::Header headers;
    
    while (ind < req.size()) {
        std::string field;
        std::string val;

        while (ind < req.size() && req[ind] != ':') {
            field.push_back(std::tolower(req[ind++]));
        }
        ind += 2; 

        while (ind < req.size() && req[ind] != '\n') {
            val.push_back(req[ind++]);
        }
        
        while (ind < req.size() && !std::isalnum(req[ind])) {
            ++ind;
        }
        
        if (ReqestProcess::banned_headers.find(field) != ReqestProcess::banned_headers.end() || field.empty())
            continue;

        if (field == "host") {
            val = origin.substr(origin.find("//") + 2);
        }

        headers[field] = val;        
    }

    if (method == ReqestProcess::get_header) {            
        responce = cpr::Get(cpr::Url{full_url}, headers, cpr::AcceptEncoding{cpr::AcceptEncodingMethods::disabled});//, 

    } else if (method == ReqestProcess::head_header) {
        responce = cpr::Head(full_url, headers, cpr::AcceptEncoding{cpr::AcceptEncodingMethods::disabled}); //TODO

    } else if (method == ReqestProcess::post_header) { //TODO
        int BUFF_SIZE = 8192;
        char* buff = new char[BUFF_SIZE];
        
        //responce = cpr::Post();
        while (true) { //send ???
            ssize_t len = recv(client_fd, buff, BUFF_SIZE - 1, 0);
            buff[len] = '\0';
        }
    }
    
}
