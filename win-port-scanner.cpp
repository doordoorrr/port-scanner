#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

bool scanPort(const string& host, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { //WSASStartup initalizes winsock lib
        cerr << "WSAStartup failed" << endl;
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //socket creation  , IPPROTO_TCP provides non-blocking mode
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation failed" << endl;
        WSACleanup();
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

    u_long mode = 1;  // non-blocking mode
    ioctlsocket(sock, FIONBIO, &mode);

    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)); //check if socket is ready for writing , successful connection

    FD_SET writeSet;
    FD_ZERO(&writeSet);
    FD_SET(sock, &writeSet);

    struct timeval timeout;
    timeout.tv_sec = 1;  // 1 second timeout
    timeout.tv_usec = 0;

    int selectResult = select(0, NULL, &writeSet, NULL, &timeout);


//cleaing up :P
    if (selectResult > 0 && FD_ISSET(sock, &writeSet)) {
        closesocket(sock);
        WSACleanup();
        return true;
    } else {
        closesocket(sock);
        WSACleanup();
        return false;
    }
}
void scanPort(const char* targetHost, int port) {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server;
    int result;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Failed to initialize Winsock. Error: " << WSAGetLastError() << endl;
        return;
    }

    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cout << "Could not create socket. Error: " << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }

    // Set up the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, targetHost, &server.sin_addr);

    // Try to connect to the port
    result = connect(sock, (struct sockaddr*)&server, sizeof(server));
    if (result == SOCKET_ERROR) {
        cout << "Port " << port << " is closed." << endl;
    } else {
        cout << "Port " << port << " is open." << endl;
    }

    // Clean up
    closesocket(sock);
    WSACleanup();
}

int main() {
    const char* targetHost = "scanme.nmap.org";  // Target host for testing
   for (int port = 80; port <= 85; ++port) { // Scanning a small range of ports for example
        scanPort(targetHost, port);
    }

    return 0;
}