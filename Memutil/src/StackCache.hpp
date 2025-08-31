#pragma once

#include <MemUtil/Generic/NonCopyable.hpp>

namespace MU
{
class SLineInfo;

class CStackCache
{
public:
    static CStackCache& getInstance();
    SLineInfo* get( void* inPtr );
    void add( SLineInfo* inInfo, void* inPtr );
    void remove( void* inPtr );

protected:
private:
    CStackCache();
    ~CStackCache();

public:
    MU_NONCOPYABLE( CStackCache )
};
}  // namespace MU