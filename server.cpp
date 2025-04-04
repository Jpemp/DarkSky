#include <string>
#include <cstring>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib") //needs to be added or else socket programming won't work on Windows

using namespace std;

static bool disconnectFlag = false;

int main(void) {

    int errorFlag; //will be used to handle and detect errors with winsock2.h functions

    //information to be used for socket IP address
    struct addrinfo server; //server creates a struct that contains server host information
    struct addrinfo *result = NULL; //pointer which will store host info through a linked list
    
    memset(&server, 0, sizeof(server)); //initializes the server struct with zeroes in every struct member so that unused members don't confuse and mess up the program further down the line. This was making the getaddrinfo function mess up

    server.ai_family = AF_INET; //specify to use the IPv4 address family
    server.ai_socktype = SOCK_STREAM; //IP address will be used for stream socket
    server.ai_protocol = IPPROTO_TCP; //TCP protocol will be used for communication for this address
    server.ai_flags = AI_PASSIVE; //indicates that this address will be used for socket binding
    char port_number[] = "8080"; //contains the port number for the socket. Figure out how to change this rather than using locking it in code so it doesn't need to recompile if other applications are using this port

    //cout << "test 1" << endl;
    WSADATA wsaData; //creates a structre to put the Windows socket data in
    errorFlag = WSAStartup(MAKEWORD(2,2), &wsaData); //allows Winsock socket programming to work on Windows by initializing WS2_32.dll. MAKEWORD tells Windows to use Winsock version 2.2 and stores the Windows socket data in wsaData
    if (errorFlag != 0) {
        cout << "Couldn't start up windows sockets: error ";
        cout << errorFlag << endl;
        WSACleanup(); //terminates any socket processes that may still run even if the program has stopped
        return -1;
    }
    //cout << "test 2" <<endl;
    errorFlag = getaddrinfo(NULL, port_number, &server, &result); //creates an addrinfo structure from the first 3 arguments that'll be stored in linked list, "result"
    if (errorFlag != 0) {
        cout << "Failed to get address information: error ";
        cout << errorFlag << endl;
        WSACleanup();
        return -1;
    }

    //cout << "test 3" << endl;
    int serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol); //creating the server socket
    if (serverSocket == INVALID_SOCKET) {
        cout << "Socket function failed: error " << endl;
        cout << WSAGetLastError() << endl;
        WSACleanup();
        return -1;
    }

    //cout << "test 4" << endl;
    errorFlag = bind(serverSocket, result->ai_addr, result->ai_addrlen); //binds the server socket
    if(errorFlag!=0) {
        cout << "Couldn't bind a socket: error ";
        cout << errorFlag << endl;
        WSACleanup();
        return -1;
     }

    //cout << "test 5" << endl;
    errorFlag = listen(serverSocket, 10); //enable the server socket to listen (wait) for the client socket
    if (errorFlag!=0){
        cout << "Socket listening failed: error ";
        cout << errorFlag << endl;
        WSACleanup();
        return -1;
    }

    //cout << "test 6" << endl;
    cout << "Listening for client socket..." << endl;
    int clientSocket = accept(serverSocket, NULL, NULL); //connects a client socket to the server socket
    if (clientSocket == INVALID_SOCKET) {
        cout << "Couldn't accept a socket: error ";
        cout << WSAGetLastError() << endl;
        return -1;
    }
    else {
        cout << "Client has connected!" << endl; //Server and client are succesfully connected. Time to test sending and recieving messages.
        const char servMessage[256] = "Server Message";
        char clieMessage[256];
        while (!disconnectFlag) {
            send(clientSocket,servMessage, 256, 0);
            recv(clientSocket, clieMessage, 256, 0);
        
            cout << clieMessage << endl;
            disconnectFlag = true;
        }

    }



    return 0;
}
