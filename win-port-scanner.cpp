#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")

void scanPort(const char* targetHost, int port) {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server;
    int result;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Failed to initialize Winsock. Error: " << WSAGetLastError() << std::endl;
        return;
    }

    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cout << "Could not create socket. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // Set up the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, targetHost, &server.sin_addr);

    // Set socket to non-blocking mode
    u_long mode = 1;
    

    if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
        std::cout << "ioctlsocket failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    // Try to connect to the port
    result = connect(sock, (struct sockaddr*)&server, sizeof(server));
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            // Handle non-blocking connect
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(sock, &writeSet);
            
            struct timeval timeout;
            timeout.tv_sec = 1;  // 1 second timeout
            timeout.tv_usec = 0;
            
            if (select(0, NULL, &writeSet, NULL, &timeout) > 0) {
                int optVal;
                int optLen = sizeof(int);
                if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&optVal, &optLen) != SOCKET_ERROR && optVal == 0) {
                    std::cout << "Port " << port << " is open." << std::endl;
                } else {
                    std::cout << "Port " << port << " is closed." << std::endl;
                }
            } else {
                std::cout << "Port " << port << " is closed." << std::endl;
            }
        } else {
            std::cout << "Connect error: " << error << std::endl;
            closesocket(sock);
            WSACleanup();
            return;
        }
    } else {
        std::cout << "Port " << port << " is open." << std::endl;
    }

    // Clean up
    closesocket(sock);
    WSACleanup();
}

int main() {
    char targetHost[100];
    int port;
    bool continueScanning = true;
    char continueInput;

    do {
        std::cout << "Enter the target host: ";
        std::cin >> targetHost;

        std::cout << "Enter the port you would like to scan (Enter -1 to exit): ";
        std::cin >> port;

        if (port == -1) {
            continueScanning = false;
        } else {
            scanPort(targetHost, port);
        }

        std::cout << "Would you like to scan another port? (Y/N): ";
        std::cin >> continueInput;
        
        if (continueInput == 'N' || continueInput == 'n') {
            continueScanning = false;
        }
    } while (continueScanning);

    std::cout << "Scanning complete. Press enter to exit...";
    std::cin.ignore();
    std::cin.get();

    return 0;
}

//g++ -o win-port-scanner win-port-scanner.cpp -lws2_32