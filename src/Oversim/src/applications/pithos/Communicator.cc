//
// Copyright (C) 2011 MIH Media lab, Stellenbosch University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @author John Gilmore
 */

#include "Communicator.h"

Define_Module(Communicator);

// initializeApp() is called when the module is being created.
// Use this function instead of the constructor for initializing variables.
void Communicator::initializeApp(int stage)
{
    // initializeApp will be called twice, each with a different stage.
    // stage can be either MIN_STAGE_APP (this module is being created),
    // or MAX_STAGE_APP (all modules were created).
    // We only care about MIN_STAGE_APP here.

    if (stage != MIN_STAGE_APP) return;

    // initialize our statistics variables
    numSent = 0;
    numReceived = 0;
    bytesSent = 0;
    bytesReceived = 0;

    // tell the GUI to display our variables
    WATCH(numSent);
    WATCH(numReceived);
    WATCH(bytesSent);
    WATCH(bytesReceived);

    bindToPort(2000);
}

// finish is called when the module is being destroyed
void Communicator::finishApp()
{
	simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);

	if (time >= GlobalStatistics::MIN_MEASURED) {

		globalStatistics->addStdDev("Pithos: Sent packets/s", numSent/time);
		globalStatistics->addStdDev("Pithos: Received packets/s", numReceived/time);

		globalStatistics->addStdDev("Pithos: Sent UDP Bytes/s", bytesSent / time);
		globalStatistics->addStdDev("Pithos: Received UDP Bytes/s", bytesReceived / time);
	}
}

void Communicator::pingTimeout(PingCall* pingCall, const TransportAddress& dest, cPolymorphic* context, int rpcId)
{
	cModule *GroupStorageModule = getParentModule()->getSubmodule("group_storage");
	//This extra step ensures that the submodules exist and also does any other required error checking
	GroupStorage *group_storage = check_and_cast<GroupStorage *>(GroupStorageModule);

	group_storage->pingTimeout(pingCall, dest, (GroupStorage::PeerStatsContext *)context, rpcId);
}

void Communicator::pingResponse(PingResponse* pingResponse, cPolymorphic* context, int rpcId, simtime_t rtt)
{
	cModule *GroupStorageModule = getParentModule()->getSubmodule("group_storage");
	//This extra step ensures that the submodules exist and also does any other required error checking
	GroupStorage *group_storage = check_and_cast<GroupStorage *>(GroupStorageModule);

	group_storage->pingResponse(pingResponse, (GroupStorage::PeerStatsContext *)context, rpcId, rtt);
}

void Communicator::handleTraceMessage(cMessage* msg)
{
	/*cModule *dht_storageModule = getParentModule()->getSubmodule("dht_storage");

	//This extra step ensures that the submodules exist and also does any other required error checking
	DHTStorage *dht_storage = check_and_cast<DHTStorage *>(dht_storageModule);

    dht_storage->handleTraceMessage(msg);*/

	error("Unexpected trace message received at the communicator.");
}

// handleTimerEvent is called when a timer event triggers
void Communicator::handleTimerEvent(cMessage* msg)
{
	error("Unexpected timer event received.");
}

void Communicator::handleUpperMessage (cMessage *msg)
{
	send(msg, "toPeer_fromUpper");		//This is the position update from the higher layer
}

simtime_t Communicator::getCreationTime()
{
	return creationTime;
}

void Communicator::handleGetCAPIRequest(RootObjectGetCAPICall* capiGetMsg)
{
	cModule *peer_logicModule = getParentModule()->getSubmodule("peer_logic");
	//This extra step ensures that the submodules exist and also does any other required error checking
	Peer_logic *peer_logic = check_and_cast<Peer_logic *>(peer_logicModule);

	peer_logic->handleGetCAPIRequest(capiGetMsg);
}

void Communicator::handlePutCAPIRequest(RootObjectPutCAPICall* capiPutMsg)
{
	cModule *peer_logicModule = getParentModule()->getSubmodule("peer_logic");
	//This extra step ensures that the submodules exist and also does any other required error checking
	Peer_logic *peer_logic = check_and_cast<Peer_logic *>(peer_logicModule);

	peer_logic->handlePutCAPIRequest(capiPutMsg);
}

bool Communicator::handleRpcCall(BaseCallMessage *msg)
{
	if (underlayConfigurator->isInInitPhase())
	{
		delete(msg);
		error("Underlay configurator is still in init phase, extend wait time.\n");
	}

    // There are many macros to simplify the handling of RPCs. The full list is in <OverSim>/src/common/RpcMacros.h.

    // start an RPC switch
    RPC_SWITCH_START(msg);
		// RPCs between nodes
		//RPC_DELEGATE(RootObjectPut, handlePutRequest);
		//RPC_DELEGATE(RootObjectGet, handleGetRequest);
		// internal RPCs
		RPC_DELEGATE(RootObjectPutCAPI, handlePutCAPIRequest);		//If we received a put request from Tier 2
		RPC_DELEGATE(RootObjectGetCAPI, handleGetCAPIRequest);		//If we received a get request from Tier 2
    // end the switch
    RPC_SWITCH_END();

    // return whether we handled the message or not.
    // don't delete unhandled messages!
    return RPC_HANDLED;
}

void Communicator::handleRpcResponse(BaseResponseMessage* msg, const RpcState& state, simtime_t rtt)
{
	cModule *dht_storageModule = getParentModule()->getSubmodule("dht_storage");

	//This extra step ensures that the submodules exist and also does any other required error checking
	DHTStorage *dht_storage = check_and_cast<DHTStorage *>(dht_storageModule);

    dht_storage->handleRpcResponse(msg, state, rtt);
}

// deliver() is called when we receive a message from the overlay.
// Unknown packets can be safely deleted here.
void Communicator::deliver(OverlayKey& key, cMessage* msg)
{
	//All messages received from the overlay, should be sent to the super peer
	//OVERLAY_WRITE is handled here
    send(msg, "sp_overlay_gate$o");
}

// handleUDPMessage() is called when we receive a message from UDP.
// Unknown packets can be safely deleted here.
void Communicator::handleUDPMessage(cMessage* msg)
{
	Packet *packet = check_and_cast<Packet *>(msg);

	RECORD_STATS(numReceived++);
	RECORD_STATS(bytesReceived += packet->getByteLength());

	if ((packet->getPayloadType() == WRITE) ||
			(packet->getPayloadType() == RESPONSE) ||
			(packet->getPayloadType() == JOIN_ACCEPT) ||
			(packet->getPayloadType() == INFORM) ||
			(packet->getPayloadType() == RETRIEVE_REQ) ||
			(packet->getPayloadType() == PEER_LEFT) ||
			(packet->getPayloadType() == PEER_JOIN) ||
			(packet->getPayloadType() == REPLICATION_REQ) ||
			(packet->getPayloadType() == REPLICATE) ||
			(packet->getPayloadType() == OBJECT_ADD))
	{
		send(msg, "gs_gate$o");
	} else if ((packet->getPayloadType() == JOIN_REQ) ||
			(packet->getPayloadType() == SP_OBJECT_ADD) ||
			(packet->getPayloadType() == SP_PEER_LEFT) ||
			(packet->getPayloadType() == SP_PEER_MIGRATED) ||
			(packet->getPayloadType() == OVERLAY_WRITE_REQ))
	{
		send(msg, "sp_group_gate$o");
	}
	else error("Communicator received unknown message from UDP");
}

void Communicator::sendPacket(cMessage *msg)
{
	Packet *pkt = check_and_cast<Packet *>(msg);

	if (underlayConfigurator->isInInitPhase())
	{
		delete(msg);
		error("Underlay configurator is still in init phase, extend wait time.");
	}

	if (pkt->getDestinationAddress().isUnspecified())
	{
		delete(msg);
		error("Communicator cannot send a packet over UDP with an unspecified destination address.");
	}

	sendMessageToUDP(pkt->getDestinationAddress(), pkt);

	RECORD_STATS(numSent++);
	RECORD_STATS(bytesSent += pkt->getByteLength());
}

void Communicator::handleSPMsg(cMessage *msg)
{
	sendPacket(msg);
}

void Communicator::handlePeerMsg(cMessage *msg)
{
	sendPacket(msg);
}

void Communicator::handleMessage(cMessage *msg)
{
	if (msg->arrivedOn("sp_group_gate$i"))
	{
		handleSPMsg(msg);
	}
	else if (msg->arrivedOn("peer_gate$i") || msg->arrivedOn("gs_gate$i") || msg->arrivedOn("os_gate$i"))
	{
		handlePeerMsg(msg);
	}
	else if (msg->arrivedOn("fromPeer_toUpper"))
	{
		send(msg, "to_upperTier");
	} else BaseApp::handleMessage(msg);
}
