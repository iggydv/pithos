//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @author Antonio Zea
 */

#include <string>

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

    // tell the GUI to display our variables
    WATCH(numSent);
    WATCH(numReceived);

    bindToPort(2000);
}


// finish is called when the module is being destroyed
void Communicator::finishApp()
{
    // finish() is usually used to save the module's statistics.
    // We'll use globalStatistics->addStdDev(), which will calculate min, max, mean and deviation values.
    // The first parameter is a name for the value, you can use any name you like (use a name you can find quickly!).
    // In the end, the simulator will mix together all values, from all nodes, with the same name.

    globalStatistics->addStdDev("MyApplication: Sent packets", numSent);
    globalStatistics->addStdDev("MyApplication: Received packets", numReceived);
}


// handleTimerEvent is called when a timer event triggers
void Communicator::handleTimerEvent(cMessage* msg)
{

}

void Communicator::handleUpperMessage (cMessage *msg)
{
	send(msg, "toPeer_fromUpper");		//This is the storage request from the game module
}

bool Communicator::handleRpcCall(BaseCallMessage *msg)
{

    // There are many macros to simplify the handling of RPCs. The full list is in <OverSim>/src/common/RpcMacros.h.

    // start a switch
    RPC_SWITCH_START(msg);

    // enters the following block if the message is of type MyNeighborCall (note the shortened parameter!)
    RPC_ON_CALL(DHTputCAPI) {
    	DHTputCAPICall* capiPutMsg = (DHTputCAPICall*)msg;          // get Call message

    }

    // end the switch
    RPC_SWITCH_END();

    // return whether we handled the message or not.
    // don't delete unhandled messages!
    return RPC_HANDLED;
}

// deliver() is called when we receive a message from the overlay.
// Unknown packets can be safely deleted here.
void Communicator::deliver(OverlayKey& key, cMessage* msg)
{
	//All messages received from the overlay, should be sent to the super peer
	//OVERLAY_WRITE is handled here
    send(msg, "sp_gate$o");
}

void Communicator::handlePkt(Packet *packet, int sp_type)
{
	if (packet->getPayloadType() == sp_type)
	{
		if (getParentModule()->getSubmodule("super_peer_logic") != NULL)
			send(packet, "sp_gate$o");
		else {
			EV << "A super peer packet type was received, but this peer (" << getParentModule()->getIndex() << ") is not a super peer. The request will be ignored.\n";
			delete(packet);
		}
	}
	else {
		EV << "Received packet of type " << packet->getPayloadType() << endl;
		send(packet, "peer_gate$o");
	}
}

// handleUDPMessage() is called when we receive a message from UDP.
// Unknown packets can be safely deleted here.
void Communicator::handleUDPMessage(cMessage* msg)
{
	Packet *packet = check_and_cast<Packet *>(msg);
	numReceived++;

	if (packet->getPayloadType() == WRITE)
	{
		send(msg, "peer_gate$o");
	}
	else if (packet->getPayloadType() == INFORM)
	{
		send(msg, "peer_gate$o");
	}
	else if (packet->getPayloadType() == JOIN_ACCEPT)
	{
		send(msg, "peer_gate$o");
	}
	else if (packet->getPayloadType() == JOIN_REQ)
	{
		send(msg, "sp_gate$o");
	}
	else if (packet->getPayloadType() == OVERLAY_WRITE_REQ)
	{
		send(msg, "sp_gate$o");
	}
	else error("Communicator received unknown message from UDP");
}

void Communicator::overlayStore(cMessage *msg)
{
	CSHA1 hash;
	char hash_str[41];		//SHA-1 produces a 160 bit/20 byte hash

	for (int i = 0 ; i < 50 ; i++)	//The string has to be cleared for the OverlayKey constructor to correctly handle it.
		hash_str[i] = 0;

	GameObject *go = (GameObject *)msg->getObject("GameObject");
	cPacket *pkt = check_and_cast<cPacket *>(msg);

	EV << "Sending object with name: " << go->getObjectName() << endl;

	//Create a hash of the game object's name
	hash.Update((unsigned char *)go->getObjectName(), strlen(go->getObjectName()));
	hash.Final();
	hash.ReportHash(hash_str, CSHA1::REPORT_HEX);

	OverlayKey nameKey(hash_str, 16);

	EV << thisNode.getIp() << ": Sending packet to " << nameKey << "!" << std::endl;

	callRoute(nameKey, pkt);
}

void Communicator::sendPacket(cMessage *msg)
{
	Packet *pkt = check_and_cast<Packet *>(msg);

	if (pkt->getDestinationAddress().isUnspecified())
	{
		delete(msg);
		error("Communicator cannot send a packet over UDP with an unspecified destination address.\n");
	}

	sendMessageToUDP(pkt->getDestinationAddress(), pkt);

	numSent++;
}

void Communicator::handleSPMsg(cMessage *msg)
{
	if (underlayConfigurator->isInInitPhase())
	{
		delete(msg);
		error("Underlay configurator is still in init phase, extend wait time.\n");
	}

	if (strcmp(msg->getName(), "overlay_write") == 0)
	{
		overlayStore(msg);
	}
	else sendPacket(msg);
}

void Communicator::handlePeerMsg(cMessage *msg)
{
	if (underlayConfigurator->isInInitPhase())
	{
		delete(msg);
		error("Underlay configurator is still in init phase, extend wait time.\n");
	}

	sendPacket(msg);
}

void Communicator::handleMessage(cMessage *msg)
{
	if (strcmp(msg->getArrivalGate()->getName(), "sp_gate$i") == 0)
	{
		handleSPMsg(msg);
	}
	else if (strcmp(msg->getArrivalGate()->getName(), "peer_gate$i") == 0)
	{
		handlePeerMsg(msg);
	}
	else if (strcmp(msg->getArrivalGate()->getName(), "fromPeer_toUpper") == 0)
	{
		send(msg, "to_upperTier");
	} else BaseApp::handleMessage(msg);
}