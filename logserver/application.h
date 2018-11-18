#pragma once

#include "serversocket.h"
#include <condition_variable>

class Application
{
public:
    Application();
    ~Application();
    
    int Execute(int argc, char* argv[]);
    
private:
    int Help();
    static void ReceiveLogData(ServerSocket* serverSocket, Application* application);
    static void WriteLogData(ServerSocket* serverSocket, Application* application);
    static void SendLogData(ServerSocket* serverSocket);
    
    ServerSocket* _serverSocket;
    std::condition_variable _cvWriteData;
    std::mutex _mutexWriteData;
};