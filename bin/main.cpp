#include <iostream>
#include <string> 
#include <cstring>
#include <cstdlib>
#include "lib/sock_manager_lib/sock_manager.h"

int main(int argc, char** argv) {

    if (argc != 2 && argc != 5 && argc != 3) {
        std::cout << "Incorrect amount of arguments: " << argc << std::endl;
        return 1;
    }

    int port = 16382;
    std::string url{""};

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--port") == 0) {
            port = std::strtoll(argv[++i], nullptr, 10);

        } else if (std::strcmp(argv[i], "--origin") == 0) {
            url = std::string(argv[++i]);
        
        } else if (std::strcmp(argv[i], "--clear-cache") == 0) {
            /*clear cache*/ //TODO
        
        } else {
            return 1;
        }
    }

    Proxying(port, url);

    return 0;
}