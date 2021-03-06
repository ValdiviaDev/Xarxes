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
				OutputMemoryStream packet;

				//Checking if the name is already in use
				for (auto &auxSocket : connectedSockets)
				{
					if (playerName == auxSocket.playerName)
					{
						packet << ServerMessage::UserExistsAlready;
						sendPacket(packet, socket);
						continue;
					}
				}
				connectedSocket.playerName = playerName;

				//Welcome message when entering the chat
				packet << ServerMessage::Welcome;
				packet << "Welcome to the chat room, " + playerName;
				sendPacket(packet, connectedSocket.socket);
			}
			else
			{
				//Notify all the other users if someone enters the room
				OutputMemoryStream packet;
				packet << ServerMessage::Welcome;
				packet << playerName + " just entered the room";
				sendPacket(packet, connectedSocket.socket);
			}
		}
		break;
	}
	case ClientMessage::ChatMessage: {

		std::string text;
		packet >> text;

		if (text[0] == '/') {
			//Text is a command
		

			if (text == "/help") {

					OutputMemoryStream packet;
					packet << ServerMessage::MessageServerAll;
					packet << "\n:::CURRENT AVAILABLE COMMANDS:::\n\n/help: view command list\n/list: view list of connected users\n/kick (username): kick out user from the chat room\n/whisper (username): initiate private chat with user\n/change_name (name): change your username\n";

					for (auto &connectedSocket : connectedSockets) {
						if (connectedSocket.socket == socket)
							sendPacket(packet, connectedSocket.socket);
					}
			}
			else if (text.find("/kick ") != std::string::npos) {
				playerName = text.substr(6); //6 = kick length + 1 for spacebar
				
				for (auto &auxSocket : connectedSockets)
				{
					if (playerName == auxSocket.playerName)
					{
						OutputMemoryStream packet;
						packet << ServerMessage::KickUser;
						sendPacket(packet, auxSocket.socket);
					}
				}

			}
			else if (text.find("/whisper ") != std::string::npos) {
				std::string name = text.substr(9);
			
				int spacebar_pos = name.find(" ");

				if (spacebar_pos != std::string::npos) {

					std::string text_whisper = name;
					name = text_whisper.substr(0, spacebar_pos);
					text_whisper = text_whisper.substr(spacebar_pos + 1);

					bool user_found = false;

					for (auto &auxSocket : connectedSockets)
					{
						if (name == auxSocket.playerName)
						{
							user_found = true;

							OutputMemoryStream packet;
							packet << ServerMessage::Whisper;
							packet << playerName;
							packet << text_whisper;
							sendPacket(packet, auxSocket.socket);

							OutputMemoryStream packet2;
							packet2 << ServerMessage::Whisper;
							packet2 << "you";
							packet2 << text_whisper;
							sendPacket(packet2, socket);
						}
					}

					if (!user_found) {

						OutputMemoryStream packet2;
						packet2 << ServerMessage::MessageServerAll;
						packet2 << "User not found.";
						sendPacket(packet2, socket);
					}
				}


			}
			else if (text == "/list") {
				//Send a list of all connected users
				OutputMemoryStream packet;
				packet << ServerMessage::UserList;
				
				std::vector<std::string> usernames;
				for (auto &connectedSocket : connectedSockets)
					usernames.push_back(connectedSocket.playerName);

				packet << usernames;

				for (auto &connectedSocket : connectedSockets) {
					if (connectedSocket.socket == socket)
						sendPacket(packet, connectedSocket.socket);
				}

			}
			else if (text.find("/change_name ") != std::string::npos) {
				playerName = text.substr(13); //13 = change_name length + 1 for spacebar
				bool name_repeated = false;

				for (auto &connectedSocket : connectedSockets)
				{
					if (connectedSocket.socket == socket)
					{
						//Check that nobody else in the room has the chosen name
						for (auto &connectedSocketAux : connectedSockets) {
							if (connectedSocketAux.playerName == playerName && connectedSocketAux.socket != connectedSocket.socket) {
								OutputMemoryStream packet;
								packet << ServerMessage::MessageServerAll;
								packet << "Someone already has that name! Try another one";
								sendPacket(packet, connectedSocket.socket);

								name_repeated = true;
							}
							
						}

						//If nobody else has this name in the room, change it
						if (!name_repeated) {
							//Change user name in the user client
							OutputMemoryStream packet;
							packet << ServerMessage::ChangeUsername;
							packet << playerName;
							sendPacket(packet, connectedSocket.socket);

							//Change the user name in the server vector
							connectedSocket.playerName = playerName;
						}
					}
				}

			} else {
				//TODO send message to the user saying that command doesnt exists, type /help
				int TODO = 0;
			}

		}
		else {
			//Text isn't a command

			for (auto &connectedSocket : connectedSockets)
			{
				OutputMemoryStream packet;
				packet << ServerMessage::MessageAll;
				packet << playerName;
				packet << text;
				sendPacket(packet, connectedSocket.socket);
			}
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
			for (auto &conSock : connectedSockets)
			{
				OutputMemoryStream packet;
				packet << ServerMessage::UserDisconnect;
				packet << connectedSocket.playerName + " has disconnected";
				sendPacket(packet, conSock.socket);
			}

			connectedSockets.erase(it);
			break;
		}
	}
}

