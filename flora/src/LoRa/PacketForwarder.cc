//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "PacketForwarder.h"
//#include "inet/networklayer/ipv4/IPv4Datagram.h"
//#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "../LoRaPhy/LoRaRadioControlInfo_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"


namespace flora {

Define_Module(PacketForwarder);


void PacketForwarder::initialize(int stage)
{
    if (stage == 0) {
        LoRa_GWPacketReceived = registerSignal("LoRa_GWPacketReceived");
        localPort = par("localPort");
        destPort = par("destPort");
        // --- Initialize Custom Queues and Timer ---
        highPriorityQueue.setName("High Priority Queue");
        lowPriorityQueue.setName("Low Priority Queue");
        serviceTimer = new cMessage("serviceTimer");

        // --- Register the Signals ---
        highQueueLengthSignal = registerSignal("highQueueLength");
        lowQueueLengthSignal = registerSignal("lowQueueLength");

        isBusy = false;
    } else if (stage == INITSTAGE_APPLICATION_LAYER) {
        startUDP();
        getSimulation()->getSystemModule()->subscribe("LoRa_AppPacketSent", this);
    }
}


void PacketForwarder::startUDP()
{
    EV << "Wywalamy sie tutaj" << endl;
    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    EV << "Dojechalismy za pierwszy resolv" << endl;
    // TODO: is this required?
    //setSocketOptions();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    // Create UDP sockets to multiple destination addresses (network servers)
    while ((token = tokenizer.nextToken()) != nullptr) {
        EV << "Wchodze w petle" << endl;
        EV << token << endl;
        L3Address result;
        L3AddressResolver().tryResolve(token, result);
        EV << "Wychodze z petli" << endl;
        if (result.isUnspecified())
            EV_ERROR << "cannot resolve destination address: " << token << endl;
        else
            EV << "Got destination address: " << token << endl;
        destAddresses.push_back(result);
    }
    EV << "Dojechalismy do konca" << endl;
}


void PacketForwarder::handleMessage(cMessage *msg) // we are using "check_and_cast" to convert packet to modern format.
{
    // --- 1. THE CASHIER (Processing the Queue) ---
    if (msg == serviceTimer) {
        if (!highPriorityQueue.isEmpty()) {

            // FIX: Use INET's check_and_cast to safely pull the packet out
            Packet *pkt = check_and_cast<Packet*>(highPriorityQueue.pop());
            emit(highQueueLengthSignal, highPriorityQueue.getLength());
            EV << "Processing HIGH PRIORITY Emergency Packet! Queue length: " << highPriorityQueue.getLength() << endl;
            processLoraMACPacket(pkt);

            scheduleAt(simTime() + 0.5, serviceTimer);

        } else if (!lowPriorityQueue.isEmpty()) {

            // FIX: Use INET's check_and_cast
            Packet *pkt = check_and_cast<Packet*>(lowPriorityQueue.pop());
            emit(lowQueueLengthSignal, lowPriorityQueue.getLength());
            EV << "Processing Routine Humidity Packet. Queue length: " << lowPriorityQueue.getLength() << endl;
            processLoraMACPacket(pkt);

            scheduleAt(simTime() + 0.5, serviceTimer);

        } else {
            isBusy = false;
        }
        return;
    }


    // --- 2. THE WAITING ROOM (Receiving Packets) ---
    EV << msg->getArrivalGate() << endl;
    if (msg->arrivedOn("lowerLayerIn")) {
        EV << "Received LoRaMAC frame" << endl;
        auto pkt = check_and_cast<Packet*>(msg);
        const auto &frame = pkt->peekAtFront<LoRaMacFrame>();

        if(frame->getReceiverAddress() == MacAddress::BROADCAST_ADDRESS) {

                    MacAddress srcAddr = frame->getTransmitterAddress();

                    unsigned char macBytes[6];
                    srcAddr.getAddressBytes(macBytes);
                    int nodeId = macBytes[5];

                    if (nodeId <= 1) {
                        lowPriorityQueue.insert(pkt);
                        emit(lowQueueLengthSignal, lowPriorityQueue.getLength());
                        EV << "Placed in LOW Priority Queue. Sender MAC ID: " << nodeId << endl;
                    } else {
                        highPriorityQueue.insert(pkt);
                        emit(highQueueLengthSignal, highPriorityQueue.getLength());
                        EV << "Placed in HIGH Priority Queue. Sender MAC ID: " << nodeId << endl;
                    }

            if (!isBusy) {
                isBusy = true;
                scheduleAt(simTime(), serviceTimer);
            }
        } else {
             delete pkt;
        }

    } else if (msg->arrivedOn("socketIn")) {
        EV << "Received UDP packet" << endl;
        auto pkt = check_and_cast<Packet*>(msg);
        send(pkt, "lowerLayerOut");
    }
}

void PacketForwarder::processLoraMACPacket(Packet *pk)
{
    // FIXME: Change based on new implementation of MAC frame.
    emit(LoRa_GWPacketReceived, 42);
    if (simTime() >= getSimulation()->getWarmupPeriod())
        counterOfReceivedPackets++;
    pk->trimFront();
    auto frame = pk->removeAtFront<LoRaMacFrame>();

    auto snirInd = pk->getTag<SnirInd>();

    auto signalPowerInd = pk->getTag<SignalPowerInd>();

    W w_rssi = signalPowerInd->getPower();
    double rssi = w_rssi.get()*1000;
    frame->setRSSI(math::mW2dBmW(rssi));
    frame->setSNIR(snirInd->getMinimumSnir());
    pk->insertAtFront(frame);

    //bool exist = false;
    EV << frame->getTransmitterAddress() << endl;
    //for (std::vector<nodeEntry>::iterator it = knownNodes.begin() ; it != knownNodes.end(); ++it)

    // FIXME : Identify network server message is destined for.
    L3Address destAddr = destAddresses[0];
    if (pk->getControlInfo())
       delete pk->removeControlInfo();

    socket.sendTo(pk, destAddr, destPort);
}

void PacketForwarder::sendPacket()
{
//    LoRaAppPacket *mgmtCommand = new LoRaAppPacket("mgmtCommand");
//    mgmtCommand->setMsgType(TXCONFIG);
//    LoRaOptions newOptions;
//    newOptions.setLoRaTP(uniform(0.1, 1));
//    mgmtCommand->setOptions(newOptions);
//
//    LoRaMacFrame *response = new LoRaMacFrame("mgmtCommand");
//    response->encapsulate(mgmtCommand);
//    response->setLoRaTP(pk->getLoRaTP());
//    response->setLoRaCF(pk->getLoRaCF());
//    response->setLoRaSF(pk->getLoRaSF());
//    response->setLoRaBW(pk->getLoRaBW());
//    response->setReceiverAddress(pk->getTransmitterAddress());
//    send(response, "lowerLayerOut");

}

void PacketForwarder::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    if (simTime() >= getSimulation()->getWarmupPeriod())
        counterOfSentPacketsFromNodes++;
}

void PacketForwarder::finish()
{
    recordScalar("LoRa_GW_DER", double(counterOfReceivedPackets)/counterOfSentPacketsFromNodes);
}



} //namespace inet
