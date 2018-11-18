#pragma once

#include "clientsocket.h"

class Application
{
public:
    Application();
    ~Application();
    
    int Execute(int argc, char* argv[]);
    
private:
    int Help();
    bool Get(const char* address, uint32_t port, uint32_t id, std::vector <std::string>& logData);
    
    ClientSocket* _clientSocket;   
};