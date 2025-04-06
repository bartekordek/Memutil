#pragma once

namespace MU
{
class IStackMgr
{
public:
    static IStackMgr& getInstance();

    virtual void getStackHere() = 0;


    IStackMgr( const IStackMgr& ) = delete;
    IStackMgr( IStackMgr&& ) = delete;
    IStackMgr& operator=( const IStackMgr& ) = delete;
    IStackMgr& operator=( IStackMgr&& ) = delete;

protected:
    IStackMgr();
    virtual ~IStackMgr();

private:

};
}  // namespace MU
