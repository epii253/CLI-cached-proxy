#include <string_view>
#include <memory.h>
#include <iostream>
#include <cctype>
#include <cstring>
#include <chrono>

#include <lib/request_process_lib/request_process.h>

bool ReqestProcess::ValidateByEtag(const std::string& etag, cpr::Response& responce, int client_fd, const std::string& origin, const std::string& method, const std::string& content) {
    std::string request = method + " " + content + "\r\n" + "If-None-Match: " + '"' + etag + '"' + "\r\n" + "\r\n";

    RedirectRequest(request.c_str(), client_fd, origin, responce);
    return responce.status_code == 304;
}

int ReqestProcess::ExtractInf(int index, const std::string_view& request, std::string& method, std::string& content) {
    
    while (index < request.size() && std::isalpha(request[index]) ) { //Parse method
        method.push_back(std::tolower(request[index++]));
    }
    ++index;
    
    while (index < request.size() && request[index] != ' ') { // parse content
        content.push_back(std::tolower(request[index++]));
    }
    ++index;

    while (index < request.size() && std::isprint(request[index]) && !std::isspace(request[index])) { //skip protocol
        ++index;
    }
    
    while (index < request.size() && !(std::isprint(request[index]) && !std::isspace(request[index]))) { // skip /r/n
        ++index;
    }   

    return index;
}

void ReqestProcess::ParseHeaders(cpr::Header& headers, const std::string_view& request, int& index, const std::string& origin) {
    while (index < request.size()) {
        std::string field;
        std::string val;

        while (index < request.size() && request[index] != ':') {
            field.push_back(std::tolower(request[index++]));
        }
        index += 2; 

        while (index < request.size() && request[index] != '\n') {
            val.push_back(request[index++]);
        }
        index += 1;
        
        if (ReqestProcess::banned_headers.find(field) != ReqestProcess::banned_headers.end() || field.empty())
            continue;

        if (field == "host") {
            val = origin.substr(origin.find("//") + 2);

        } else if (field == "origin") {
            val = origin;
        }


        headers[field] = val;    

        if (std::isspace(request[index])) {
            index += 2; // skip \r\n
            break;
        }
    }
}

void ReqestProcess::RedirectRequest(const char* buffer, int client_fd, const std::string& origin, cpr::Response& responce) {
    std::string_view req(buffer);
    std::string method;
    std::string content;

    int ind = ExtractInf(0, req, method, content);

    cpr::Url full_url{origin + content};
    cpr::Header headers;

    ParseHeaders(headers, req, ind, origin);
    

    if (method == ReqestProcess::get_header) {            
        responce = cpr::Get(cpr::Url{full_url}, headers, cpr::AcceptEncoding{cpr::AcceptEncodingMethods::disabled});
        
    } else if (method == ReqestProcess::head_header) {
        responce = cpr::Head(cpr::Url{full_url}, headers, cpr::AcceptEncoding{cpr::AcceptEncodingMethods::disabled}); //similar to Get

    } else if (method == ReqestProcess::post_header) { 
        int BUFF_SIZE = std::strtoll(headers["content-length"].c_str(), nullptr, 10);
        std::unique_ptr<char[]> buff(new char[BUFF_SIZE]); //RAII 
        int buff_ind = 0;

        while (buff_ind < BUFF_SIZE && ind < req.size()) { //Copy content to Body-buffer
            buff[buff_ind] = req[ind];

            ++buff_ind;
            ++ind; 
        }
        
        responce = cpr::Post(cpr::Url{full_url}, headers, cpr::Body{buff.get()},cpr::AcceptEncoding{cpr::AcceptEncodingMethods::disabled});
    }
    
}
