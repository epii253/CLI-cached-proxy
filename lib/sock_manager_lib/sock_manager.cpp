#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "lib/sock_manager_lib/sock_manager.h"
#include <lib/cache_rdb_lib/cache_rdb.h>
#include <lib/request_process_lib/request_process.h>

void SendData(int client_fd, size_t size, const char* data) {
    size_t to_send = size;
    size_t sent = 0;
    
    while (sent < to_send) {
        ssize_t n = send(client_fd,
                        data + sent,     
                        to_send - sent,       
                        0);                  
        if (n < 0) {
            std::cerr << "send error" << std::endl;
            break;
        }
        sent += static_cast<size_t>(n);
    }

}

std::string MakeHeader(const cpr::Response& responce) {
    std::string to_responce = "HTTP/1.1 " + std::to_string(responce.status_code) + responce.reason + "\r\n";
            
    for (auto& h : responce.header) {
        if (headers_to_remove.find(h.first) == headers_to_remove.end())
            to_responce += h.first + ": " + h.second + "\r\n";
    }

    return to_responce + "\r\n";
}

int InitilazeServerSocket(int port) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd < 0) {
        std::cerr << "Unsucessful soket open" << std::endl;

        return 1;
    }

    sockaddr_in addr{};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    addr.sin_family = AF_INET;

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(listen_fd);

        std::cerr << "Unsucessful soket bind" << std::endl;
        return 1;
    }

    if (listen(listen_fd, SOMAXCONN) < 0) {
        close(listen_fd);

        std::cerr << "Unsucessful listening start" << std::endl;
        return 1;
    }
    std::cout << "Server listening on port " << port << "\n";

    return listen_fd;
}

std::pair<std::string, std::string> GetMethodAndContent(char* buff) {
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

void CliendWork(int client_fd, const std::string& adress, const std::string& url) {
    RedisConnection cache;
    SocketWrapper socket(client_fd);

    int BUFF_SIZE = 8192;
    std::unique_ptr<char[]> buff(new char[BUFF_SIZE]); //RAII 

    while (true) {
        ssize_t len = recv(socket.socket_fd, buff.get(), BUFF_SIZE - 1, 0);
        buff[len] = '\0';

        if (len > 0) {
            auto inf = GetMethodAndContent(buff.get());

            if ((inf.first == "get" || inf.first == "head") && cache.CheckCache(inf.second)) { //Cache-hit
                std::vector<std::string> vec;
                cache.GetCache(inf.second, vec);

                SendData(socket.socket_fd, vec[0].size(), vec[0].data());
                SendData(socket.socket_fd, vec[1].size(), vec[1].data());
                continue;
            }

            cpr::Response responce;
            ReqestProcess::RedirectRequest(buff.get(), socket.socket_fd, url,responce); 
            responce.header["content-length"] = std::to_string(responce.text.size());
            
            std::string header = MakeHeader(responce); 

            if ((inf.first == "get" || inf.first == "head") && cached_satus.find(responce.status_code) != cached_satus.end()) {
                cache.Cache(inf.second, header, responce.text, responce);
            } 
            
            SendData(socket.socket_fd, header.size(), header.data());
            SendData(socket.socket_fd, responce.text.size(), responce.text.data());

        } else 
            break;
    }
    std::cout << "Close connection" << std::endl;
}

void Proxying(int port, std::string url) {
    SocketWrapper listen_fd(InitilazeServerSocket(port));

    while (listen_fd.socket_fd < 0 && port < 7000) {
        port++;
        listen_fd = SocketWrapper(InitilazeServerSocket(port));
    }

    if (listen_fd.socket_fd < 0) {
        std::cerr << "Cannot open socket" << std::endl;
        return;
    }

    std::thread redis(std::system, "redis-server");
    redis.detach();    

    //TODO check success ?

    std::string adress = "localhost:" + std::to_string(port);

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        SocketWrapper client_fd( accept(listen_fd.socket_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len) );

        if (client_fd.socket_fd < 0) {
            std::cerr << "Unsucessful connection" << std::endl;
            continue;
        }
        std::cout << "Accepted connection: " << client_fd.socket_fd << std::endl;

        std::thread serv(CliendWork, client_fd.socket_fd, adress, url);
        serv.detach();

        client_fd.Realese();
    }
}