#pragma once

#include "clientsocket.h"
#include <mutex>
#include <condition_variable>

class Application
{
public:
    Application();
    ~Application();
    
    int Execute(int argc, char* argv[]);
    
private:
    int Help();
    bool Send(const char* address, uint32_t port, uint32_t id, const char* logData);
    static void SendData(const char* address, uint32_t port, uint32_t id, Application* application);
    
    ClientSocket* _clientSocket;
    std::mutex _sendMutex;
    std::mutex _exitMutex;
    bool _exit;
};