#include <string>
#include <cstring>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib") //needs to be added or else socket programming won't work on Windows

using namespace std;

static int serverSocket;
static int clientSocket;

static bool connectFlag = false;
atomic <bool> exitFlag = false;

//function declarations
void temp_change();
void fan_change();
void time_change();
void power_settings();
void system_data();
void user_Input();
void server_menu();

int main() {

    int errorFlag; //will be used to handle and detect errors with winsock2.h functions

    //information to be used for socket IP address
    struct addrinfo server; //server creates a struct that contains server host information
    struct addrinfo* result = NULL; //pointer which will store host info through a linked list

    memset(&server, 0, sizeof(server)); //initializes the server struct with zeroes in every struct member so that unused members don't confuse and mess up the program further down the line. This was making the getaddrinfo function mess up

    server.ai_family = AF_INET; //specify to use the IPv4 address family
    server.ai_socktype = SOCK_STREAM; //IP address will be used for stream socket
    server.ai_protocol = IPPROTO_TCP; //TCP protocol will be used for communication for this address
    server.ai_flags = AI_PASSIVE; //indicates that this address will be used for socket binding
    char port_number[] = "8080"; //contains the port number for the socket. Figure out how to change this rather than using locking it in code so it doesn't need to recompile if other applications are using this port

    //cout << "test 1" << endl;
    WSADATA wsaData; //creates a structre to put the Windows socket data in
    errorFlag = WSAStartup(MAKEWORD(2, 2), &wsaData); //allows Winsock socket programming to work on Windows by initializing WS2_32.dll. MAKEWORD tells Windows to use Winsock version 2.2 and stores the Windows socket data in wsaData
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
    serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol); //creating the server socket
    if (serverSocket == INVALID_SOCKET) {
        cout << "Socket function failed: error " << endl;
        cout << WSAGetLastError() << endl;
        WSACleanup();
        return -1;
    }

    //cout << "test 4" << endl;
    errorFlag = bind(serverSocket, result->ai_addr, result->ai_addrlen); //binds the server socket
    if (errorFlag != 0) {
        cout << "Couldn't bind a socket: error ";
        cout << errorFlag << endl;
        WSACleanup();
        return -1;
    }

    //cout << "test 5" << endl;
    errorFlag = listen(serverSocket, 10); //enable the server socket to listen (wait) for the client socket
    if (errorFlag != 0) {
        cout << "Socket listening failed: error ";
        cout << errorFlag << endl;
        WSACleanup();
        return -1;
    }

    //cout << "test 6" << endl;
    cout << "Listening for client socket..." << endl;
    clientSocket = accept(serverSocket, NULL, NULL); //connects a client socket to the server socket
    if (clientSocket == INVALID_SOCKET) {
        cout << "Couldn't accept a socket: error ";
        cout << WSAGetLastError() << endl;
        closesocket(clientSocket); //closes the socket connection
        WSACleanup();
        return -1;
    }
    else {
        cout << "ESP32 has connected to computer!" << endl; //Server and client are succesfully connected. Time to test sending and recieving messages.
        server_menu();
    }

    closesocket(clientSocket);

    return 0;
}

void server_menu() {
    connectFlag = true;
    char commandMessage;
    char clieMessage[256] = "";

    //SIMPLIFY THIS WHERE THERE ISNT AS MANY MENUS. DATA VIEWING IN 2 3 AND 4 CAN BE CHANGED TO BE IN THE SAME WINDOW AS THE CHANGE CONDITION WINDOW


    while (connectFlag) {
        cout << "Settings:" << endl;
        cout << endl;
        cout << "(0): Exit" << endl;
        cout << "(1): View System Data" << endl;
        cout << "(2): Change Fan Speed" << endl;
        cout << "(3): Change Temperature Condition" << endl;
        cout << "(4): Change Time Schedule" << endl;
        cout << "(5): View System Power" << endl;

        cin >> commandMessage;
        //send(clientSocket, "Hello from Server", 256, 0);
        //recv(clientSocket, clieMessage, 256, 0);
        //cout << clieMessage << endl;

        switch (commandMessage) {
        case '0':
            cout << "Exiting program!" << endl;
            connectFlag = false;
            break;
        case '1':
            send(clientSocket, "0", 256, 0);
            system_data();
            break;
        case '2':
            send(clientSocket, "1", 256, 0);
            fan_change();
            break;
        case '3':
            send(clientSocket, "2", 256, 0);
            temp_change();
            break;
        case '4':
            send(clientSocket, "3", 256, 0);
            time_change();
            break;
        case '5':
            send(clientSocket, "4", 256, 0);
            power_settings();
            break;
        default:
            cout << "That was an invalid entry! Please try again." << endl;
            break;
        }
    }
}

void temp_change() {
    char exitChar;
    char tempChange[256] = "";
    char tempCondition[256] = "";

    recv(clientSocket, tempCondition, 256, 0);

    while (true) {
        cout << "Temperature Change:" << endl;
        cout << "(0): Exit" << endl;
        cout << "(1): Change Temperature" << endl;
        cout << "Temperature Condition: ";
        cout << tempCondition << endl;
        cin >> exitChar;
        if (exitChar == '0') {
            break;
        }
        else if (exitChar == '1') {
            send(clientSocket, "1", 256, 0);
            cout << "Enter New Temperature Condition" << endl;
            cin >> tempChange;

            send(clientSocket, tempChange, 256, 0);
            recv(clientSocket, tempCondition, 256, 0);
            
        }
        else {
            cout << "Invalid Entry. Please Try Again" << endl;
        }
    }
    memset(tempCondition, 0, sizeof(tempCondition));
    memset(tempChange, 0, sizeof(tempChange));
}

void fan_change() {
    char exitChar;
    char fanSpeed;
    bool exitFan = false;
    bool exitMenu = false;

    while(!exitMenu){
        cout << "Fan Change:" << endl;
        cout << "Fan Speed: " << endl;
        cout << "(0): Exit" << endl;
        cout << "(1): Change Fan Speed" << endl;
        cout << endl;
       
        cin >> exitChar;
        if (exitChar == '0') {
            exitMenu = true;
            break;
        }
        else if (exitChar == '1') {
            while (!exitFan) {
                cout << "(0): Off" << endl;
                cout << "(1): Low Speed" << endl;
                cout << "(2): Medium Speed" << endl;
                cout << "(3): High Speed" << endl;
                cout << "(4): Max Speed" << endl;
                cout << "Any Other Key: Cancel" << endl;
                cin >> fanSpeed;
                switch (fanSpeed) {
                case '0':
                    send(clientSocket, "0", 256, 0);
                    break;
                case '1':
                    send(clientSocket, "1", 256, 0);
                    break;
                case '2':
                    send(clientSocket, "2", 256, 0);
                    break;
                case '3':
                    send(clientSocket, "3", 256, 0);
                    break;
                case '4':
                    send(clientSocket, "4", 256, 0);
                    break;
                default:
                    exitFan = true;
                    break;
                }
            }
            exitFan = false;
        }
            else {
                cout << "Invalid Entry" << endl;
            }

    }
}

void time_change() { //still needs to be done
    char exitChar;
    char timeCommand;
    cout << "Time Settings:" << endl;
    cout << "(0): Exit" << endl;
    cout << "(1): Change Time Schedule" << endl;
    cout << "Current Time Schedule: " << endl;
    //insert schedule display here
    while (true) {
        cin >> exitChar;
        if (exitChar == '0') {
            break;
        }
        else if (exitChar == '1') {
            cout << "What would you like to do to the time schedule?" << endl;
            cout << "(0): Delete a time" << endl;
            cout << "(1): Add a new time" << endl;
            cout << "(2): Change a time in the system" << endl;
            cout << "Any Other Key: Cancel" << endl;
            cin >> timeCommand;
            if (timeCommand == '0') {
                cout << "Which schedule would you like to delete?" << endl;
            }
            else if (timeCommand == '1') {
                cout << "Enter a new time to add: " << endl;
            }
            else if (timeCommand == '2') {
                cout << "Which schedule would you like to change?" << endl;
            }
            else {
                continue;
            }
        }
        else {
            cout << "Invalid Entry. Please Try Again" << endl;
        }

    }
}

void power_settings() {
    bool loopBreak = false;
    char exitChar;
    cout << "Power Settings:" << endl;
    cout << "(0): Exit" << endl;
    cout << "(1): Turn On Recording System" << endl;
    cout << "(2): Turn Off Recording System" << endl;
    cout << "(3): Turn On Fan" << endl;
    cout << "(4): Turn Off Fan" << endl;

    while (!loopBreak) {

        cin >> exitChar;
        switch (exitChar) {
        case '0':
            send(clientSocket, "0", 256, 0);
            loopBreak = true;
            break;
        case '1':
            send(clientSocket, "1", 256, 0);
            break;
        case '2':
            send(clientSocket, "2", 256, 0);
            break;
        case '3':
            send(clientSocket, "3", 256, 0);
            break;
        case '4':
            send(clientSocket, "4", 256, 0);
            break;
        default:
            cout << "Invalid Entry. Please Try Again." << endl;
            break;
        }
    }
}

void system_data() {
    thread t1(user_Input);
    char tempRead[256] = "";
    char timeRead[256] = "";
    char fanRead[256] = "";
    char deviceOn[1];
    char fanOn[1];
    cout << "System Settings:" << endl;
    cout << "(0): Exit" << endl;
    cout << endl;

    //chrono::seconds(5);

    while (!exitFlag) {
        recv(clientSocket, deviceOn, 1, 0);
        cout << "Recording Device: ";
        if (deviceOn[0] == '1') {
            cout << "On" << endl;
        }
        else {
            cout << "Off" << endl;
        }
        //chrono::seconds(1);
        recv(clientSocket, fanOn, 1, 0);
        cout << "Fan: ";
        if (fanOn[0] == '1') {
            cout << "On" << endl;
        }
        else {
            cout << "Off" << endl;
        }
        //chrono::seconds(1);
        recv(clientSocket, tempRead, 256, 0);
        cout << "Temperature: ";
        cout << tempRead << endl;
        //chrono::seconds(1);
        recv(clientSocket, timeRead, 256, 0);
        cout << "Time: ";
        cout << timeRead << endl;
        //chrono::seconds(1);
        /*recv(clientSocket, fanRead, 256, 0);
        cout << "Fan Speed: ";
        cout << fanRead << endl;*/
        chrono::seconds(1);

    }
    //cout << "Exit Loop" << endl;
    exitFlag = false;
    t1.join();
    cout << "Exiting..." << endl;

}

void user_Input(void) {
    char exitChar;
    do {
        cin >> exitChar;
    } while (exitChar != '0');
    cout << "Exit thread loop" << endl;
    exitFlag = true;
    send(clientSocket, "0", 256, 0);

}
