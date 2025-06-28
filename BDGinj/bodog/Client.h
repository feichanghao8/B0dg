#pragma once

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define WIN32_LEAN_AND_MEAN
#include "nlohmann/json.hpp"

// #include <winsock2.h>
// #include <ws2tcpip.h>
// #include <iostream>
// #include <chrono>


#define BUFFER_SIZE  16*1024

class Client {
public:
    char* serverIp = (char*)"127.0.0.1";
    int serverPort = 12345;

    Client(){}
    Client(char* _serverIp, int _port): serverIp(_serverIp), serverPort(_port){}

    // SOCKET currentSocketConnection = -1;

    std::map<void*, SOCKET> connections;

    bool connectionPresent(void* key) {
        if(connections.find(key) != connections.end()) {
            return true;
        }
        return false;
    }

    SOCKET getConnection(void* key) {
        if(connectionPresent(key)) {
            return connections[key];
        } else {
            return -1;
        }
    }

    void logTime(void* streamPtr, long long startTime, const char* message) {
        int timeDiff = (int)getTimeDifferenceMillis(startTime);
        // logfff(streamPtr, "Added connection: %x %x. Time to add: %d ms\n", streamPtr, connectionSocket, timeDiff);
        logfff(streamPtr, "Client: %s, time: %d\n", message, timeDiff);
    }


    int startConnection(void* streamPtr) {
        long long start = getCurrentTimeMillis();

        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            logfff(streamPtr, "Error: Winsock initialization failed.\n");
            logTime(streamPtr, start, "Error: Winsock initialization failed");
            return EXIT_FAILURE;
        }

        // Create a socket
        // this->currentSocketConnection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        SOCKET connectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connectionSocket == INVALID_SOCKET) {
            WSACleanup();
            logTime(streamPtr, start, "Error: Unable to create socket.");
            return EXIT_FAILURE;
        }

        // TODO: We assume that server might take 8 seconds to reply. Rediscuss this with the
        // team and make sure this is correct approach. Currently we had to get rid of
        // threads. It might be a problem.
        int timeoutSecs = 30*1000; // 30 seconds
        if (setsockopt(connectionSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutSecs, sizeof(timeoutSecs)) == SOCKET_ERROR) {
            logfff(streamPtr, "setsockopt failed: %d\n", WSAGetLastError());
            closesocket(connectionSocket);
            WSACleanup();
            logTime(streamPtr, start, "Setsockopt failed");
            return EXIT_FAILURE;
        }

        // Define the server address
        sockaddr_in server_address;
        std::memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(serverPort);
        server_address.sin_addr.s_addr = inet_addr(serverIp);

        //--------------------------------------------------------------------------------       
        //---------- CONNECT -------------------------------------------------------------       
        // Set the socket to non-blocking mode
        u_long nonBlocking = 1;
        ioctlsocket(connectionSocket, FIONBIO, &nonBlocking);

        int maxTimeToWait = 15;
        int singleTimeoutTime = 1; // Seconds
        int retryAmount = (int)((float)maxTimeToWait / (float)singleTimeoutTime);
        bool isConnectSuccesfull = false;
        for(int i = 0; i < retryAmount; i++) {
            logfff(streamPtr, "Trying to connect [%d] max:[%d]\n", i, retryAmount);
            int result = connect(connectionSocket, (sockaddr*)&server_address, sizeof(server_address));
            if (result == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
                logfff(streamPtr, "Connect failed: %d\n", WSAGetLastError());
            }

            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(connectionSocket, &readfds);

            struct timeval timeout;
            timeout.tv_sec = singleTimeoutTime;  // seconds
            timeout.tv_usec = 0;

            result = select(0, NULL, &readfds, NULL, &timeout);
            if (result == 0) {
                logfff(streamPtr, "Connect timed out after %d seconds. [%d]\n", timeout.tv_sec, WSAGetLastError());
                closesocket(connectionSocket);
            } else if (result < 0) {
                logfff(streamPtr, "Select failed: %d\n", WSAGetLastError());
                closesocket(connectionSocket);
            } else {
                // NOTE: We sucessfully connected
                isConnectSuccesfull = true;
                logfff(streamPtr, "Connected: %d\n", connectionSocket);
                break;
            }
        }

        if(!isConnectSuccesfull) {
            closesocket(connectionSocket);
            WSACleanup();
            return EXIT_FAILURE;
        }


        // Set the socket back to blocking mode
        nonBlocking = 0;
        ioctlsocket(connectionSocket, FIONBIO, &nonBlocking);
        
        //--------------------------------------------------------------------------------       
        //--------------------------------------------------------------------------------       

        // // Connect to the server
        // if (connect(connectionSocket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        //     logfff(streamPtr, "Error: Connection to server failed. Error: [%d]\n", WSAGetLastError());
        //     closesocket(connectionSocket);
        //     WSACleanup();
        //     logTime(streamPtr, start, "Connect failed");
        //     return EXIT_FAILURE;
        // }

        if(connectionPresent(streamPtr)) {
            logfff( streamPtr, "ERROR: We are trying to add connection but we already have it in map. Probably not removed at the end of game or error occured");
            auto it = connections.find(streamPtr);
            if (it != connections.end()) {
                connections.erase(it);
            }
        } 
        connections.insert({streamPtr, connectionSocket});

        logTime(streamPtr, start, "Added connection");
        return 0;
    }

    nlohmann::json sendMessage(void* streamPtr, std::string message, nlohmann::json* result) {

        SOCKET currentConnection = getConnection(streamPtr);

        if(currentConnection == -1) {
            try {
                startConnection(streamPtr);
                currentConnection = getConnection(streamPtr);
            } catch(const std::exception& e) {
                logfff(streamPtr, "Error connecting to the server: ", e.what());
            }
            catch(...) {
                logfff(streamPtr, "Error connecton. General");
            }
        }


        if(currentConnection == -1) {
            logfff(streamPtr, "Error creating connection. Abort.\n");
            return *result;
        }


        //const char* message = "Hello, Server!";
        if (send(currentConnection, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
            logfff(streamPtr, "Error sending message: socket: %x, error: %d\n", currentConnection, WSAGetLastError());
            closeConnection(streamPtr);
            return *result;
            // return EXIT_FAILURE;
        }

        static constexpr auto kBufferSize {1024};

        // Receive a response from the server
        char buffer[kBufferSize] = {};
        std::memset(buffer, 0, kBufferSize);
        int bytes_received = recv(currentConnection, buffer, kBufferSize - 1, 0);
        if (bytes_received == SOCKET_ERROR) {
            logfff(streamPtr, "Error: Failed to receive response. Error was %d\n", WSAGetLastError());
        }
        else {
            logfff(streamPtr, "Got response from server [%d] [%d] %s\n", connections.size(), bytes_received, buffer);
            try {
                *result = nlohmann::json::parse(std::string(buffer));
            }
            catch (const std::exception& e) {
                // TODO: Maybe set it our custom error result as json
                logfff(streamPtr, "Error[sendMessage] %s\n", e.what());
            }
        }

        // Close the socket
        // closesocket(this->currentSocketConnection);
        // WSACleanup();

        return *result;
    }

    void closeConnection(void* streamPtr) {
        SOCKET currentConnection = getConnection(streamPtr);
        if(currentConnection > 0 && currentConnection != INVALID_SOCKET) {
            logfff(streamPtr, "Close connection: %x %x\n", streamPtr, currentConnection);
            closesocket(currentConnection);
            WSACleanup();

            auto it = connections.find(streamPtr);
            if (it != connections.end()) {
                connections.erase(it);
            }
        }
    }

#undef BUFFER_SIZE
};
