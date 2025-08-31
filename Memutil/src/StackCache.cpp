#include "StackCache.hpp"
#include <MemUtil/IStack.hpp>
#include <MemUtil/Memutil.hpp>
#include <MemUtil/Import_tracy.hpp>
#include <MemUtil/STL_Imports/STD_mutex.hpp>
#include <MemUtil/STL_Imports/STD_unordered_map.hpp>

using namespace MU;

std::mutex g_mtx;
std::unordered_map<void*, SLineInfo> g_stackLineCache;

CStackCache& CStackCache::getInstance()
{
    static CStackCache s_instance;
    return s_instance;
}

CStackCache::CStackCache()
{
    MU_MEASURE_SCOPE;
    // TODO: check some average values.
    //Memutil::getInstance().executeWithoutAllocationLog(
    //    [this]()
    //    {
    //        g_stackLineCache.reserve( 2048u );
    //    } );
}

SLineInfo* CStackCache::get( void* inPtr )
{
    MU_MEASURE_SCOPE;
    std::lock_guard<std::mutex> locker( g_mtx );
    const auto it = g_stackLineCache.find( inPtr );
    if( it != g_stackLineCache.end() )
    {
        return &it->second;
    }
    return nullptr;
}

void CStackCache::add( SLineInfo* inInfo, void* inPtr )
{
    MU_MEASURE_SCOPE;
    std::lock_guard<std::mutex> locker( g_mtx );
    Memutil::getInstance().beginUntracked();
    g_stackLineCache[inPtr] = *inInfo;
    Memutil::getInstance().endUntracked();
}

void CStackCache::remove( void* inPtr )
{
    MU_MEASURE_SCOPE;
}

CStackCache::~CStackCache()
{
}