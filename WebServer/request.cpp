#include "request.h"

void initRequest(struct SocketState* socket)
{
	socket->request->buffer = NULL;
	strcpy(socket->request->method, "");
	strcpy(socket->request->contentType, "");
	strcpy(socket->request->query, "");
	strcpy(socket->request->path, "..\\ServerFiles\\");
	socket->request->contentLength = 0;
	strcpy(socket->request->acceptLanguage, "");
	strcpy(socket->request->accept, "");
	strcpy(socket->request->host, "");
	socket->request->isContentRange = 0;
	socket->request->bodyMessage = strdup(strstr(socket->buffer, "\r\n\r\n") + 4);
}

void parseRequestLine(struct SocketState* socket)
{
	// extract method
	char* token = strdup(socket->buffer);
	char* tokenOffset = token;
	token = strtok(token, " ");
	strcpy(socket->request->method, token);
	token[strlen(token)] = ' ';

	// extract path
	token = strstr(token, "/") + 1;
	if (token[0] == ' ')
		strcpy(socket->request->path, "/"); // alternatively can forward to index.html here
	else
	{
		token = strtok(token, " ");
		// check for query
		if (strstr(token, "?") != NULL)
		{
			token = strtok(token, "?");
			strcat(socket->request->path, token); // copy path
			token += strlen(token);
			token[0] = '?';
			strcpy(socket->request->query, token); // copy query
			token += strlen(token);
			token[0] = ' ';
		}
		else
		{
			strcat(socket->request->path, token);
			token += strlen(token);
			token[0] = ' ';
		}

	}
	
	free(tokenOffset);
}

void parseMessage(struct SocketState* socket)
{
	cout << socket->buffer << endl; 

	socket->send = SEND;
	socket->request = (Request*)malloc(sizeof(Request));
	int firstRequestLen = strlen(socket->buffer);

	initRequest(socket);
	socket->request->buffer = strdup(socket->buffer);
	parseRequestLine(socket);

	char* token = strdup(socket->buffer);
	token = strtok(token, "\r\n");
	while (token != NULL) {
		if (strncmp(token, "Content-Type:", 13) == 0) {
			strcpy(socket->request->contentType, token + 13 + 1);
		}
		else if (strncmp(token, "Content-Length:", 15) == 0)
		{
			socket->request->contentLength = atoi(token + 15 + 1);
		}
		else if (strncmp(token, "Content-Range:", 14) == 0)
		{
			socket->request->isContentRange = 1;
		}
		else if (strncmp(token, "Accept-Language:", 16) == 0)
		{
			strcpy(socket->request->acceptLanguage, token + 16 + 1);
		}
		else if (strncmp(token, "Accept:", 7) == 0)
		{
			strcpy(socket->request->accept, token + 7 + 1);
		}
		else if (strncmp(token, "Host:", 5) == 0)
		{
			strcpy(socket->request->host, token + 5 + 1);
		}
		token = strtok(NULL, "\r\n");
	}
	free(token);


	memcpy(socket->buffer, &socket->buffer[firstRequestLen], socket->len - firstRequestLen);
	socket->len -= firstRequestLen;
}

void deleteRequest(Request* req) {
	if (req != NULL) {
		if (req->bodyMessage != NULL) {
			free(req->bodyMessage);
		}
		if (req->buffer != NULL) {
			free(req->buffer);
		}
		free(req);
	}	
}