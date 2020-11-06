#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	// TODO(jesus): TCP connection stuff
	// - Create the socket
	socketClient = socket(AF_INET, SOCK_STREAM, 0);
	if (socketClient == INVALID_SOCKET) {
		reportError("socket");
		return false;
	}

	// - Create the remote address object
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	const char* remoteAddrStr = serverAddressStr;
	inet_pton(AF_INET, remoteAddrStr, &serverAddress.sin_addr);

	// - Connect to the remote address
	if (connect(socketClient, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
		reportError("connect");
		return false;
	}


	// - Add the created socket to the managed list of sockets using addSocket()
	addSocket(socketClient);

	// If everything was ok... change the state
	state = ClientState::Start;

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		//Sending the name with streams now
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;

		if (sendPacket(packet, socketClient)) {
			state = ClientState::Logging;
		}
		else
		{
			reportError("SendPacket in NetworkingClient Update");
			disconnect();
			state = ClientState::Stopped;
		}
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("%s connected to the server...", playerName.c_str());


		//Printing port and server adress of client
		char str[INET_ADDRSTRLEN];

		inet_ntop(AF_INET, &(serverAddress.sin_addr), str, INET_ADDRSTRLEN);

		ImGui::Text("port %i, addr %s", serverAddress.sin_port, str);
		
		//Chat window
		ImGui::BeginChild(1, { 390.0f, 420.0f }, true);
		ImGui::EndChild();

		//Chat input box
		static char chat_text[30] = "";
		ImGui::InputText("Chat", chat_text, IM_ARRAYSIZE(chat_text));

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream &packet)
{
	if (state == ClientState::Logging) {
		ClientMessage clientMessage;
		packet >> clientMessage;
		if (clientMessage == ClientMessage::Welcome) {
			std::string welcomMsg;
			packet >> welcomMsg;
			LOG(welcomMsg.c_str());
		}
	}

	//state = ClientState::Stopped;
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

