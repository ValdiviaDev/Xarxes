#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	ChatMessage,
	Welcome
};

enum class ServerMessage
{
	Welcome,
	MessageAll,
	UserExistsAlready,
	MessageServerAll,
	UserDisconnect,
	KickUser,
	UserList,
	Whisper,
	ChangeUsername
};

