#pragma once
// Copyright 2014 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "generated/StellarXDR.h"
#include <string>
#include <memory>
#include <map>
#include "crypto/SecretKey.h"

#define DEFAULT_PEER_PORT 39133

namespace stellar
{
class HistoryArchive;

class Config : public std::enable_shared_from_this<Config>
{
  public:
    typedef std::shared_ptr<Config> pointer;

    enum TestDbMode
    {
        TESTDB_IN_MEMORY_SQLITE,
        TESTDB_ON_DISK_SQLITE,
        TESTDB_UNIX_LOCAL_POSTGRESQL,
        TESTDB_TCP_LOCALHOST_POSTGRESQL,
        TESTDB_MODES
    };

    // application config

    // The default way stellar-core starts is to load the state from disk and catch
    // up to the network before starting SCP.
    // If you need different behavior you need to use --new or --local which set
    // the following flags:

    // Will start a brand new ledger. And SCP will start running immediately
    // should only be used once to start a whole new network
    bool START_NEW_NETWORK;

    bool REBUILD_DB;

    // This is a mode for testing. It prevents you from trying to connect to
    // other peers
    bool RUN_STANDALONE;
    // Mode for testing. Ledger will only close when told to over http
    bool MANUAL_CLOSE;
    uint32_t PROTOCOL_VERSION;
    std::string VERSION_STR;
    std::string LOG_FILE_PATH;
    std::string TMP_DIR_PATH;
    std::string BUCKET_DIR_PATH;
    uint32_t DESIRED_BASE_FEE;     // in stroops
    uint32_t DESIRED_BASE_RESERVE; // in stroops
    uint32_t HTTP_PORT;    // what port to listen for commands on. 0 for don't
    bool PUBLIC_HTTP_PORT; // if you accept commands from not localhost

    // overlay config
    uint32_t PEER_PORT;
    SecretKey PEER_KEY;
    PublicKey PEER_PUBLIC_KEY;
    unsigned TARGET_PEER_CONNECTIONS;
    unsigned MAX_PEER_CONNECTIONS;
    // Peers we will always try to stay connected to
    std::vector<std::string> PREFERRED_PEERS;
    std::vector<std::string> KNOWN_PEERS;

    // SCP config
    SecretKey VALIDATION_KEY;
    uint32_t QUORUM_THRESHOLD;
    std::vector<uint256> QUORUM_SET;

    // History config
    std::map<std::string, std::shared_ptr<HistoryArchive>> HISTORY;

    // Database config
    std::string DATABASE;

    std::vector<std::string> COMMANDS;
    std::vector<std::string> REPORT_METRICS;

    Config();

    void load(const std::string& filename);
    void applyCommands();
};
}
