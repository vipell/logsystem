#include "serversocket.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <chrono>
#include <fstream>

const char* const LogDataStorage = "data_storage.log";

// Monitor Client requests:
const char* const GetRequest = "G";
const char* const SubscribeRequest = "S";
const char* const UnsubscribeRequest = "U";

inline std::string GetLocalTime(time_t& time)
{
    char buffer[1024] = { 0 };
    struct::tm* tm = std::localtime(&time);
    ::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", tm);
    return std::string(buffer);
}

ServerSocket::ServerSocket() :
    _logSocket(0),
    _monSocket(0),
    _sockaddrLog(),
    _sockaddrMon(),
    _logData(),
    _mutexData(),
    _mutexFile()
{
}

ServerSocket::~ServerSocket()
{
    this->Close();
}

bool ServerSocket::Open(uint16_t logPort, uint16_t monitorPort, uint8_t maxConnectionCount)
{
    this->Close();
    
    _logSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_logSocket == -1)
    {
        // TODO - error handling (errno)
        return false;
    }
    _monSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_monSocket == -1)
    {
        // TODO - error handling (errno)
        return false;
    }
    
    _sockaddrLog.sin_family = AF_INET;
    _sockaddrLog.sin_addr.s_addr = INADDR_ANY;
    _sockaddrLog.sin_port = htons(logPort);    
    if (bind(_logSocket, (sockaddr*)&_sockaddrLog, sizeof(_sockaddrLog)) == -1)
    {
        // TODO - error handling (errno)
        return false;
    }
    
    _sockaddrMon.sin_family = AF_INET;
    _sockaddrMon.sin_addr.s_addr = INADDR_ANY;
    _sockaddrMon.sin_port = htons(monitorPort);    
    if (bind(_monSocket, (sockaddr*)&_sockaddrMon, sizeof(_sockaddrMon)) == -1)
    {
        // TODO - error handling (errno)
        return false;
    }
    
    if (listen(_logSocket, maxConnectionCount) == -1)
    {
        // TODO - error handling (errno)
        return false;
    }

    if (listen(_monSocket, maxConnectionCount) == -1)
    {
        // TODO - error handling (errno)
        return false;
    }
    
    return true;
}

bool ServerSocket::Close()
{
    if (_logSocket)
    {
        close(_logSocket);
        _logSocket = 0;
    }
    if (_monSocket)
    {
        close(_monSocket);
        _monSocket = 0;
    }
        
    memset(&_sockaddrLog, 0, sizeof(_sockaddrLog));
    memset(&_sockaddrMon, 0, sizeof(_sockaddrMon));
    
    return true;    
}

bool ServerSocket::Send()
{
    sockaddr_in sockaddrClient;
    socklen_t socklenClient = 0;
    memset(&sockaddrClient, 0, sizeof(sockaddrClient));
    
    int socket = accept(_monSocket, (sockaddr*)&sockaddrClient, &socklenClient);
    if (socket == -1)
    {
        // TODO - error handling (errno)
        return false;
    }

    char buffer[256];
    memset(&buffer, 0, 256);
    
    size_t bytesRead = read(socket, buffer, 255);
    if (bytesRead == -1)
    {
        // TODO - error handling (errno)
        return false;        
    }
    
    std::string request(buffer);
    size_t separator = request.find_first_of('|'); // request must be in format '<request> | <clientId>'
    if (separator == std::string::npos || separator < 2 || (request.length() - separator) < 3)
    {
        // TODO - error handling - wrong message format, must be: '<request> | <clientId>'
        return false;
    }
    
    uint32_t id = std::stoi(request.substr(separator + 2, std::string::npos));
    request = request.substr(0, separator - 1);
    
    if (0 == request.compare(GetRequest))
    {
        std::vector <std::string> logData;
        if (!this->GetData(id, logData))
        {
            return false;    
        }
        
        for (auto data : logData) 
        {
            data = "LOGENTRY | " + data;
            if (send(socket, data.c_str(), strlen(data.c_str()), 0) == -1)
            {
                // TODO - error handling - report error and continue for loop
            }
        }
    }
    else if (0 == request.compare(SubscribeRequest))
    {
        // TODO - request to subscribe on Real Time monitoring is not imlemented yet
        return false;
    }
    else if (0 == request.compare(UnsubscribeRequest))
    {
        // TODO - request to unsubscribe on Real Time monitoring is not imlemented yet
        return false;
    }
    else
    {
        // TODO - error handling - wrong request received
        return false;
    }
    
    close(socket);

    return true;
}

bool ServerSocket::Receive()
{
    sockaddr_in sockaddrClient;
    socklen_t socklenClient = 0;
    memset(&sockaddrClient, 0, sizeof(sockaddrClient));
    
    int socket = accept(_logSocket, (sockaddr*)&sockaddrClient, &socklenClient);
    if (socket == -1)
    {
        // TODO - error handling (errno)
        return false;
    }
    
    char buffer[BUFSIZ];
    memset(&buffer, 0, BUFSIZ);    
    size_t bytesRead = read(socket, buffer, BUFSIZ);
    if (bytesRead == -1)
    {
        // TODO - error handling (errno)
        return false;        
    }
    
    LogData logData;
    if (!this->SetLogData(buffer, logData))
    {
        return false;
    }
    
    _mutexData.lock();
    _logData.push_back(logData);
    _mutexData.unlock();
    
    close(socket);
    
    return true;
}

bool ServerSocket::WriteData()
{
    _mutexFile.lock();
    FILE* file = fopen(LogDataStorage, "a");
    if (NULL == file)
    {
        _mutexFile.unlock();
        return false;
    }
    
    _mutexData.lock();
    for (auto data : _logData)
    {
        std::string entry = GetLocalTime(data.timestamp) + " | " + std::to_string(data.clientId) +  " | " + data.message + '\n';
        fputs(entry.c_str(), file);
    }
    _logData.clear();
    _mutexData.unlock();
    
    fclose(file);
    _mutexFile.unlock();
    
    return true;
}

bool ServerSocket::SetLogData(const char* buffer, LogData& logData)
{
    std::string data(buffer);    
    size_t separator = data.find_first_of('|');
    if (separator == std::string::npos || separator < 2 || (data.length() - separator) < 3)
    {
        // TODO - error handling - wrong message format, must be: '<clientId> | <message>'
        return false;
    }
    
    logData.clientId = std::stoi(data.substr(0, separator));
    logData.message = data.substr(separator + 2, std::string::npos);
    logData.timestamp = std::time(0);  // set current time
    
    return true;
}

bool ServerSocket::GetData(uint32_t clientId, std::vector <std::string>& logData)
{
    _mutexFile.lock();
    std::ifstream file(LogDataStorage, std::ifstream::in);
    if (!file.good())
    {
        // TODO - error handling - opening Logs Data Storage
        _mutexFile.unlock();
        return true;
    }
    
    std::string line;
    while (std::getline(file, line))
    {
        std::string pattern = " | " + std::to_string(clientId) + " | ";  // search pattern for format: ' | <clientId> | '
        if(!clientId || (clientId && line.find(pattern) != std::string::npos)) // clientId == 0 -> all clients log data
        {
            logData.push_back(line);
        }
    }
    
    file.close();
    _mutexFile.unlock();
    
    if (!logData.size())
    {
        // TODO - error handling - no log data for target id
    }
    
    return true;
}
