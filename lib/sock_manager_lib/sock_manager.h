#pragma once

#include <string>
#include <cpr/cpr.h>
#include <set>
#include <utility>

const std::set<std::string> headers_to_remove{"strict-transport-security", "alt-svc", "upgrade-insecure-requests", \
     "set-cookie", "content-security-policy", "x-powered-by"}; 

const std::set<int> cached_satus{200, 203, 204, 206, 300, 301, 404, 405, 410, 414, 501};
 

struct SocketWrapper {
     int socket_fd = -1;

     SocketWrapper(int fd) :
     socket_fd(fd)
     {}

     ~SocketWrapper() {
          if (socket_fd > 0)
               close(socket_fd);
     }
};

void SendData(int client_fd, size_t size, const char* data);
void Proxying(int port, std::string url);
int InitilazeServerSocket(int port);

void CliendWork(int client_fd, const std::string& adress, const std::string& url);

std::pair<std::string, std::string> GetMethodAndContent(char* buff);