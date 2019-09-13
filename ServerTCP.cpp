#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <cstring>
#include <fstream>
#include <conio.h>

#define BUFLEN 2028

using namespace std;

void send2(SOCKET SocketSend, char data[], int size){
  int check;
  char feedback[3];
  check = send(SocketSend, data,size,0);
  if(check == SOCKET_ERROR){
    cerr << "send failed  wich error: " << WSAGetLastError() << endl;
    closesocket(SocketSend);
    WSACleanup();
  };
  ZeroMemory(data,size);
  recv(SocketSend,feedback,sizeof(feedback),0);
  ZeroMemory(feedback,sizeof(feedback));
}

void sendFile(SOCKET SocketSend, char filename[]){
  unsigned int bytes, round, remainder, i = 1, j = 0;
  char bytestxt[100], sendData[BUFLEN], *ext;
  float percent;

  CONSOLE_CURSOR_INFO cursor = {1, FALSE};
  SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);

  while(filename[j] != '\0'){
    if(filename[j] == '\\') filename[j] = '/';
    j++;
  }

  ifstream in(filename,ios::binary|ios::ate);
  if(!in.is_open()){
    cerr << "Error to open file" << endl;
    return;
  }

  ext = strrchr(filename,'.');

  strcpy(sendData,ext);
  send2(SocketSend,sendData,sizeof(sendData));

  bytes = in.tellg();
  sprintf(bytestxt,"%d",bytes);
  in.seekg(0,ios::beg);

  strcpy(sendData,bytestxt);
  send2(SocketSend,sendData,sizeof(sendData));

  round  = bytes/sizeof(sendData);

  while(i <= round){
    in.read(sendData,sizeof(sendData));
    send2(SocketSend,sendData,sizeof(sendData));
    percent = (float) i/round*100;
    printf("\r%.0f%% Loading", percent);
    i++;
  }
  remainder = bytes % sizeof(sendData);
  in.read(sendData,remainder);
  send2(SocketSend,sendData,remainder);
  printf("\r%.0f%% Complete!\n", percent);
  in.close();

  CONSOLE_CURSOR_INFO cursor2 = {1, TRUE};
  SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor2);
}


int main(int argc, char const *argv[]) {
  int check = 0; // Variable for to check errors

  char host[] = "192.168.0.102";
  char port[] = "5050";

  if(argc >= 2) strcpy(host,*(argv + 1));
  if(argc >= 3) strcpy(port,*(argv + 2));

  //Initializing Winsock2
  WSADATA wsaData;
  check = WSAStartup(MAKEWORD(2,2),&wsaData);
  if(check != 0){
    cerr << "Can't initialize the Winsock2, ERROR: " << check << endl;
    return(1);
  }else cout << "1) Winsock2 initialized successfully." << endl;

  //Loading the information in struct addrinfo
  struct addrinfo hints,*result;

  ZeroMemory(&hints,sizeof(hints)); //This code is used to initialize the viriables that were not initialized
  hints.ai_family = AF_INET;        //Family of the network address
  hints.ai_socktype = SOCK_STREAM;  //Type of the stream of the socket
  hints.ai_protocol = IPPROTO_TCP;  //Type of protocol that will be used
  hints.ai_flags = AI_PASSIVE;      //this code allows you to use the bind function

  //Loading the host and port in struct sockaddr of the struct addrinfo
  check = getaddrinfo(host,port,&hints, &result);
  if(check != 0){
    cerr << "The function getaddrinfo() failed, ERROR: " << check << endl;
    return(1);
  }else cout << "2) Information loaded in struct sockaddr." << endl;

  //Creating the Socket Server
  SOCKET HTTPServerSocket = INVALID_SOCKET;
  HTTPServerSocket = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
  if( HTTPServerSocket == INVALID_SOCKET){
    cerr << "The function socket() failed, ERROR: " << WSAGetLastError() << endl;
    freeaddrinfo(result);
    WSACleanup();
    return(1);
  }else cout << "3) The server socket was successfully created." << endl;

  //Bind host and port of server with PC system
  check = bind(HTTPServerSocket,result->ai_addr,(int)result->ai_addrlen);
  if(check == SOCKET_ERROR){
    cerr << "The function bind failed, ERROR: " << WSAGetLastError() << endl;
    closesocket(HTTPServerSocket);
    WSACleanup();
    return(1);
  }else cout << "4) Host and Port bind the PC system." << endl;

  //listening the host and port
  check = listen(HTTPServerSocket,SOMAXCONN);
  if(check == SOCKET_ERROR){
    cerr << "The function listen() failed, ERROR: " << WSAGetLastError() << endl;
    closesocket(HTTPServerSocket);
    WSACleanup();
    return(1);
  }else cout << "5) the server listening to the host and port." << endl;

  printf("\n >>> %s:%s >>> \n", host, port);

  //Accepting connection
  SOCKET Clients = INVALID_SOCKET;
  Clients = accept(HTTPServerSocket,NULL,NULL);
  if(Clients == INVALID_SOCKET){
    cerr << "The function accept() failed, ERROR: " << WSAGetLastError() << endl;
    closesocket(HTTPServerSocket);
    WSACleanup();
    return(1);
  }else cout << "\n---- BROWSER CONNECTED ----\n" << endl;

  //Receiving and Sending Data
  char sendData[BUFLEN], filename[100];
  strcpy(sendData,"CONNECTED");
  send2(Clients,sendData,BUFLEN);

  while(true){
    cout << "RGAServer>> ";
    cin.getline(filename,sizeof(filename));
    if(strcmp(filename,"quit") == 0) break;
    sendFile(Clients,filename);
    ZeroMemory(filename,sizeof(filename));
    cout << endl;
  }

  //Close Socket and Winsock
  closesocket(HTTPServerSocket);
  WSACleanup();
  return(0);
}
