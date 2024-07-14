#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <string>



#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>



using namespace std;

bool scanPort(const string& host, int port)
{
    int sock = socket(AF_INET , SOCK_STREAM, 0);
    if (sock <0){
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    struct hostent* server = gethostbyname(host.c_str());
    if (server == nullptr)
    {
        close(sock);
        return false;
    }
    
    bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    int result = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    close(sock);
    return result >= 0;
}

vector<int> portScanner(const string& host, const vector<int>& portRange)
{
    vector<int> openPorts;
    vector<future<bool>> futures;

    for (int port : portRange)
    {
        futures.push_back(async(launch::async, scanPort, host, port));
    }

    for (size_t i = 0; i <futures.size(); ++i)
    {
        if (futures[i].get()){
            opePorts.push_back(portRange[i]);
        }
    }

    return openPorts;
}

int main()
{
    string targetHost = "scanme.nmap.org"; //target host for testing
    vector<int> targetPorts = {22, 80, 443, 25, 110, 143, 21};

    cout << "Scanning" << targetHost << "for open ports..." <<endl;

    if (!openPorts.empty()) 
    {

        cout << "Open Ports: " << endl;

        for (int port: openPorts) 
        {
            cout << port << endl;
        }
    } else 
    {
        cout << "No open ports found!! :( " << endl;
    }

    return 0;
}

