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

vector<int> portScanner(const string& host, const vector<int>& portRange) {  //port scanner wooo scans a range of ports concurrently using async and future 
    vector<int> openPorts;
    vector<future<bool>> futures;

    for (int port : portRange) {
        futures.push_back(async(launch::async, scanPort, host, port));
    }

    for (size_t i = 0; i < futures.size(); ++i) {
        if (futures[i].get()) {
            openPorts.push_back(portRange[i]);
        }
    }

    return openPorts;
}

int main() {
    string targetHost = "scanme.nmap.org";  // Target host for testing
    vector<int> targetPorts = {22, 80, 443, 25, 110, 143, 587, 3306, 3389, 8080};  // Example ports to scan

    vector<int> openPorts = portScanner(targetHost, targetPorts);

    cout << "Open ports on " << targetHost << ":" << endl;
    for (int port : openPorts) {
        cout << port << endl;
    }

    return 0;
}