#include <GameServer.h>
#include <NetworkCommon.h>
#include <iostream>

void GameServer::processWaitingPackets(sf::UdpSocket& socket)
{
	sf::Socket::Status status;
	do
	{
		// We try to see if there is a message to process
		sf::Packet packet;
		sf::IpAddress remoteAddress;
		unsigned short remotePort;
		status = socket.receive(packet, remoteAddress, remotePort);
		if (status == sf::Socket::NotReady || status == sf::Socket::Disconnected)
			break;

		// We process the message
		processReceivedPacket(socket, packet, remoteAddress, remotePort);

	} while (true); // We exit this loop thanks to break instruction when ((status == sf::Socket::NotReady) || (status == sf::Socket::Disconnected))
}

void GameServer::processReceivedPacket(sf::UdpSocket& socket, sf::Packet& packet, sf::IpAddress& remoteAddress, unsigned short remotePort)
{
	sf::Uint32 msgType;
	packet >> msgType;
	ClientMsgType messageType = static_cast<ClientMsgType>(msgType);
	switch (messageType)
	{
	case ClientMsgType::ClientIdRequest:
	{
		sf::Uint32 newID = getNewClientID();
		ClientData client = ClientData(newID, remoteAddress, remotePort);
		
		sf::Packet toSend;
		toSend << newID << mClock.getElapsedTime();
		socket.send(toSend, remoteAddress, remotePort);

		break;
	}
	case ClientMsgType::Input:
	{
		break;
	}
	case ClientMsgType::PingResponse:
	{
		sf::Uint32 id;
		sf::Time timeSent;
		packet >> id >> timeSent;

		ClientData& client = getClientFromID(id);
		client.setDelay(sf::microseconds((mClock.getElapsedTime() - timeSent).asMicroseconds() / 2));

		break;
	}
	default:
		break;
	}
}

sf::Uint64 GameServer::getNewEntityID()
{
	sf::Uint64 id = mAvailableEntityIDs.top();
	mAvailableEntityIDs.pop();
	if (mAvailableEntityIDs.empty()) mAvailableEntityIDs.emplace(id + 1);
	return id;
}

sf::Uint32 GameServer::getNewClientID()
{
	sf::Uint32 id = mAvailableClientIDs.top();
	mAvailableClientIDs.pop();
	if (mAvailableClientIDs.empty()) mAvailableClientIDs.emplace(id + 1);
	return id;
}

ClientData& GameServer::getClientFromID(sf::Uint32 id)
{
	for (auto& client : mClients)
	{
		if (client.getID() == id) return client;
	}
	std::cerr << "Error: no client with such ID : " << id << std::endl;
	exit(EXIT_FAILURE);
}

void GameServer::sendPing(ClientData& client)
{
	sf::Packet packet;
	packet << static_cast<sf::Uint32>(ServerMsgType::PingRequest) << mClock.getElapsedTime();
	mSocket.send(packet, client.getAddress(), client.getPort());
}
