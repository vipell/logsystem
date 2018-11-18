#include "clientsocket.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

// Monitor Client requests:
const char* const GetRequest = "G";

ClientSocket::ClientSocket() :
    _socket(0),
    _sockaddrServer()
{
}

ClientSocket::~ClientSocket()
{
}

bool ClientSocket::Open(const char* address, uint16_t port)
{
    this->Close();
    
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
    {
        // TODO - error handling (errno)
        return false;
    }
    
    _sockaddrServer.sin_family = AF_INET;
    inet_pton(AF_INET, address, (void *)&_sockaddrServer.sin_addr);
    _sockaddrServer.sin_port = htons(port);
    
    if (connect(_socket, (sockaddr *)&_sockaddrServer, sizeof(_sockaddrServer)) == -1)
    {
        // TODO - error handling (errno)
        return false;
    }
        
    return true;
}

bool ClientSocket::Close()
{
    if (_socket)
    {
        close(_socket);
        _socket = 0;
    }
    memset(&_sockaddrServer, 0, sizeof(_sockaddrServer));
    
    return true;
}

bool ClientSocket::Send(uint32_t id, const char* data)
{
    // Message format: '<clientId> | <message>'    
    std::string message = std::to_string(id) + " | " + std::string(data);    
    if(send(_socket, message.c_str(), strlen(message.c_str()), 0) == -1)
    {
        // TODO - error handling - (errno)
        return false;
    }        
    return true;
}

bool  ClientSocket::Get(uint32_t id, std::vector<std::string>& data)
{
    // Message format: '<request> | <clientId>'
    std::string message = std::string(GetRequest) + " | " + std::to_string(id);
    if (send(_socket, message.c_str(), strlen(message.c_str()), 0) == -1)
    {
        // TODO - error handling - (errno)
        return false;
    }
    
    char buffer[BUFSIZ];
    while(1)
    {
        memset(&buffer, 0, BUFSIZ);
        size_t bytesRead =  read(_socket, buffer, BUFSIZ);
        if (bytesRead == -1)
        {
            // TODO - error handling - (errno)
            return false;
        }
        else if (!bytesRead)
        {
            break; // while
        }
        this->Add(buffer, data);
    }
    
    if (!data.size())
    {
        // TODO - error handling
        return false;        
    }
        
    return true;
}

void ClientSocket::Add(const char* buffer, std::vector<std::string>& data)
{
    std::string log(buffer);
    size_t pos = 0;
    size_t pos2 = 0;
    while (pos != std::string::npos)
    {
        pos += 10;
        pos2 = log.find("LOGENTRY |", pos);
        size_t len = std::string::npos;
        if (pos2 != std::string::npos)
        {
            len = pos2 - pos;       
        }
        std::string entry = log.substr(pos, len);
        data.push_back(entry);
        pos = pos2;
    }
}
