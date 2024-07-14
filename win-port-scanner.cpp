#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

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
    ioctlsocket(sock, FIONBIO, &mode);

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
            
            if (select(0, NULL, &writeSet, NULL, &timeout) > 0 && FD_ISSET(sock, &writeSet)) {
                std::cout << "Port " << port << " is open." << std::endl;
            } else {
                std::cout << "Port " << port << " is closed." << std::endl;
            }
        } else {
            std::cout << "Connect error: " << error << std::endl;
        }
    } else {
        std::cout << "Port " << port << " is open." << std::endl;
    }

    // Clean up
    closesocket(sock);
    WSACleanup();
}

int main() {
    const char* targetHost = "scanme.nmap.org";  // Target host for testing
    for (int port = 80; port <= 85; ++port) {
        scanPort(targetHost, port);
    }

    std::cout << "press enter to exit...";
    std::cin.get();

    return 0;
}

//g++ -o win-port-scanner win-port-scanner.cpp -lws2_32