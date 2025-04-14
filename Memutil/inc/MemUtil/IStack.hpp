#pragma once

#include <MemUtil/Config.hpp>
#include <MemUtil/Generic/StringStatic.hpp>
#include <MemUtil/STL_Imports/STD_array.hpp>


namespace MU
{
struct SLineInfo
{
    StringStatic<256> Value;
    std::uint16_t Number;
    void* Addr{ nullptr };

    bool operator==( const SLineInfo& arg ) const
    {
        return Value == arg.Value;
    }
};

class CIStack
{
public:
    void* Data{ nullptr };
    std::uint64_t Size{ 0u };


    CIStack();

    virtual void fetch() = 0;
    virtual void decode() = 0;
    virtual const std::array<SLineInfo, G_MaxStackSize>& getStackLines() const = 0;

    virtual ~CIStack();

protected:
private:
};
}  // namespace MU