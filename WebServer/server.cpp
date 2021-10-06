#include "server.h"
#include "response.h"


void main()
{
	struct SocketState sockets[MAX_SOCKETS] = { 0 };
	int socketsCount = 0;

	
	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == listenSocket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(TIME_PORT);

	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Time Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	addSocket(sockets, socketsCount, listenSocket, LISTEN);

	while (true)
	{
		//Timer .
		time_t now = time(0);
		for (int i = 1; i < MAX_SOCKETS; i++)
		{
			if (now - sockets[i].stamp > TIMEOUT && sockets[i].isStamped == 1)
			{
				cout << sockets[i].id << " Closed due to timeout !!" << endl; //temp 
				closesocket(sockets[i].id);
				removeSocket(sockets, socketsCount, i);
			}
		} 

        fd_set waitRecv;
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv);
		}

		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}

		int nfd;
		timeval timeout{ 0,  0 };
		nfd = select(0, &waitRecv, &waitSend, NULL, &timeout);

		if (nfd == SOCKET_ERROR)
		{
			cout << "Time Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				nfd--;
				switch (sockets[i].send)
				{
				case SEND:
					sendMessage(sockets, socketsCount, i);
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv))
			{
				nfd--;
				switch (sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(sockets, socketsCount, i);
					break;

				case RECEIVE:
					receiveMessage(sockets, socketsCount, i);
					break;
				}
			}
		}

	}

	cout << "Time Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}


bool addSocket(struct SocketState *sockets, int& socketsCount, SOCKET id, int what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			sockets[i].isStamped= 0;
			socketsCount++;
			return (true);
		}
	}
	return (false);
}

void removeSocket(struct SocketState* sockets, int& socketsCount, int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	sockets[index].isStamped = 0;
	socketsCount--;
}

void acceptConnection(struct SocketState* sockets, int& socketsCount, int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Time Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}

	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "Time Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(sockets, socketsCount, msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}

	return;
}

void receiveMessage(struct SocketState* sockets, int& socketsCount, int index)
{
	SOCKET msgSocket = sockets[index].id;

	int len = sockets[index].len;
	int bytesRecv = recv(msgSocket, sockets[index].buffer, sizeof(sockets[index].buffer), 0);
	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(sockets, socketsCount, index);
		return;
	}
	if (bytesRecv == 0)
	{
		cout << sockets[index].id<<" Closed !" << endl; //temp 
		closesocket(msgSocket);
		removeSocket(sockets, socketsCount, index);
		return;
	}
	else
	{
		sockets[index].buffer[len + bytesRecv] = '\0'; 
		sockets[index].len += bytesRecv; 
		sockets[index].isStamped = 1;
		sockets[index].stamp = time(NULL);
		if (sockets[index].len > 0) {
			parseMessage(&sockets[index]);
		}	
	}
}

void sendMessage(struct SocketState* sockets, int& socketsCount, int index)
{
	int bytesSent = 0;
	int length = 0;
	char* response = createResponse(sockets,index);
	
	SOCKET msgSocket = sockets[index].id;

	bytesSent = send(msgSocket, response, (int)strlen(response), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	free(response);
	delete(sockets[index].request);
	sockets[index].send = IDLE;
}

