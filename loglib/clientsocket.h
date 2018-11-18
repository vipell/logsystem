#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <vector>

class ClientSocket
{

public:
    ClientSocket();
    virtual ~ClientSocket();
    
    bool Open(const char* address, uint16_t port);
    bool Close();
    bool Send(uint32_t id, const char* data); // for Logging clients
    bool Get(uint32_t id, std::vector<std::string>& data);  // for Monitoring clients  
        
private :
    void Add(const char* buffer, std::vector<std::string>& data);
    
private:
    int _socket;
    sockaddr_in _sockaddrServer;
};