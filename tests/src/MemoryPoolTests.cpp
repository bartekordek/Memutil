#include "MemoryPoolTests.hpp"
#include <MemUtil/Memutil.hpp>

void* operator new( std::size_t size )
{
    void* result = std::malloc( size );
    MU::Memutil::getInstance().logAlloc( result, size );
    return result;
}

void* operator new[]( std::size_t size )
{
    void* result = std::malloc( size );
    MU::Memutil::getInstance().logAlloc( result, size );
    return result;
}

void operator delete( void* ptr ) noexcept
{
    MU::Memutil::getInstance().logFree( ptr );
    std::free( ptr );
}

void operator delete( void* ptr, std::size_t size ) noexcept
{
    MU::Memutil::getInstance().logFree( ptr );
    std::free( ptr );
}

void operator delete[]( void* ptr ) noexcept
{
    MU::Memutil::getInstance().logFree( ptr );
    std::free( ptr );
}

void operator delete[]( void* ptr, std::size_t size ) noexcept
{
    MU::Memutil::getInstance().logFree( ptr );
    std::free( ptr );
}

MemoryPoolTests::MemoryPoolTests()
{
}

TEST_F( MemoryPoolTests, TEST_SINGLE_ALLOCATION )
{
    MU::Memutil& instance = MU::Memutil::getInstance();
    instance.toggleTracking( true );

    TestClass<8>* tc = new TestClass<8>();
    instance.waitForAllCallStacksToBeDecoded();
    instance.dumpActiveAllocations();


    delete tc;
    instance.toggleTracking( false );
}

TEST_F( MemoryPoolTests, TEST_MULTIPLE_ALLOCATION )
{
    //TODO:
}

TEST_F( MemoryPoolTests, TESTING_0 )
{
}

TEST_F( MemoryPoolTests, TESTING_1 )
{
}