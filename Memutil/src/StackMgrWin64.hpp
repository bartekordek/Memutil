#pragma once

#include <MemUtil/IStackMgr.hpp>

namespace MU
{
class StackMgrWin64 final: public IStackMgr
{
public:
    StackMgrWin64();

    ~StackMgrWin64();


    StackMgrWin64( const StackMgrWin64& ) = delete;
    StackMgrWin64( StackMgrWin64&& ) = delete;
    StackMgrWin64& operator=( const StackMgrWin64& ) = delete;
    StackMgrWin64& operator=( StackMgrWin64&& ) = delete;

protected:
private:
    void getStackHere() override;
};
}  // namespace MU
