#pragma once

#include "Gtest.hpp"

#include <MemUtil/STL_Imports/STD_array.hpp>
#include <MemUtil/STL_Imports/STD_bitset.hpp>

#if _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4625 )
#pragma warning( disable : 4626 )
#endif

template <int Size>
class TestClass
{
public:
    TestClass()
    {
        for( auto& value : m_values )
        {
            value = std::byte{ 1 };
        }
    }

    ~TestClass()
    {
        for( auto& value : m_values )
        {
            value = std::byte{ 0 };
        }
    }

protected:
private:
    std::array<std::byte, Size> m_values;
};

namespace MU
{
class Memutil;
}

class MemoryPoolTests: public ::testing::Test
{
protected:
    MemoryPoolTests();

    virtual ~MemoryPoolTests()
    {
    }

    virtual void SetUp();
    virtual void TearDown()
    {
    }

    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::uint64_t getRandom( std::uint64_t from, std::uint64_t to );
    static MU::Memutil* MemUtil;
    static std::size_t m_samplesCount;
    static std::vector<std::uint64_t> m_sampels;
    static std::vector<void*> m_ptrs;

    static float m_trackedTimeNewDelete;
    static float m_untrackedTimeNewDelete;

    static float m_trackedNew;
    static float m_untrackedNew;

    static float m_trackedDelete;
    static float m_untrackedDelete;

private:
};

#ifdef _MSC_VER
#pragma warning( pop )
#endif