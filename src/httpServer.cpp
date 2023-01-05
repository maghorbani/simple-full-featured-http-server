#include "httpServer.h"

#include <thread>
#include <chrono>
namespace http{
    
void Server::listen()
{
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
}