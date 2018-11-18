#include "application.h"

#include <iostream>
#include <string.h>

const char* const Address = "127.0.0.1";
const uint16_t PortNumber = 4001;

Application::Application() :
    _clientSocket(NULL)
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
    
    if (argc == 3)
    {
        address = argv[1];
        port = std::stoi(argv[2]);
    }

    printf("Server address: %s\n", address);    
    printf("Server port number: %i\n", port);
    
    char clientId[16];
    memset(&clientId, 0, sizeof(clientId));
    
    while (1)
    {
        printf("Enter Client ID or 0 for all clients or press 'Enter' to exit: ");
        fgets(clientId, 15, stdin);
        if (clientId[0] == '\n')
        {
            break; // while
        }
        
        int id = std::stoi(clientId);
        std::vector <std::string> logData;
        if (this->Get(address, port, id, logData))
        {
            for (auto data : logData)
            {
                printf("%s\n", data.c_str());
            }
        }
    }
    
    printf("Application finished\n");    
    return 0;
}

bool Application::Get(const char* address, uint32_t port, uint32_t id, std::vector <std::string>& logData)
{
    if (!this->_clientSocket->Open(address, port))
    {
        printf("Failed to open connection to sever\n");
        return false;
    }
    bool result = this->_clientSocket->Get(id, logData);
    if (!result)
    {
        printf("Failed to get log data from sever\n");
    }
    this->_clientSocket->Close();
    
    return result;
}


int Application::Help()
{
    return 1;
}

