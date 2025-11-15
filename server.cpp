#include <iostream>
#include "netHeader.h"

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};



class CustomServer : public net::serverInterface<CustomMsgTypes>
{
public:
	CustomServer(uint16_t nPort) : net::serverInterface<CustomMsgTypes>(nPort)
	{

	}

protected:
	virtual bool onClientConnect(std::shared_ptr<net::connection<CustomMsgTypes>> client)
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	virtual void onClientDisconnect(std::shared_ptr<net::connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->getID() << "]\n";
	}

	// Called when a message arrives
	virtual void onMsg(std::shared_ptr<net::connection<CustomMsgTypes>> client, net::message<CustomMsgTypes>& msg)
	{
		switch (msg.header.id)
		{
		case CustomMsgTypes::ServerPing:
		{
			std::cout << "[" << client->getID() << "]: Server Ping\n";

			// Client đã gửi giá trị uint64_t (timeValue)
			// Server không cần phải làm gì ngoài việc chuyển tiếp tin nhắn (msg) trở lại,
			// vì tin nhắn đó vẫn chứa uint64_t đó trong body.
			
			// Simply bounce message back to client
			client->send(msg); 
		}
		break;

		case CustomMsgTypes::MessageAll:
		{
			std::cout << "[" << client->getID() << "]: Message All\n";

			// Construct a new message and send it to all clients
			net::message<CustomMsgTypes> msg;
			msg.header.id = CustomMsgTypes::ServerMessage;
			msg << client->getID();
			msgAllClients(msg, client);

		}
		break;
		}
	}
};

int main()
{
	CustomServer server(60000); 
	server.start();

	while (1)
	{
		server.update(-1, true);
	}
	


	return 0;
}