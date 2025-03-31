#include <cstring>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

int main(void){

    //sets up sockaddr structure. This struct stores the socket address of the server socket.
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("192.168.1.1");
    server.sin_port = 80;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); //creating the server socket
    if(serverSocket == INVALID_SOCKET){
        cout << "Socket function failed! Exiting program..." << endl;
        return -1;
    }

    if(!bind(serverSocket, (struct sockaddr*) &server, sizeof(server)) == SOCKET_ERROR){ //binds the server socket
        
        return -1;
    }
    if(!listen(serverSocket, 10)){ //enable the server socket to listen (wait) for the client socket
        return -1;
    }

    int clientSocket = accept(serverSocket, NULL, NULL); //connects a client socket to the server socket


}
