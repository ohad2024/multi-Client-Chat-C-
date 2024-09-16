#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

const int BUFFER_SIZE = 200;
const int SERVER_PORT = 54000;
const std::string SERVER_IP = "127.0.0.1";

class ChatClient {
    SOCKET _clientSocket;
    sockaddr_in _serverAddr;
    bool _isRunning = true;
    std::string _userName;

    bool initializeWinsock() {
        WSADATA wsaData;
        return (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
    }

    bool createSocket() {
        _clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        return (_clientSocket != INVALID_SOCKET);
    }

    bool connectToServer() {
        _serverAddr.sin_family = AF_INET;
        _serverAddr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP.c_str(), &_serverAddr.sin_addr);
        return (connect(_clientSocket, (sockaddr*)&_serverAddr, sizeof(_serverAddr)) != SOCKET_ERROR);
    }

    void getUserName() {
        while (_userName.empty()) {
            std::cout << "Enter your name: ";
            std::getline(std::cin, _userName);
        }
    }

    void sendMessage() {
        std::string message;
        std::string joinMessage = _userName + " joined the chat.";
        send(_clientSocket, joinMessage.c_str(), joinMessage.size() + 1, 0);

        while (_isRunning) {
            std::getline(std::cin, message);
            if (!message.empty()) {
                if (message == "exit") {
                    _isRunning = false;
                }
                std::string fullMessage = _userName + ": " + message;
                send(_clientSocket, fullMessage.c_str(), fullMessage.size() + 1, 0);
            }
        }
    }

    void receiveMessage() {
        char receiveBuffer[BUFFER_SIZE];
        while (_isRunning) {
            int bytesReceived = recv(_clientSocket, receiveBuffer, BUFFER_SIZE, 0);
            if (bytesReceived > 0) {
                std::cout.write(receiveBuffer, bytesReceived);
                std::cout << std::endl;
            }
            if (bytesReceived == 0 || bytesReceived == SOCKET_ERROR) {
                _isRunning = false;
            }
        }
    }

public:
    void runClient() {
        getUserName();

        if (!initializeWinsock() || !createSocket() || !connectToServer()) return;

        std::thread sendThread(&ChatClient::sendMessage, this);
        std::thread receiveThread(&ChatClient::receiveMessage, this);

        sendThread.join();
        receiveThread.join();

        closesocket(_clientSocket);
        WSACleanup();
    }
};

int main() {
    ChatClient client;
    client.runClient();
    return 0;
}
