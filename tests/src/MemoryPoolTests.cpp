#include "MemoryPoolTests.hpp"
#include <MemUtil/Memutil.hpp>
#include <MemUtil/STL_Imports/STD_chrono.hpp>
#include <MemUtil/STL_Imports/STD_random.hpp>
#include <MemUtil/STL_Imports/STD_vector.hpp>

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

void operator delete( void* ptr, std::size_t /*size*/ ) noexcept
{
    MU::Memutil::getInstance().logFree( ptr );
    std::free( ptr );
}

void operator delete[]( void* ptr ) noexcept
{
    MU::Memutil::getInstance().logFree( ptr );
    std::free( ptr );
}

void operator delete[]( void* ptr, std::size_t /*size*/ ) noexcept
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
    instance.dumpActiveAllocationsToOutput();

    delete tc;
    instance.toggleTracking( false );
}

TEST_F( MemoryPoolTests, TwoAllocations )
{
    std::size_t allocCount{ 2u };

    std::vector<TestClass<8>*> pointers;
    pointers.resize( allocCount );

    MU::Memutil& instance = MU::Memutil::getInstance();
    instance.toggleTracking( true );

    for( std::size_t i = 0u; i < allocCount; ++i )
    {
        pointers[i] = new TestClass<8>();
    }

    instance.waitForAllCallStacksToBeDecoded();
    instance.dumpActiveAllocationsToOutput();

    for( std::size_t i = 0u; i < allocCount; ++i )
    {
        delete pointers[i];
    }
    instance.toggleTracking( false );
}

TEST_F( MemoryPoolTests, TEST_SINGLE_ALLOCATION_TO_CHAR_BUF )
{
    constexpr std::size_t buffSize{ 2048 };
    char* charOutput = new char[buffSize];

    MU::Memutil& instance = MU::Memutil::getInstance();
    instance.toggleTracking( true );

    TestClass<8>* tc = new TestClass<8>();
    instance.waitForAllCallStacksToBeDecoded();
    instance.dumpActiveAllocationsToBuffer( charOutput, buffSize );

    printf( "char buff:\n%s\n", charOutput );

    delete tc;
    instance.toggleTracking( false );

    delete[] charOutput;
}

TEST_F( MemoryPoolTests, SpeedBenchmark )
{
    const std::size_t testSampleCount{ 8000000 };
    const std::size_t maxAllocationBlock{ 16384 };

    std::vector<std::uint64_t> samples;
    samples.resize( testSampleCount );

    for( std::size_t i = 0; i < testSampleCount; ++i )
    {
        const std::uint64_t currentSize = getRandom( 2, maxAllocationBlock );
        samples[i] = currentSize;
    }

    auto runAllocations = [&samples]()
    {
        auto start = std::chrono::steady_clock::now();
        for( std::size_t i = 0; i < testSampleCount; ++i )
        {
            std::byte* allocatedMemory = new std::byte[samples[i]];
            delete[] allocatedMemory;
        }
        auto elapsed = std::chrono::steady_clock::now() - start;
        std::uint64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>( elapsed ).count();
        return static_cast<float>( milliseconds );
    };

    const float without = runAllocations();

    MU::Memutil& instance = MU::Memutil::getInstance();

    instance.toggleTracking( true );
    const float with = runAllocations();
    instance.toggleTracking( false );

    printf( "%18s %4.0f ms\n", "Without tracking: ", without );
    printf( "%18s %4.0f ms\n", "With tracking: ", with );
    printf( "Tracked is %2.0f%% longer than untracked.\n", ( 100.0 * with / without ) - 100.0f );
}



std::uint64_t MemoryPoolTests::getRandom( std::uint64_t from, std::uint64_t to )
{
    static std::random_device rand_dev;
    static std::mt19937 generator( rand_dev() );
    std::uniform_int_distribution<std::uint64_t> distr( from, to );
    return distr( generator );
}