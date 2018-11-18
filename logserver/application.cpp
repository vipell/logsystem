#include "application.h"

#include <iostream>
#include <thread>

const uint16_t LogPortNumber = 4000;
const uint16_t MonPortNumber = 4001;
const uint8_t MaxConnectionCount = 10;

Application::Application() :
    _serverSocket(NULL),
    _cvWriteData(),
    _mutexWriteData()
{
    _serverSocket = new ServerSocket();
}

Application::~Application()
{
    if (_serverSocket != NULL)
    {
        delete _serverSocket;
        _serverSocket = NULL;
    }    
}

/* ---------------- Usage:
   ---------------- Examples: 
   ---------------- Notes:
-------------------------- */

int Application::Execute(int argc, char* argv[])
{
    if (_serverSocket == NULL)
    {
        return 1;    
    }
    
    uint16_t logPort = LogPortNumber;
    uint16_t monPort = MonPortNumber; 
    
    if (argc == 3)
    {
        logPort = std::stoi(argv[1]);
        monPort = std::stoi(argv[2]);
    }
    printf("Logging clients port number: %i\n", logPort);
    printf("Monitoring clients port number: %i\n", monPort);
    
    if (!_serverSocket->Open(logPort, monPort, MaxConnectionCount))
    {
        printf("Failed to open sockets\n");
        return 1;
    }
        
    printf("Activating receiving data from logging clients\n");    
    std::thread receiveLogData(&ReceiveLogData, _serverSocket, this);
    
    printf("Activating writing data to Log Storage\n");
    std::thread writeLogData(&WriteLogData, _serverSocket, this);
    
    printf("Activating sending data to monitoring clients\n");    
    std::thread sendLogData(&SendLogData, _serverSocket);    
    
    do 
    {
        printf("Press 'Enter' to exit...\n");
    } while (std::cin.get() != '\n');
    
    _serverSocket->Close();
    
    receiveLogData.detach();
    writeLogData.detach();
    sendLogData.detach();
    
    printf("Application finished\n");    
    return 0;
}

void Application::ReceiveLogData(ServerSocket* serverSocket, Application* application)
{
    while (1)
    {
        if (serverSocket->Receive())
        {
            // Notify WriteLogData thread about received data
            std::unique_lock<std::mutex> lock(application->_mutexWriteData);
            application->_cvWriteData.notify_one();
        }
    }    
}

void Application::WriteLogData(ServerSocket* serverSocket, Application* application)
{
    while (1)
    {
        std::unique_lock<std::mutex> lock(application->_mutexWriteData);
        application->_cvWriteData.wait(lock);
        
        if (!serverSocket->WriteData())
        {
            printf("Writing data to Log Storage failed\n");
        }
    }
}

void Application::SendLogData(ServerSocket* serverSocket)
{
    while (1)
    {
        if (!serverSocket->Send())
        {
            printf("Sending data to monitoring clients failed\n");
        }        
    }
}

int Application::Help()
{
    return 1;
}

