#pragma once

#include <MemUtil/STL_Imports/STD_cstdint.hpp>
#include <MemUtil/Generic/Import_boost_assert.hpp>
#include <MemUtil/STL_Imports/STD_cstring.hpp>

namespace MU
{

template <std::uint16_t Capacity>
class StringStatic
{
public:
    StringStatic()
    {
    }

    StringStatic( const char* arg ):
        m_size( std::strlen( arg ) )
    {
        BOOST_ASSERT_MSG( m_size < Capacity, "TOO SMALL BUFFER!" );
        std::strcpy( m_value, arg );
    }

    StringStatic( const StringStatic& arg ):
        m_size( arg.m_size )
    {
        BOOST_ASSERT_MSG( m_size < Capacity, "TOO SMALL BUFFER!" );
        std::strcpy( m_value, arg.m_value );
    }

    StringStatic( StringStatic&& arg ):
        m_size( arg.size )
    {
        std::strcpy( m_value, arg.m_value );
        arg.m_size = 0u;
    }

    StringStatic& operator=( const StringStatic& arg )
    {
        if( this != &arg )
        {
            std::strcpy( m_value, arg.m_value );
            m_size = arg.m_size;
        }
        return *this;
    }

    StringStatic& operator=( StringStatic&& arg )
    {
        if( this != &arg )
        {
            std::strcpy( m_value, arg.m_value );
            m_size = arg.m_size;

            arg.m_size = 0u;
        }
        return *this;
    }

    StringStatic& operator=( const char* arg )
    {
        m_size = std::strlen( arg );
        BOOST_ASSERT_MSG( m_size < Capacity, "TOO SMALL BUFFER!" );

        std::strcpy( m_value, arg );

        return *this;
    }

    template <std::uint16_t CapacityArg>
    bool operator==(const StringStatic<CapacityArg>& arg) const
    {
        if constexpr( CapacityArg != Capacity )
        {
            return false;
        }

        return operator==( arg.m_value );
    }

    bool operator==( const char* arg ) const
    {
        return std::strcmp( m_value, arg ) == 0;
    }

    bool operator<(const StringStatic& arg) const
    {
        return std::strcmp( m_value, arg.m_value ) < 0;
    }

    void append( const char* inStr )
    {
        const std::uint16_t stringLength = static_cast<std::uint16_t>( std::strlen( inStr ) );
        append( inStr, stringLength );
    }

    void append( const char* inStr, std::uint16_t inSize )
    {
        if( m_size + inSize + 1u >= Capacity )
        {
            BOOST_ASSERT_MSG( false, "NOT ENOUGHT PLACE FOR THIS STRING!" );
            return;
        }

        std::strncpy( m_value + m_size, inStr, inSize );
        m_size += inSize;
        m_value[m_size] = '\n';
        m_value[m_size + 1] = '\0';
        m_size += 1;
    }

    void print() const
    {
    }

    const char* c_str() const
    {
        return m_value;
    }

    ~StringStatic()
    {
    }

protected:
private:
    char m_value[Capacity]{};
    std::uint16_t m_size{ 0u };
};
}  // namespace MU