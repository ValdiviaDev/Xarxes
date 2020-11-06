#include "ModuleNetworkingServer.h"




//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////



bool ModuleNetworkingServer::start(int port)
{
	// TODO(jesus): TCP listen socket stuff
	// - Create the listenSocket
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET) {
		reportError("socket");
		return false;
	}

	// - Set address reuse
	int enable = 1;

	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable)) == SOCKET_ERROR) {
		reportError("setsockopt");
		return false;
	}

	// - Bind the socket to a local interface
	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port);
	localAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(listenSocket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
		reportError("bind");
		return false;
	}

	// - Enter in listen mode
	int simultaneousConnections = 3;

	if (listen(listenSocket, simultaneousConnections) == SOCKET_ERROR) {
		reportError("listen");
		return false;
	}

	// - Add the listenSocket to the managed list of sockets using addSocket()
	addSocket(listenSocket);
	


	state = ServerState::Listening;

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, const InputMemoryStream &packet)
{
	// Set the player name
	ClientMessage clientMessage;
	packet >> clientMessage;

	std::string playerName;
	packet >> playerName;

	switch (clientMessage) {
	case ClientMessage::Hello: {
		
		for (auto &connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				connectedSocket.playerName = playerName;

				//Welcome message when entering the chat
				OutputMemoryStream packet;
				packet << ServerMessage::Welcome;
				packet << "Welcome to the chat room, " + playerName;
				sendPacket(packet, connectedSocket.socket);
			}
		}
		break;
	}
	case ClientMessage::ChatMessage: {

		std::string text;
		packet >> text;

		for (auto &connectedSocket : connectedSockets)
		{

			

			OutputMemoryStream packet;
			packet << ServerMessage::MessageAll;
			packet << playerName;
			packet << text;
			sendPacket(packet, connectedSocket.socket);

		}


		break;
	}
	}
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			connectedSockets.erase(it);
			break;
		}
	}
}

