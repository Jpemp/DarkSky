#include <string>
#include <cstring>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib") //needs to be added or else socket programming won't work on Windows

using namespace std;

static bool connectFlag = false;

//function declarations
void temp_settings();
void fan_settings();
void time_settings();
void power_settings();
void system_data();

int main() {

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
        closesocket(clientSocket); //closes the socket connection
        WSACleanup();
        return -1;
    }
    else {
        cout << "ESP32 has connected to computer!" << endl; //Server and client are succesfully connected. Time to test sending and recieving messages.
        connectFlag = true;
        char commandMessage;
        char clieMessage[256] = "";

        while (connectFlag) {
            cout << "Settings:" << endl;
            cout << "(0): Exit" << endl;
            cout << "(1): System Data" << endl;
            cout << "(2): Fan" << endl;
            cout << "(3): Temperature Sensor" << endl;
            cout << "(4): Time Module" << endl;
            cout << "(5): System Power" << endl;

            cin >> commandMessage;
            cout << endl;
            //send(clientSocket, "Hello from Server", 256, 0);
            //recv(clientSocket, clieMessage, 256, 0);
            cout << clieMessage << endl;

            switch (commandMessage) {
                case '0':
                    cout << "Exiting program!" << endl;
                    connectFlag = false;
                    break;
                case '1':
                    system_data();
                    break;
                case '2':
                    fan_settings();
                    break;
                case '3':
                    temp_settings();
                    break;
                case '4':
                    time_settings();
                    break;
                case '5':
                    power_settings();
                    break;
                default:
                    cout << "That was an invalid entry! Please try again." << endl;
                    break;
            }
        }
        /*//const char servMessage[256] = "Server Message";
        char servMessage[256] = "Hello from Server";
        char clieMessage[256] = "";
            

        while (connectFlag) {
            
            //cin >> servMessage;
            send(clientSocket,servMessage, 256, 0);
            recv(clientSocket, clieMessage, 256, 0);
            connectFlag = false;

        }
        cout << clieMessage << endl;
        */
    }

    closesocket(clientSocket);

    return 0;
}

void temp_settings() {
    char exitChar;

    cout << "Temperature:" << endl;
    cout << "Current Temperature" << endl;
    cout << "Temperature-Fan Condition" << endl;
    cout << "(0): Exit" << endl;
    cout << "(1): Change Temperature Settings" << endl;
    while (true) {
        cin >> exitChar;
        if (exitChar == '0') {
            break;
        }
        else if (exitChar == '1') {
            //put temp change code here
        }
        else {
            cout << "Invalid Entry" << endl;
        }

    }

}
void fan_settings() {
    char exitChar;
    cout << "Fan:" << endl;
    cout << "Fan Speed" << endl;
    cout << "(0): Exit" << endl;
    cout << "(1): Change Fan Settings" << endl;
    while (true) {
        cin >> exitChar;
        if (exitChar == '0') {
            break;
        }
        else if (exitChar == '1') {
            //put fan change code here
        }
        else {
            cout << "Invalid Entry" << endl;
        }

    }
}
void time_settings() {
    char exitChar;
    cout << "Time Settings:" << endl;
    cout << "(0): Exit" << endl;
    cout << "(1): Change Time Settings" << endl;
    while (true) {
        cin >> exitChar;
        if (exitChar == '0') {
            break;
        }
        else if (exitChar == '1') {
            //put time change code here
        }
        else {
            cout << "Invalid Entry" << endl;
        }

    }
}
void power_settings() {
    char exitChar;
    cout << "Power Settings:" << endl;
    cout << "(0): Exit" << endl;
    cout << "(1): Turn On Recording System" << endl;
    cout << "(2): Turn On Fan" << endl;
    while (true) {
        cin >> exitChar;
        if (exitChar == '0') {
            break;
        }
        else if (exitChar == '1') {
            //put power code here
        }
        else if (exitChar == '2') {
            //put power code here
        }
        else {
            cout << "Invalid Entry" << endl;
        }

    }

}

void system_data() {
    char exitChar;
    cout << "System Settings:" << endl;
    cout << endl;
    cout << "Fan Speed:" << endl;
    cout << "Temperature:" << endl;
    cout << "Time:" << endl;
    cout << "(0): Exit" << endl;

    while (true) {
        cin >> exitChar;
        if (exitChar == '0') {
            break;
        }
        else {
            cout << "Invalid Entry" << endl;
        }
    }
}
