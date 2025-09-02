#pragma once

#include <MemUtil/IStack.hpp>

#if defined( MU_LINUX )

struct backtrace_state;

namespace MU
{
class CStackLinux final: public CIStack
{
public:
    CStackLinux();
    CStackLinux( const CStackLinux& arg );
    CStackLinux( CStackLinux&& arg );

    CStackLinux& operator=( const CStackLinux& arg );
    CStackLinux& operator=( CStackLinux&& arg );

    void fetch() override;
    void decode() override;
    const StackContents& getStackLines() const override;
    bool operator==( const CStackLinux& arg ) const;

    ~CStackLinux();

protected:
private:
    StackContents m_stackFrames;
    std::array<void*, G_MaxStackSize> m_data;

    static backtrace_state* getState();
    static backtrace_state* s_backTrace;
};

using CStack = CStackLinux;

}  // namespace MU

#endif  // #if defined( MU_LINUX )