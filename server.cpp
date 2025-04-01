#include <cstring>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

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

    cout << "test 1" << endl;
    WSADATA wsaData; //creates a structre to put the Windows socket data in
    errorFlag = WSAStartup(MAKEWORD(2,2), &wsaData); //allows Winsock socket programming to work on Windows by initializing WS2_32.dll. MAKEWORD tells Windows to use Winsock version 2.2 and stores the Windows socket data in wsaData
    if (errorFlag != 0) {
        cout << "Couldn't start up windows sockets: ";
        cout << errorFlag << endl;
        WSACleanup(); //terminates any socket processes that may still run even if the program has stopped
        return -1;
    }
    cout << "test 2" <<endl;
    errorFlag = getaddrinfo(NULL, port_number, &server, &result); //creates an addrinfo structure from the first 3 arguments that'll be stored in linked list, "result"
    if (errorFlag != 0) {
        cout << "Failed to get address information: ";
        cout << errorFlag << endl;
        return -1;
    }

    cout << "test 3" << endl;
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); //creating the server socket
    if (serverSocket == INVALID_SOCKET) {
        cout << "Socket function failed! Exiting program..." << endl;
        return -1;
    }

    if (bind(serverSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) { //binds the server socket

        return -1;
    }
    if (listen(serverSocket, 10) == SOCKET_ERROR) { //enable the server socket to listen (wait) for the client socket
        return -1;
    }

    int clientSocket = accept(serverSocket, NULL, NULL); //connects a client socket to the server socket
    if (clientSocket == INVALID_SOCKET) {
        return -1;
    }

    return 0;
}
