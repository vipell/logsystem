#include "application.h"

#include <iostream>
#include <random>
#include <thread>

const char* const Address = "127.0.0.1";
const uint16_t PortNumber = 4000;

inline uint32_t GetRandomId()
{
    std::random_device rd;
    return rd();
}

Application::Application() :
    _clientSocket(NULL),
    _sendMutex(),
    _exitMutex(),
    _exit(false)
{
    _clientSocket = new ClientSocket();
}

Application::~Application()
{
    if (_clientSocket != NULL)
    {
        delete _clientSocket;
        _clientSocket = NULL;
    }    
}

/* ---------------- Usage:
   ---------------- Examples: 
   ---------------- Notes:
-------------------------- */

int Application::Execute(int argc, char* argv[])
{
    const char* address = Address;
    uint16_t port = PortNumber;
    uint16_t id = GetRandomId();
    
    if (argc == 4)
    {
        address = argv[1];
        port = std::stoi(argv[2]);
        id = std::stoi(argv[3]);        
    }
    
    printf("Server address: %s\n", address);    
    printf("Server port number: %i\n", port);    
    printf("Client ID: %i\n", id);

    printf("Activating automatic log writing\n");    
    std::thread sendData(&SendData, address, port, id, this);

    int count = 0;
    while (1)
    {
        printf("Press 'Enter' to write log or enter 'q' to exit...\n");
        char value = getchar();
        if (value != 'q')
        {
            std::string data = "Manual log data " + std::to_string(count++);
            _sendMutex.lock();
            if (this->Send(address, port, id, data.c_str()))
            {
                printf("Log written\n");    
            }
            _sendMutex.unlock();
        }
        else
        {
            break; // while
        }
    }
    
    _exitMutex.lock();
    _exit = true;
    _exitMutex.unlock();
    
    sendData.join();
    
    printf("Application finished\n");    
    return 0;
}

bool Application::Send(const char* address, uint32_t port, uint32_t id, const char* logData)
{
    if (!this->_clientSocket->Open(address, port))
    {
        printf("Failed to open connection to sever\n");
        return false;
    }
    bool result = this->_clientSocket->Send(id, logData);
    if (!result)
    {
        printf("Failed to send log data to sever\n");
    }
    this->_clientSocket->Close();
    
    return result;
}

void Application::SendData(const char* address, uint32_t port, uint32_t id, Application* application)
{
    int count = 0;
    while(1)
    {
        std::string data = "Automatic log data " + std::to_string(count++);
        application->_sendMutex.lock();
        application->Send(address, port, id, data.c_str());
        application->_sendMutex.unlock();
        
        application->_exitMutex.lock();
        if (application->_exit)
        {
            break;            
        }
        application->_exitMutex.unlock();
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    application->_exitMutex.unlock();
}

int Application::Help()
{
    return 1;
}

