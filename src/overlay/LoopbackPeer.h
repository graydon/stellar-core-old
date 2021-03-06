#pragma once

// Copyright 2014 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "overlay/Peer.h"
#include <deque>
#include <random>

/*
Another peer out there that we are connected to
*/

namespace stellar
{
// [testing] Peer that communicates via byte-buffer delivery events queued in
// in-process io_services.
//
// NB: Do not construct one of these directly; instead, construct a connected
// pair of them wrapped in a LoopbackPeerConnection that explicitly manages the
// lifecycle of the connection.

class LoopbackPeer : public Peer
{
  private:
    std::shared_ptr<LoopbackPeer> mRemote;
    std::deque<xdr::msg_ptr> mQueue;

    bool mCorked{false};
    size_t mMaxQueueDepth{0};

    std::default_random_engine mGenerator;
    std::bernoulli_distribution mDuplicateProb{0.0};
    std::bernoulli_distribution mReorderProb{0.0};
    std::bernoulli_distribution mDamageProb{0.0};
    std::bernoulli_distribution mDropProb{0.0};

    struct Stats
    {
        size_t messagesDuplicated{0};
        size_t messagesReordered{0};
        size_t messagesDamaged{0};
        size_t messagesDropped{0};

        size_t bytesDelivered{0};
        size_t messagesDelivered{0};
    };

    Stats mStats;

    void sendMessage(xdr::msg_ptr&& xdrBytes);

  public:
    virtual ~LoopbackPeer()
    {
    }
    LoopbackPeer(Application& app, PeerRole role);
    void drop();
    std::string getIP();

    void deliverOne();
    void deliverAll();
    void dropAll();
    size_t getBytesQueued() const;
    size_t getMessagesQueued() const;

    Stats const& getStats() const;
    std::deque<xdr::msg_ptr>& getQueue();
    std::shared_ptr<LoopbackPeer> const& getTarget() const;

    bool getCorked() const;
    void setCorked(bool c);

    size_t getMaxQueueDepth() const;
    void setMaxQueueDepth(size_t sz);

    double getDamageProbability() const;
    void setDamageProbability(double d);

    double getDropProbability() const;
    void setDropProbability(double d);

    double getDuplicateProbability() const;
    void setDuplicateProbability(double d);

    double getReorderProbability() const;
    void setReorderProbability(double d);

    bool recvHello(StellarMessage const& msg);

    friend class LoopbackPeerConnection;
};

/**
* Testing class for managing a simulated network connection between two
* LoopbackPeers.
*/
class LoopbackPeerConnection
{
    std::shared_ptr<LoopbackPeer> mInitiator;
    std::shared_ptr<LoopbackPeer> mAcceptor;

  public:
    LoopbackPeerConnection(Application& initiator, Application& acceptor);
    ~LoopbackPeerConnection();
    std::shared_ptr<LoopbackPeer> getInitiator() const;
    std::shared_ptr<LoopbackPeer> getAcceptor() const;
};
}
