
# Data Logging System (logsystem)

## Introduction
Data Logging System (DLS) is designed to collect data sent from logging clients on logging server.
Monitoring clients read data logged by server and show it. Monitoring clients are able to show all data
collected by server or client specific data. DLS can work in Local Area Network or locally on single computer.

## Log Server (logserver)

### Usage
./logserver <log_port_number>  <mon_port_number>,
where: < log_port_number> is port number to serve logging clients, <mon_port_number> is port number to serve monitoring clients.
Example: ./logserver 5000 5001
Note: default parameters values (4000 and 4001 correspondingly) are applied in the case if logserver is started without parameters.

### Functionality
Server stores received log data locally into Log Data Storage (file). LDS is created (if not exist) in the same directory where logserver executable is
located and has ‘data_storage.log’ name. Each log data entry has following format in LDS:
<_timestamp_> | <client_id> | <log_data>,
where <_timestamp_> is date and time added by server when it receives log data, <client_id> is unique client ID, <log_data> is data received from client.

### Implementation
Server uses 2 sockets and 3 threads. 1st socket and 1st thread are used to receive log data from logging clients, add time stamp value and store it in memory. 2nd thread get
signal from 1st thread when data is available and writes received log data from memory to LDS file. 2nd socket and 3rd thread are used to receive requests from monitoring clients,
retrieve target log data from LDS file and send it back to monitoring clients.

## Logging Client (logclient)

### Usage
./logclient <server_address>  <server_port_number> <client_id>,
where: <server_address> is log server address,  <server_port_number> is server port number for logging clients, <client_id> is unique client ID.
Example: ./logclient 192.168.0.100 5000 234567
Note: default parameters values (127.0.0.1, 4000 and generated unique Client ID correspondingly) are applied in the case if logclient is started without parameters.

### Functionality
Logging Client is sending automatically every 5 seconds data to Log Server. Also it is possible to send manually data to server by
pressing ‘Enter’ button. Each data entry sent by client to server contains client ID and unique log data.

## Monitoring Client (monitorclient)

### Usage
./monitorclient <server_address>  <server_port_number>,
where: <server_address> is log server address,  <server_port_number> is server port number for monitoring clients.
Example: ./monitorclient 192.168.0.100 5001
Note: default parameters values (127.0.0.1 and 4001 correspondingly) are applied in the case if monitorclient is started without parameters.

### Functionality
Monitoring client is able to read data from Log Server for all clients or for some target client defined by client Id.
To read all log data client Id value ‘0’ is used.

## Log Library (loglib)

### Description
Log Library is a static library that provide ServerSocket and ClientSocket classes services. ServerSocket is used by logserver, ClientSocket is used by logclient and monitorclient.
