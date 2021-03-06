#pragma once

// Copyright 2014 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "generated/StellarXDR.h"
#include "overlay/Peer.h"

/*
Public interface to the Overlay module

Maintains the connection to the Stellar network
Handles flooding the appropriate messages to the rest of the world
There are 3 classes of messages:
-Messages to a particular Peer that want a response:
    Hello,  getPeers
-Messages that are flooded to the network:
    transaction, prepare, prepared, etc...
-Messages that you want a response from some peer but you don't care which.
Should keep rotating peers till you get an answer:
    getTxSet, getQuorumSet, ...


*/

namespace stellar
{

class PeerRecord;

class OverlayManager
{
  public:

    static std::unique_ptr<OverlayManager> create(Application& app);
    static void dropAll(Database& db);

    // called by Ledger
    virtual void ledgerClosed(LedgerHeaderHistoryEntry const& ledger) = 0;

    // called by Herder
    virtual void broadcastMessage(StellarMessage const& msg,
                                  bool force = false) = 0;

    // called internally
    virtual void recvFloodedMsg(StellarMessage const& msg,
                                Peer::pointer peer) = 0;
    virtual Peer::pointer getRandomPeer() = 0;
    // returns NULL if the passed peer isn't found
    virtual Peer::pointer getNextPeer(Peer::pointer peer) = 0;
    virtual Peer::pointer getConnectedPeer(const std::string& ip, int port) = 0;

    virtual void addConnectedPeer(Peer::pointer peer) = 0;
    virtual void dropPeer(Peer::pointer peer) = 0;
    virtual bool isPeerAccepted(Peer::pointer peer) = 0;
    virtual std::vector<Peer::pointer>& getPeers() = 0;

    virtual void connectTo(const std::string& addr) = 0;
    virtual void connectTo(PeerRecord& pr) = 0;

    virtual ~OverlayManager() {}

};
}
