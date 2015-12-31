
#pragma once

#include "FlowManager.h"

#define COOKIE_SIZE	0x40

/**************************************************
P2PConnection represents a direct P2P connection 
with another peer
*/
class P2PConnection : public FlowManager {
	friend class RTMFPConnection;
public:
	P2PConnection(FlowManager& parent, std::string id, Invoker* invoker, OnSocketError pOnSocketError, OnStatusEvent pOnStatusEvent, OnMediaEvent pOnMediaEvent, const Mona::SocketAddress& hostAddress, const Mona::Buffer& pubKey, bool responder);

	virtual ~P2PConnection() {
		close();
	}

	virtual Mona::UDPSocket&	socket() { return _parent.socket(); }

	// Add a command to the main stream (play/publish)
	virtual void addCommand(CommandType command, const char* streamName, bool audioReliable=false, bool videoReliable=false);

	// Return true if the stream exists, otherwise false (only for RTMFP connection)
	virtual bool getPublishStream(const std::string& streamName, bool& audioReliable, bool& videoReliable);

	const std::string				peerId; // Peer Id of the peer connected
	static Mona::UInt32				P2PSessionCounter; // Global counter for generating incremental P2P sessions id

	// Close the connection properly
	virtual void close();

	// Set the tag used for this connection (responder mode)
	void setTag(const std::string& tag) { _tag = tag; }

	// Return the tag used for this p2p connection (initiator mode)
	std::string	getTag() { return _tag; }

	// Manage all handshake messages (marker 0x0B)
	virtual void manageHandshake(Mona::Exception& ex, Mona::BinaryReader& reader);

	// Handle the first P2P responder handshake message (called by RTMFPConnection)
	void responderHandshake0(Mona::Exception& ex, std::string tag, const Mona::SocketAddress& address);

	// Handle the second P2P responder handshake message
	void responderHandshake1(Mona::Exception& ex, Mona::BinaryReader& reader);

	// Send the second P2P initiator handshake message in a middle mode (local)
	void initiatorHandshake70(Mona::Exception& ex, Mona::BinaryReader& reader, const Mona::SocketAddress& address);

	// Send the third P2P initiator handshake message
	bool initiatorHandshake2(Mona::Exception& ex, Mona::BinaryReader& reader);

	// Flush the connection
	// marker values can be :
	// - 0B for handshake
	// - 0A for raw response in P2P mode (only for responder)
	// - 8A for AMF responde in P2P mode (only for responder)
	// - 4A for acknowlegment in P2P mode (TODO: see if it is needed)
	virtual void				flush(bool echoTime, Mona::UInt8 marker);

	// Does the connection is terminated? => can be deleted by parent
	bool consumed() { return (_handshakeStep == 3 && connected == false); }

protected:
	// Handle stream creation (only for RTMFP connection)
	virtual void				handleStreamCreated(Mona::UInt16 idStream);

	// Handle play request (only for P2PConnection)
	virtual void				handlePlay(const std::string& streamName, FlashWriter& writer);

	// Handle a P2P address exchange message (Only for RTMFPConnection)
	virtual void				handleP2PAddressExchange(Mona::Exception& ex, Mona::PacketReader& reader);

private:
	FlowManager&				_parent; // RTMFPConnection related to
	Mona::UInt32				_sessionId; // id of the P2P session;
	std::string					_farKey; // Key of the server/peer

	// Play/Publish command
	std::string					_streamName;
	bool						_responder; // is responder?
};
