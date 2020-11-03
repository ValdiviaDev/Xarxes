#include "Networks.h"
#include "ModuleNetworking.h"


static uint8 NumModulesUsingWinsock = 0;



void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}

	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	//**************NUEVO (hay qe arreglar)*********************************************
	InputMemoryStream packet;
	int bytesRead = recv(socket, packet.GetBufferPtr(), packet.GetCapacity(), 0);

	if (bytesRead > 0) {
		packet.SetSize((uint32)bytesRead);
		onSocketReceivedData(socket, packet);
	}
	//*********************************************************************************

	// NOTE(jesus): You can use this temporary buffer to store data from recv()
	const uint32 incomingDataBufferSize = Kilobytes(1);
	byte incomingDataBuffer[incomingDataBufferSize];

	// TODO(jesus): select those sockets that have a read operation available
	fd_set readSet; //Create a collection of sockets 
	FD_ZERO(&readSet);

	for (int i = 0; i < sockets.size(); ++i)
		FD_SET(sockets[i], &readSet);//Add a socket to a collection


	// Timeout (return immediately)
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	if (select(0, &readSet, nullptr, nullptr, &timeout) == SOCKET_ERROR) {
		reportError("select");
	}

	// TODO(jesus): for those sockets selected, check whether or not they are
	// a listen socket or a standard socket and perform the corresponding
	// operation (accept() an incoming connection or recv() incoming data,
	// respectively).
	// On accept() success, communicate the new connected socket to the
	// subclass (use the callback onSocketConnected()), and add the new
	// connected socket to the managed list of sockets.
	// On recv() success, communicate the incoming data received to the
	// subclass (use the callback onSocketReceivedData()).
	for (int i = 0; i < readSet.fd_count; ++i) {
		SOCKET auxSocket = readSet.fd_array[i];
		
		if (isListenSocket(auxSocket)) {
			sockaddr_in sockAddr;
			int addrSize = sizeof(sockAddr);

			SOCKET connectedSocket = accept(auxSocket, (sockaddr*)&sockAddr, &addrSize);

			if (connectedSocket == INVALID_SOCKET) {
				reportError("Oops, the accept function in failed");
				return false;
			}

			onSocketConnected(connectedSocket, sockAddr);
			addSocket(connectedSocket);
		}
		else {
			int receiving = recv(auxSocket, (char*)incomingDataBuffer, incomingDataBufferSize, 0);

			if (receiving == SOCKET_ERROR) {
				//Here the client disconnects
				HandleDisconnections(auxSocket);
	
			}
			else if (receiving == 0 || receiving == ECONNRESET) {
				//Here the server disconnects
				HandleDisconnections(auxSocket);
			}
			else {
				//This is the case where the receiving succeds
				onSocketReceivedData(auxSocket, incomingDataBuffer);
			}
		}
	}

	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}

void ModuleNetworking::HandleDisconnections(SOCKET toDisconnect)
{
	// TODO(jesus): handle disconnections. Remember that a socket has been
	// disconnected from its remote end either when recv() returned 0,
	// or when it generated some errors such as ECONNRESET.
	// Communicate detected disconnections to the subclass using the callback
	// onSocketDisconnected().
	onSocketDisconnected(toDisconnect);

	// TODO(jesus): Finally, remove all disconnected sockets from the list
	// of managed sockets.
	for (std::vector<SOCKET>::iterator it = sockets.begin(); it != sockets.end(); ++it)
	{
		if ((*it) == toDisconnect)
		{
			sockets.erase(it);
			break;
		}
	}
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}

bool ModuleNetworking::sendPacket(const OutputMemoryStream& packet, SOCKET socket)
{
	int result = send(socket, packet.GetBufferPtr(), packet.GetSize(), 0);
	if (result == SOCKET_ERROR)
	{
		reportError("send");
		return false;
	}
	return true;
}