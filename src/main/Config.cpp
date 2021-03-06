// Copyright 2014 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "main/Config.h"
#include "history/HistoryArchive.h"
#include "generated/StellarCoreVersion.h"
#include "lib/util/cpptoml.h"
#include "util/Logging.h"
#include "util/types.h"
#include "crypto/Base58.h"

namespace stellar
{
Config::Config() : PEER_KEY(SecretKey::random())
{
    // fill in defaults

    // non configurable
    PROTOCOL_VERSION = 1;
    VERSION_STR = STELLAR_CORE_VERSION;
    REBUILD_DB = false;
    DESIRED_BASE_RESERVE = 10000000;

    // configurable
    DESIRED_BASE_FEE = 10;
    PEER_PORT = DEFAULT_PEER_PORT;
    RUN_STANDALONE = false;
    MANUAL_CLOSE = false;
    TARGET_PEER_CONNECTIONS = 20;
    MAX_PEER_CONNECTIONS = 50;
    LOG_FILE_PATH = "stellar-core.log";
    TMP_DIR_PATH = "tmp";
    BUCKET_DIR_PATH = "buckets";
    QUORUM_THRESHOLD = 1000;
    HTTP_PORT = 39132;
    PUBLIC_HTTP_PORT = false;
    PEER_PUBLIC_KEY = PEER_KEY.getPublicKey();

    DATABASE = "sqlite3://:memory:";
}

void
Config::load(const std::string& filename)
{
    try
    {
        cpptoml::toml_group g;
        if (filename == "-")
        {
            cpptoml::parser p(std::cin);
            g = p.parse();
        }
        else
        {
            g = cpptoml::parse_file(filename);
        }

        for (auto& item : g)
        {
            if (item.first == "PEER_PORT")
                PEER_PORT = (int)item.second->as<int64_t>()->value();
            else if (item.first == "HTTP_PORT")
                HTTP_PORT = (int)item.second->as<int64_t>()->value();
            else if (item.first == "PUBLIC_HTTP_PORT")
                PUBLIC_HTTP_PORT = item.second->as<bool>()->value();
            else if (item.first == "QUORUM_THRESHOLD")
                QUORUM_THRESHOLD = (int)item.second->as<int64_t>()->value();
            else if (item.first == "DESIRED_BASE_FEE")
                DESIRED_BASE_FEE =
                    (uint32_t)item.second->as<int64_t>()->value();
            else if (item.first == "RUN_STANDALONE")
                RUN_STANDALONE = item.second->as<bool>()->value();
            else if (item.first == "MANUAL_CLOSE")
                MANUAL_CLOSE = item.second->as<bool>()->value();
            else if (item.first == "LOG_FILE_PATH")
                LOG_FILE_PATH = item.second->as<std::string>()->value();
            else if (item.first == "TMP_DIR_PATH")
                TMP_DIR_PATH = item.second->as<std::string>()->value();
            else if (item.first == "BUCKET_DIR_PATH")
                BUCKET_DIR_PATH = item.second->as<std::string>()->value();
            else if (item.first == "VALIDATION_SEED")
            {
                std::string seed = item.second->as<std::string>()->value();
                VALIDATION_KEY = SecretKey::fromBase58Seed(seed);
            }
            else if (item.first == "PEER_SEED")
            {
                std::string seed = item.second->as<std::string>()->value();
                PEER_KEY = SecretKey::fromBase58Seed(seed);
                PEER_PUBLIC_KEY = PEER_KEY.getPublicKey();
            }
            else if (item.first == "TARGET_PEER_CONNECTIONS")
                TARGET_PEER_CONNECTIONS =
                    (int)item.second->as<int64_t>()->value();
            else if (item.first == "MAX_PEER_CONNECTIONS")
                MAX_PEER_CONNECTIONS = (int)item.second->as<int64_t>()->value();
            else if (item.first == "PREFERRED_PEERS")
            {
                for (auto v : item.second->as_array()->array())
                {
                    PREFERRED_PEERS.push_back(v->as<std::string>()->value());
                }
            }
            else if (item.first == "KNOWN_PEERS")
            {
                for (auto v : item.second->as_array()->array())
                {
                    KNOWN_PEERS.push_back(v->as<std::string>()->value());
                }
            }
            else if (item.first == "QUORUM_SET")
            {
                for (auto v : item.second->as_array()->array())
                {
                    uint256 p = fromBase58Check256(
                        VER_ACCOUNT_ID, v->as<std::string>()->value());
                    QUORUM_SET.push_back(p);
                }
            }
            else if (item.first == "COMMANDS")
            {
                for (auto v : item.second->as_array()->array())
                {
                    COMMANDS.push_back(v->as<std::string>()->value());
                }
            }
            else if (item.first == "HISTORY")
            {
                auto hist = item.second->as_group();
                if (hist)
                {
                    for (auto const& archive : *hist)
                    {
                        auto tab = archive.second->as_group();
                        if (!tab)
                        {
                            throw std::invalid_argument("malformed HISTORY config block");
                        }
                        std::string get, put, mkdir;
                        auto gg = tab->get_as<std::string>("get");
                        auto pp = tab->get_as<std::string>("put");
                        auto mm = tab->get_as<std::string>("mkdir");
                        if (gg)
                            get = *gg;
                        if (pp)
                            put = *pp;
                        if (mm)
                            mkdir = *mm;
                        HISTORY[archive.first] =
                            std::make_shared<HistoryArchive>(archive.first, get,
                                                             put, mkdir);
                    }
                }
            }
            else if (item.first == "DATABASE")
                DATABASE = item.second->as<std::string>()->value();
            else
            {
                std::string err("Unknown configuration entry: '");
                err += item.first;
                err += "'";
                throw std::invalid_argument(err);
            }
        }
    }
    catch (cpptoml::toml_parse_exception& ex)
    {
        std::string err("Failed to parse '");
        err += filename;
        err += "' :";
        err += ex.what();
        throw std::invalid_argument(err);
    }
}
}
