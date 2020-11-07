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
		
		//Printing chat messages 
		for (int i = 0; i < chat.size(); i++) {

			if (chat[i].server) {
				ImGui::TextColored({ 0.85,0.85,0.0,1 }, "%s: %s", chat[i].user.c_str(), chat[i].text.c_str());

			}
			else {
				if (chat[i].whisper) {
					ImGui::TextWrapped("%s whispered: %s", chat[i].user.c_str(), chat[i].text.c_str());
				}
				else {
					ImGui::TextWrapped("%s: %s", chat[i].user.c_str(), chat[i].text.c_str());
				}
			}
		}

		ImGui::EndChild();

		//Chat input box
		static char chat_text[1024] = "";
		if (ImGui::InputText("Chat", chat_text, IM_ARRAYSIZE(chat_text), ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue)) {

			OutputMemoryStream packet;
			packet << ClientMessage::ChatMessage;
			packet << playerName;
			packet << chat_text;

			//Send the chat message to the server
			if (!sendPacket(packet, socketClient)) {
				reportError("Chat message could not be sent.");
			}

			//To keep the input text focused after sending the message
			ImGui::SetKeyboardFocusHere(-1);

			//Delete line when sent
			strcpy_s(chat_text, "");
		}


		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream &packet)
{
	ServerMessage clientMessage;
	packet >> clientMessage;
	ChatLine line;
	if (state == ClientState::Logging) {

		switch (clientMessage) {
		//Server message
		case ServerMessage::Welcome:
		case ServerMessage::UserDisconnect:
		{
			packet >> line.text;
			line.user = "Server";
			line.server = true;

			chat.push_back(line);
			break;
		}
		case ServerMessage::MessageAll:

			packet >> line.user;
			packet >> line.text;

			chat.push_back(line);
			break;

		case ServerMessage::KickUser:
		case ServerMessage::UserExistsAlready:

			chat.clear();
			state = ClientState::Stopped;
			disconnectOne(socket);
			
			break;

		case ServerMessage::MessageServerAll:
			line.user = "Server";
			packet >> line.text;
			line.server = true;

			chat.push_back(line);
			break;

		case ServerMessage::UserList: {
			line.user = "Server";
			line.text = "\n :::LIST OF CONNECTED USERS:::\n";
			
			//Unwrap list of users and add them to the message of the chat
			std::vector<std::string> usernames;
			packet >> usernames;
			for (int i = 0; i < usernames.size(); ++i) {
				line.text += "\n" + usernames[i];
			}

			line.server = true;
			chat.push_back(line);
			break;
			}
		case ServerMessage::Whisper:

			
			packet >> line.user;
			packet >> line.text;
			line.whisper = true;

			chat.push_back(line);
			break;

		case ServerMessage::ChangeUsername:
			packet >> playerName;
			line.user = "Server";
			line.text = "New username: " + playerName;
			
			line.server = true;
			chat.push_back(line);
			break;

		}
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

