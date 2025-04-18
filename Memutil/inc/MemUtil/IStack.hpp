#pragma once

#include <MemUtil/Config.hpp>
#include <MemUtil/Generic/StringStatic.hpp>
#include <MemUtil/STL_Imports/STD_array.hpp>
#include <MemUtil/STL_Imports/STD_mutex.hpp>


namespace MU
{

class SLineInfo final
{
public:
    StringStatic<256> Value;
    std::uint16_t Number;

    SLineInfo();
    SLineInfo( const SLineInfo& arg );
    SLineInfo( SLineInfo&& arg );
    SLineInfo& operator=( const SLineInfo& arg );
    SLineInfo& operator=( SLineInfo&& arg );

    bool operator==( const SLineInfo& arg ) const;

    ~SLineInfo();
};

//using StackContents = StringStatic<2048>;
using StackContents = std::array<SLineInfo, G_MaxStackSize>;

enum class EStackType: std::int8_t
{
    None = -1,
    Alloc,
    Dealloc
};

class CIStack
{
public:
    void* Data{ nullptr };
    std::uint64_t Size{ 0u };
    EStackType Type{ EStackType::None };

    CIStack();
    CIStack( const CIStack& arg );
    CIStack( CIStack&& arg );
    CIStack& operator=( const CIStack& arg );
    CIStack& operator=( CIStack&& arg );

    virtual void fetch() = 0;
    virtual void decode() = 0;
    virtual const StackContents& getStackLines() const = 0;

    virtual ~CIStack();

protected:
    const SLineInfo* getFromCache( void* inPtr ) const;
    void addToCache( void* inPtr, SLineInfo* inLine );

private:
    mutable std::mutex m_cacheMtx;
};
}  // namespace MU