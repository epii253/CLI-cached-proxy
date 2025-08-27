#pragma once

#include <string>
#include <unistd.h>
#include <string_view>
#include <sys/socket.h>
#include <netinet/in.h>


struct SocketWrapper {
     int socket_fd = -1;

     SocketWrapper(int fd) :
     socket_fd(fd)
     {}

     void Realese() {
          socket_fd = -1;
     }

     ~SocketWrapper() {
          if (socket_fd > 0)
               close(socket_fd);
     }
};

int SendData(int client_fd, size_t size, const char* data);
void Proxying(int port, std::string url);
int InitilazeServerSocket(int port);

void CliendWork(int client_fd, const std::string& adress, const std::string& url);

std::pair<std::string, std::string> GetMethodAndContent(const char* buff);
