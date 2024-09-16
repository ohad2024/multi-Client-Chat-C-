#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include "C:/Users/ohado/Downloads/ohadcode/vector-test1/Vector.hpp"

#pragma comment(lib, "Ws2_32.lib")

const int SERVER_PORT = 54000;
const int BUFFER_SIZE = 200;
const int MAX_CONNECTIONS = SOMAXCONN;

class ChatServer {
    SOCKET _serverSocket;
    Vector<SOCKET> _clientSockets;
    sockaddr_in _serverAddr;
    bool _isRunning = true;

    bool initializeWinsock() {
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        return (iResult == 0);
    }

    bool createSocket() {
        _serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        return (_serverSocket != INVALID_SOCKET);
    }

    bool bindSocket() {
        _serverAddr.sin_family = AF_INET;
        _serverAddr.sin_port = htons(SERVER_PORT);
        _serverAddr.sin_addr.s_addr = INADDR_ANY;
        return (bind(_serverSocket, (sockaddr*)&_serverAddr, sizeof(_serverAddr)) != SOCKET_ERROR);
    }

    bool listenForConnections() {
        return (listen(_serverSocket, MAX_CONNECTIONS) != SOCKET_ERROR);
    }

    void broadcastMessage(const char* message, SOCKET senderSocket) {
        for (int i = 0; i < _clientSockets.getSize(); ++i) {
            SOCKET clientSocket = _clientSockets[i];
            if (clientSocket != senderSocket) {
                send(clientSocket, message, strlen(message), 0);
            }
        }
    }

    void removeClient(SOCKET clientSocket) {
        bool clientFound = false;
        for (int i = 0; i < _clientSockets.getSize() && !clientFound; ++i) {
            if (_clientSockets[i] == clientSocket) {
                _clientSockets.erase(i);
                clientFound = true;
            }
        }
    }

    void handleClient(SOCKET clientSocket) {
        char buffer[BUFFER_SIZE];
        bool isClientConnected = true;
        while (isClientConnected) { 
            int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesReceived > 0) {
                std::cout.write(buffer, bytesReceived);
                std::cout << std::endl;
                broadcastMessage(buffer, clientSocket);
            }
            else if (bytesReceived == 0 || bytesReceived == SOCKET_ERROR) {
                closesocket(clientSocket); 
                removeClient(clientSocket);
                isClientConnected = false;
            }
        }
    }

public:
    void runServer() {
        if (!initializeWinsock() || !createSocket() || !bindSocket() || !listenForConnections()) return;

        while (_isRunning) {
            SOCKET clientSocket = accept(_serverSocket, nullptr, nullptr);
            if (clientSocket != INVALID_SOCKET) {
                _clientSockets.pushBack(clientSocket);
                std::thread clientThread(&ChatServer::handleClient, this, clientSocket);
                clientThread.detach();
            }
        }

        closesocket(_serverSocket); 
        WSACleanup();
    }
};

int main() {
    ChatServer server;
    server.runServer();
    return 0;
}
