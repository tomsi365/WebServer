#ifndef __REQUEST_H
#define __REQUEST_H

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

struct SocketState;
struct request {
	char* buffer;
	char path[256];
	char method[32];
	char host[256];
	char accept[256]; // Supported: */*, text/* , text/html 
	char query[256];
	char acceptLanguage[256]; // Should only respond to en, hb
	char contentType[256]; // Content sent to server can only be text/html
	int contentLength;
	int isContentRange;
	char* bodyMessage;
};

typedef struct request Request;

void initRequest(struct SocketState* socket);
void parseRequestLine(struct SocketState* socket);
void parseMessage(struct SocketState* socket);
void deleteRequest(Request* req);

#endif