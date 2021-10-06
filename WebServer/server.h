#ifndef __SERVER_H
#define __SERVER_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include "server.h"
#include "request.h"
#include <windows.h>

const int TIME_PORT = 27015;
const int MAX_SOCKETS = 60;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;
const int SEND_TIME = 1;
const int SEND_SECONDS = 2;
const int BUFFER_SIZE = 8192;
const int TIMEOUT = 20;

typedef struct request Request;
struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	Request* request;
	char buffer[BUFFER_SIZE];
	int len;
	DWORD stamp;
	int isStamped;
};

bool addSocket(struct SocketState* sockets, int& socketsCount, SOCKET id, int what);
void removeSocket(struct SocketState* sockets, int& socketsCount, int index);
void acceptConnection(struct SocketState* sockets, int& socketsCount, int index);
void receiveMessage(struct SocketState* sockets, int& socketsCount, int index);
void sendMessage(struct SocketState* sockets, int& socketsCount, int index);

#endif