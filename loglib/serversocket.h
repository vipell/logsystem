#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <vector>
#include <mutex>

struct LogData
{
    uint32_t clientId;
    std::string message;
    time_t timestamp;
};

class ServerSocket
{

public:
    ServerSocket();
    virtual ~ServerSocket();
    
    bool Open(uint16_t logPort, uint16_t monitorPort, uint8_t maxConnectionCount);
    bool Close();
    bool Send(); // for Monitor clients
    bool Receive(); // for Logging clients
    bool WriteData(); // for Writing received data to Data Storage  
    
private:    
    bool SetLogData(const char* buffer, LogData& logData);
    bool GetData(uint32_t clientId, std::vector <std::string>& logData);
    
private:
    int _logSocket; // socket for handling Logging clients requests
    int _monSocket; // socket for handling Monitor clients requests
    
    sockaddr_in _sockaddrLog;
    sockaddr_in _sockaddrMon;
    
    std::vector <LogData> _logData;
    
    std::mutex _mutexData; // mutex for Log Data handling
    std::mutex _mutexFile; // mutex for Data Storage handling
};