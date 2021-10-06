#ifndef __RESPONSE_H
#define __RESPONSE_H

#pragma warning( disable : 6387 )
#include "server.h"
#include <string.h>
#include <windows.h>
using namespace std;

#define CODE_200 "200 OK"
#define CODE_404 "404 Not Found"
#define CODE_204 "204 No Content"
#define CODE_415 "415 Unsupported Media Type"
#define CODE_201 "201 Created"
#define CODE_403 "403 FORBIDDEN"
#define CODE_501 "501 Not Implemented"
#define CODE_500 "500 Internal Server Error"
#define CODE_400 "400 Bad Request"
#define CODE_406 "406 Not Acceptable"
#define OPTIONS 1
#define NO_OPTIONS 0

const int HEADER_INIT_SIZE=1024;

bool checkValidHeaders(struct SocketState* sockets, int index);
char* createResponse(struct SocketState* sockets, int index);
char* getMethod(SocketState* socket);
char* headMethod(SocketState* socket);
char* optionsMethod(SocketState* socket);
char* deleteMethod(SocketState* socket);
char* postMethod(SocketState* socket);
char* traceMethod(SocketState* socket);
char* putMethod(SocketState* socket);
char* stringfy(const char* status, const char* type, char* bodyMessage, int options);
int isFileExist(char* path);
char* readFile(char* path);
int appendFile(char* path, char* str);
void checkLanguageFileName(SocketState* socket);

#endif