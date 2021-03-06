#pragma once

// Copyright 2015 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "history/HistoryManager.h"
#include "history/CatchupStateMachine.h"
#include "history/PublishStateMachine.h"
#include <memory>

namespace medida
{
class Meter;
}

namespace stellar
{

class Application;

class HistoryManagerImpl : public HistoryManager
{
    Application& mApp;
    std::unique_ptr<TmpDir> mWorkDir;
    std::unique_ptr<PublishStateMachine> mPublish;
    std::unique_ptr<CatchupStateMachine> mCatchup;

    medida::Meter& mPublishSkip;
    medida::Meter& mPublishQueue;
    medida::Meter& mPublishDelay;
    medida::Meter& mPublishStart;
    medida::Meter& mPublishSuccess;
    medida::Meter& mPublishFailure;

    medida::Meter& mCatchupStart;
    medida::Meter& mCatchupSuccess;
    medida::Meter& mCatchupFailure;
public:
    HistoryManagerImpl(Application& app);
    ~HistoryManagerImpl() override;

    void verifyHash(std::string const& filename, uint256 const& hash,
                    std::function<void(asio::error_code const&)> handler) const override;

    void decompress(std::string const& filename_gz,
                    std::function<void(asio::error_code const&)> handler,
                    bool keepExisting = false) const override;

    void compress(std::string const& filename_nogz,
                  std::function<void(asio::error_code const&)> handler,
                  bool keepExisting = false) const override;

    void putFile(std::shared_ptr<HistoryArchive const> archive,
                 std::string const& local, std::string const& remote,
                 std::function<void(asio::error_code const&)> handler) const override;

    void getFile(std::shared_ptr<HistoryArchive const> archive,
                 std::string const& remote, std::string const& local,
                 std::function<void(asio::error_code const&)> handler) const override;

    void mkdir(std::shared_ptr<HistoryArchive const> archive,
               std::string const& dir,
               std::function<void(asio::error_code const&)> handler) const override;

    bool
    maybePublishHistory(std::function<void(asio::error_code const&)> handler) override;

    bool hasAnyWritableHistoryArchive() override;

    void publishHistory(std::function<void(asio::error_code const&)> handler) override;

    void catchupHistory(
        uint32_t initLedger, ResumeMode mode,
        std::function<void(asio::error_code const& ec, ResumeMode mode,
                           LedgerHeaderHistoryEntry const& lastClosed)>
            handler) override;

    void snapshotWritten(asio::error_code const&) override;

    HistoryArchiveState getLastClosedHistoryArchiveState() const override;

    std::string const& getTmpDir() override;

    std::string localFilename(std::string const& basename) override;

    uint64_t getPublishSkipCount() override;
    uint64_t getPublishQueueCount() override;
    uint64_t getPublishDelayCount() override;
    uint64_t getPublishStartCount() override;
    uint64_t getPublishSuccessCount() override;
    uint64_t getPublishFailureCount() override;

    uint64_t getCatchupStartCount() override;
    uint64_t getCatchupSuccessCount() override;
    uint64_t getCatchupFailureCount() override;

};

}
