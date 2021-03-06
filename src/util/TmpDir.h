#pragma once

// Copyright 2015 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include <memory>
#include <string>

namespace stellar
{
class TmpDir
{
    std::unique_ptr<std::string> mPath;

  public:
    TmpDir(std::string const& prefix);
    TmpDir(TmpDir&&);
    ~TmpDir();
    std::string const& getName() const;
};

class TmpDirManager
{
    std::string mRoot;
    void clean();

  public:
    TmpDirManager(std::string const& root);
    ~TmpDirManager();
    TmpDir tmpDir(std::string const& prefix);
};
}
