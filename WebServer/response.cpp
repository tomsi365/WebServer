#include "response.h"

bool checkValidHeaders(struct SocketState* sockets, int index)
{
	bool valid = true;
	
	if (strcmp(sockets[index].request->accept, "") != 0)
	{
		if (strstr(sockets[index].request->accept, "*/*") == NULL &&
			strstr(sockets[index].request->accept, "text/*") == NULL &&
			strstr(sockets[index].request->accept, "text/html") == NULL)
		{
			valid = false;
		}
	}
	else if (strcmp(sockets[index].request->acceptLanguage, "") != 0) 
	{
		if (strstr(sockets[index].request->acceptLanguage, "en") == NULL &&
			strstr(sockets[index].request->acceptLanguage, "he") == NULL &&
			strstr(sockets[index].request->acceptLanguage, "fr") == NULL)
		{
			valid = false;
		}
	}
		
	return valid;
}

char* createResponse(struct SocketState* sockets, int index)
{
	char* response = NULL;
	if (checkValidHeaders(sockets, index)) 
	{
		string method = sockets[index].request->method;
		int fileLength = 0;

		if (method == "GET")
		{
			response = getMethod(&sockets[index]);
		}
		else if (method == "HEAD")
		{
			response = headMethod(&sockets[index]);
		}
		else if (method == "OPTIONS")
		{
			response = optionsMethod(&sockets[index]);
		}
		else if (method == "PUT")
		{
			response = putMethod(&sockets[index]);
		}
		else if (method == "TRACE")
		{
			response = traceMethod(&sockets[index]);
		}
		else if (method == "DELETE")
		{
			response = deleteMethod(&sockets[index]);
		}
		else if (method == "POST")
		{
			response = postMethod(&sockets[index]);
		}
		else
		{
			response = stringfy(CODE_400, "text/html", NULL, NO_OPTIONS); //400 Bad Request
		}
	}
	else
	{
		response = stringfy(CODE_406, "text/html", NULL, NO_OPTIONS); //406 Not Acceptable (Header Check Failed)
	}
	
	cout << response << endl; 
	return response;
}

char* deleteMethod(SocketState* socket) {
	char* response = NULL;

	if (remove(socket->request->path) == 0) {
		response = stringfy(CODE_200, "text/html", NULL, NO_OPTIONS);
	}
	else {
		response = stringfy(CODE_500, "text/html", NULL, NO_OPTIONS); //500 Internal Server Error
	}

	return response;
}

char* headMethod(SocketState* socket) {
	char* response = NULL;

	char* file = readFile(socket->request->path);
	if (file != NULL) {
		free(file);
		response = stringfy(CODE_204, "text/html", NULL, NO_OPTIONS);
	}
	else {
		response = stringfy(CODE_404, "text/html", NULL, NO_OPTIONS);
	}

	return response;
}


char* optionsMethod(SocketState* socket) {
	char* response = NULL;

	char* file = readFile(socket->request->path);
	if (file != NULL) {
		free(file);
		response = stringfy(CODE_200, "text/html", NULL, OPTIONS);
	}
	else {
		response = stringfy(CODE_404, "text/html", NULL, NO_OPTIONS);
	}

	return response;
}


char* getMethod(SocketState* socket) {
	char* response = NULL;
	checkLanguageFileName(socket);
	
	char* file = readFile(socket->request->path);
	if (file != NULL) {
		response = stringfy(CODE_200, "text/html", file, NO_OPTIONS);
		free(file);
	}
	else {
		response = stringfy(CODE_404, "text/html", NULL, NO_OPTIONS);
	}

	return response;
}


char* postMethod(SocketState* socket) {
	char* response = NULL;

	cout << "received POST message with body: " << socket->request->bodyMessage << endl;

	if (isFileExist(socket->request->path))
	{
		appendFile(socket->request->path, socket->request->bodyMessage);
		response = stringfy(CODE_204, "text/html", NULL, NO_OPTIONS);
	}
	else
	{
		response = stringfy(CODE_404, "text/html", NULL, NO_OPTIONS);
	}

	return response;
}


char* traceMethod(SocketState* socket) {
	char* response = NULL;

	if (socket->request->contentLength!=0) { 
		response = stringfy(CODE_400, "text/html", NULL, NO_OPTIONS); //400 Bad Request
	}
	else {
		response = stringfy(CODE_200, "message/http", socket->request->buffer, NO_OPTIONS);
	}

	return response;
}

char* putMethod(SocketState* socket) {
	char* response = NULL;

	string contentType = socket->request->contentType;
	if (contentType != "text/html") {
		response = stringfy(CODE_415, "text/html", NULL, NO_OPTIONS); //415 Unsupported Media Type
	}
	else {
		if (socket->request->isContentRange) {
			response = stringfy(CODE_501, "text/html", NULL, NO_OPTIONS); //501 Not Implemented
		}
		else {
			if (isFileExist(socket->request->path)) {
				if (appendFile(socket->request->path, socket->request->bodyMessage)) {
					response = stringfy(CODE_204, "text/html", NULL, NO_OPTIONS);
				}
				else {
					response = stringfy(CODE_500, "text/html", NULL, NO_OPTIONS); //500 Internal Server Error
				}
			}
			else {
				if (appendFile(socket->request->path, socket->request->bodyMessage)) {
					response = stringfy(CODE_201, "text/html", NULL, NO_OPTIONS);
				}
				else {
					response = stringfy(CODE_500, "text/html", NULL, NO_OPTIONS); //500 Internal Server Error
				}
			}
		}
	}

	return response;
}

char* stringfy(const char* status, const char* type,char* bodyMessage, int options) {
	char* date = NULL;
	time_t timer;
	time(&timer);
	date = ctime(&timer);
	date[strlen(date) - 1] = '\0'; //cut '\n' 

	char* str;
		if (options == OPTIONS) // no body
		{
			str = (char*)malloc(HEADER_INIT_SIZE);
			sprintf(str, "HTTP/1.1 %s\n", status);
			sprintf(str+strlen(str), "Allow: OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE\n");
			sprintf(str+strlen(str), "Date: %s\nServer: Microsoft-IIS/8.5\nContent-Type: %s;\nContent-Length: 0\r\n\r\n", date, type);
		}
		else if (bodyMessage != NULL) {
			str = (char*)malloc(strlen(bodyMessage) + HEADER_INIT_SIZE);
			sprintf(str, "HTTP/1.1 %s\nDate: %s\nServer: Microsoft-IIS/8.5\nContent-Type: %s;\nContent-Length: %d\r\n\r\n%s", status, date, type, (int)strlen(bodyMessage), bodyMessage);
		}
		else {
			str = (char*)malloc(HEADER_INIT_SIZE);
			sprintf(str, "HTTP/1.1 %s\nDate: %s\nServer: Microsoft-IIS/8.5\nContent-Type: %s;\nContent-Length: 0\r\n\r\n", status, date, type);
		}


	return str;
}

int isFileExist(char* path) {
	FILE* file;
	if ((file = fopen(path, "r")))
	{
		fclose(file);
		return 1;
	}
	return 0;
}

char* readFile(char* path) {
	FILE* file = fopen(path, "r");
	char* str = NULL;
	if (file) {
		fseek(file, 0, SEEK_END);
		int physicalSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		if (physicalSize != EOF) {
			str = (char*)malloc(physicalSize + 1);
			physicalSize = fread(str, 1, physicalSize, file);
			if (physicalSize != EOF) {
				str[physicalSize] = '\0';
			}
			else {
				free(str);
				str = NULL;
			}
		}
		fclose(file);
	}
	return str;
}

int appendFile(char* path, char* str) {
	FILE* file = fopen(path, "a");
	if (file != NULL) {
		fprintf(file, "%s\r\n", str); // doesnt work on binary
		fclose(file);
		return 1;
	}
	else {
		return 0;
	}
}

void checkLanguageFileName(SocketState* socket)

{

		// error check
		int valid = 1;

		char* lang = strstr(socket->request->query, "lang=");
		if (lang != NULL)
		{
			lang--;
			if (lang[0] != '?' && lang[0] != '&')
				valid = 0;

			lang += 6;

			if (lang[2] != '\0' && lang[2] != '&')
				valid = 0;

			if (valid && (strncmp(lang, "he", 2) == 0 || strncmp(lang, "fr", 2) == 0))
			{
				char* fileName = socket->request->path;
				fileName += 15; // prefix filepath for server files

				char* buffer = (char*)malloc(sizeof(char) * (18 + strlen(fileName) + 1));
				sprintf(buffer, "..\\ServerFiles\\%s\\%s", lang, fileName);
				strcpy(socket->request->path, buffer);
				free(buffer);
			}

		}
		
	
}