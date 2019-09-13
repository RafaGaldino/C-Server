#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <iostream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <fstream>

#define BUFLEN 2028

using namespace std;

void recv2(SOCKET SocketRecv, char Data[], int size){
  char response[3] = "OK";
  recv(SocketRecv,Data,size,0);
  cout << Data << endl;
  ZeroMemory(Data,size);
  send(SocketRecv,response,sizeof(response),0);
}

void recvFile(SOCKET SocketRecv, char ext[10]){
  float percent;
  unsigned int bytes, round, remainder, i = 1, number = 0;
  char recvData[BUFLEN], response[3] = "OK", name[] = "file", char_number[50], way[100];

  CONSOLE_CURSOR_INFO cursor = {1, FALSE};
  SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);

  number = rand();
  sprintf(char_number,"%d", number);
  strcpy(way,name);
  strcat(way,char_number);
  strcat(way,ext);
  send(SocketRecv,response,sizeof(response),0);

  recv(SocketRecv,recvData,sizeof(recvData),0);
  bytes = atoi(recvData);
  ZeroMemory(recvData,sizeof(recvData));
  send(SocketRecv,response,sizeof(response),0);

  ofstream out(way,ios::binary);

  round = bytes/sizeof(recvData);

  while(i <= round){
    recv(SocketRecv,recvData,sizeof(recvData),0);
    out.write(recvData,sizeof(recvData));
    ZeroMemory(recvData,sizeof(recvData));
    percent = (float) i/round*100;
    printf("\r%.0f%% Loading", percent);
    send(SocketRecv,response,sizeof(response),0);
    i++;
  }
  remainder = bytes % sizeof(recvData);
  recv(SocketRecv,recvData,sizeof(recvData),0);
  out.write(recvData,remainder);
  ZeroMemory(recvData,sizeof(recvData));
  send(SocketRecv,response,sizeof(response),0);
  printf("\r%.0f%% Complete!\n", percent);
  out.close();

  ZeroMemory(way,sizeof(way));
  CONSOLE_CURSOR_INFO cursor2 = {1, TRUE};
  SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor2);
}

int main(int argc, char *argv[]){
  int check;

  char host[20] = "192.168.0.102";
  char port[5] = "5050";

  if(argc >= 2) strcpy(host,*(argv + 1));
  if(argc >= 3) strcpy(port,*(argv + 2));

  // Initialize the Winsock
  WSADATA wsaData;
  check = WSAStartup(MAKEWORD(2,2),&wsaData);
  if(check != 0){
    cerr << "Can't initialize the Winsock, ERROR: " << check << endl;
    return(1);
  }

  //Loading the information in the struct getaddrinfo
  struct addrinfo hints,*result = NULL;

  ZeroMemory(&hints,sizeof(hints));// It Completes with zero the variables wasn't initialized
  hints.ai_family = AF_INET; //Family of address network
  hints.ai_socktype = SOCK_STREAM; // Type stream of the socket
  hints.ai_protocol = IPPROTO_TCP; // Type protocol

  //Loading host and port in the struct sockaddr
  check = getaddrinfo(host, port, &hints, &result);
  if(check != 0){
    cerr << "Function getaddrinfo() failed, ERROR: " << check << endl;
    WSACleanup();
    return(1);
  }

  //Creating the socket
  SOCKET ClientSocket = INVALID_SOCKET;

  ClientSocket = socket(result->ai_family,result->ai_socktype, result->ai_protocol);
  if (ClientSocket == INVALID_SOCKET) {
      cerr << "socket failed with error: " << WSAGetLastError() << endl;
      WSACleanup();
      return(1);
  }

  //Connecting with the server
  while(true){
    check = connect(ClientSocket,result->ai_addr, (int)result->ai_addrlen);
    if(check != SOCKET_ERROR) break;
  }
  freeaddrinfo(result);

  // Receiving and Sending data

  char recvData[BUFLEN];
  recv2(ClientSocket,recvData,BUFLEN);

  while(true){
    check = recv(ClientSocket,recvData,sizeof(recvData),0);
    if(check > 0){
      recvFile(ClientSocket, recvData);
      check = 0;
    }
    else break;
    ZeroMemory(recvData,sizeof(recvData));
  }

  //Close Socket and Winsock
  shutdown(ClientSocket,SD_SEND);
  closesocket(ClientSocket);
  WSACleanup();
  return(0);
}
