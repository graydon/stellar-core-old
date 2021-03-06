#pragma once

// Copyright 2014 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "Peer.h"
#include "PeerDoor.h"
#include "PeerRecord.h"
#include "overlay/ItemFetcher.h"
#include "overlay/Floodgate.h"
#include <vector>
#include <thread>
#include "generated/StellarXDR.h"
#include "overlay/OverlayManager.h"
#include "util/Timer.h"

namespace medida
{
class Meter;
class Counter;
}

/*
Maintain the set of peers we are connected to
*/
namespace stellar
{

class OverlayManagerImpl : public OverlayManager
{
  protected:
    Application& mApp;
    // peers we are connected to
    std::vector<Peer::pointer> mPeers;
    PeerDoor::pointer mDoor;

    medida::Meter& mMessagesReceived;
    medida::Meter& mMessagesBroadcast;
    medida::Meter& mConnectionsAttempted;
    medida::Meter& mConnectionsEstablished;
    medida::Meter& mConnectionsDropped;
    medida::Counter& mPeersSize;

    void tick();
    VirtualTimer mTimer;

    void storePeerList(const std::vector<std::string>& list, int rank);
    void storeConfigPeers();
    bool isPeerPreferred(Peer::pointer peer);

    friend class OverlayManagerTests;

  public:
    Floodgate mFloodGate;

    OverlayManagerImpl(Application& app);
    ~OverlayManagerImpl();

    void ledgerClosed(LedgerHeaderHistoryEntry const& ledger) override;
    void recvFloodedMsg(StellarMessage const& msg, Peer::pointer peer) override;
    void broadcastMessage(StellarMessage const& msg, bool force = false) override;

    void connectTo(const std::string& addr) override;
    virtual void connectTo(PeerRecord& pr) override;

    void addConnectedPeer(Peer::pointer peer) override;
    void dropPeer(Peer::pointer peer) override;
    bool isPeerAccepted(Peer::pointer peer) override;
    std::vector<Peer::pointer>& getPeers() override;

    // returns NULL if the passed peer isn't found
    Peer::pointer getNextPeer(Peer::pointer peer) override;
    Peer::pointer getConnectedPeer(const std::string& ip, int port) override;

    void connectToMorePeers(int max);
    Peer::pointer getRandomPeer() override;
};
}
