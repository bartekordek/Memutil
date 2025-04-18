#include "MemoryPoolTests.hpp"
#include <MemUtil/Memutil.hpp>
#include <MemUtil/Import_tracy.hpp>
#include <MemUtil/STL_Imports/STD_chrono.hpp>
#include <MemUtil/STL_Imports/STD_random.hpp>
#include <MemUtil/STL_Imports/STD_vector.hpp>

void* operator new( std::size_t size )
{
    MU_MEASURE_SCOPE;
    void* result = std::malloc( size );
    MU::Memutil::getInstance().logAlloc( result, size );
    return result;
}

void* operator new[]( std::size_t size )
{
    MU_MEASURE_SCOPE;
    void* result = std::malloc( size );
    MU::Memutil::getInstance().logAlloc( result, size );
    return result;
}

void operator delete( void* ptr ) noexcept
{
    MU_MEASURE_SCOPE;
    MU::Memutil::getInstance().logFree( ptr );
    std::free( ptr );
}

void operator delete( void* ptr, std::size_t /*size*/ ) noexcept
{
    MU_MEASURE_SCOPE;
    MU::Memutil::getInstance().logFree( ptr );
    std::free( ptr );
}

void operator delete[]( void* ptr ) noexcept
{
    MU_MEASURE_SCOPE;
    MU::Memutil::getInstance().logFree( ptr );
    std::free( ptr );
}

void operator delete[]( void* ptr, std::size_t /*size*/ ) noexcept
{
    MU_MEASURE_SCOPE;
    MU::Memutil::getInstance().logFree( ptr );
    std::free( ptr );
}

MemoryPoolTests::MemoryPoolTests()
{

}

void MemoryPoolTests::SetUp()
{
}

MU::Memutil* MemoryPoolTests::MemUtil{ nullptr };
std::size_t MemoryPoolTests::m_samplesCount{ 0u };

std::vector<std::uint64_t> MemoryPoolTests::m_sampels;
std::vector<void*> MemoryPoolTests::m_ptrs;

float MemoryPoolTests::m_trackedTimeNewDelete{ 0.f };
float MemoryPoolTests::m_untrackedTimeNewDelete{ 0.f };

float MemoryPoolTests::m_trackedNew{ 0.f };
float MemoryPoolTests::m_untrackedNew{ 0.f };

float MemoryPoolTests::m_trackedDelete{ 0.f };
float MemoryPoolTests::m_untrackedDelete{ 0.f };

void MemoryPoolTests::SetUpTestSuite()
{
    MemUtil = &MU::Memutil::getInstance();

    m_samplesCount = 1000000;
    m_sampels.resize( m_samplesCount );
    m_ptrs.resize( m_samplesCount );

    constexpr std::size_t maxAllocationBlock{ 8192 };

    for( std::size_t i = 0; i < m_samplesCount; ++i )
    {
        const std::uint64_t currentSize = getRandom( 2, maxAllocationBlock );
        m_sampels[i] = currentSize;
    }
}

TEST_F( MemoryPoolTests, TEST_SINGLE_ALLOCATION )
{
    MU_MEASURE_SCOPE;
    MemUtil->toggleTracking( true );

    TestClass<8>* tc = new TestClass<8>();
    MemUtil->waitForAllCallStacksToBeDecoded();

    MemUtil->dumpActiveAllocationsToOutput();

    delete tc;
    MemUtil->toggleTracking( false );
}

TEST_F( MemoryPoolTests, TwoAllocations )
{
    MU_MEASURE_SCOPE;
    std::size_t allocCount{ 2u };

    std::vector<TestClass<8>*> pointers;
    pointers.resize( allocCount );

    MemUtil->toggleTracking( true );

    for( std::size_t i = 0u; i < allocCount; ++i )
    {
        pointers[i] = new TestClass<8>();
    }

    MemUtil->waitForAllCallStacksToBeDecoded();
    MemUtil->dumpActiveAllocationsToOutput();

    for( std::size_t i = 0u; i < allocCount; ++i )
    {
        delete pointers[i];
    }
    MemUtil->toggleTracking( false );
}

TEST_F( MemoryPoolTests, TEST_SINGLE_ALLOCATION_TO_CHAR_BUF )
{
    MU_MEASURE_SCOPE;
    constexpr std::size_t buffSize{ 2048 };
    char* charOutput = new char[buffSize];

    MemUtil->toggleTracking( true );

    TestClass<8>* tc = new TestClass<8>();
    MemUtil->waitForAllCallStacksToBeDecoded();
    MemUtil->dumpActiveAllocationsToBuffer( charOutput, buffSize );

    printf( "char buff:\n%s\n", charOutput );

    delete tc;
    MemUtil->toggleTracking( false );

    delete[] charOutput;
}

TEST_F( MemoryPoolTests, SortingValues )
{
    MU_MEASURE_SCOPE;

    std::vector<void*> allocations;
    allocations.reserve( 2048 );

    // Allocate 256 block with 2B.
    MemUtil->toggleTracking( true );
    for( std::size_t i = 0u; i < 256; ++i )
    {
        std::byte* ptr = new std::byte[2];
        allocations.push_back( ptr );
    }

    // Allocate 8 block with 4B.
    for( std::size_t i = 0u; i < 8; ++i )
    {
        std::byte* ptr = new std::byte[4];
        allocations.push_back( ptr );
    }

    // Allocate 4 block with 16B.
    for( std::size_t i = 0u; i < 4; ++i )
    {
        std::byte* ptr = new std::byte[16];
        allocations.push_back( ptr );
    }

    MemUtil->waitForAllCallStacksToBeDecoded();
    MemUtil->dumpActiveAllocationsToOutput();

    for( const auto ptr : allocations )
    {
        delete[] ptr;
    }

    MemUtil->toggleTracking( false );
}

TEST_F( MemoryPoolTests, SpeedBenchmarkNewDeleteTracked )
{
    MU_MEASURE_SCOPE;

    MemUtil->toggleTracking( true );
    auto start = std::chrono::steady_clock::now();
    for( std::size_t i = 0; i < m_samplesCount; ++i )
    {
        MU_MEASURE_SCOPE;
        std::byte* allocatedMemory = new std::byte[m_sampels[i]];
        delete[] allocatedMemory;
    }
    auto elapsed = std::chrono::steady_clock::now() - start;
    m_trackedTimeNewDelete = static_cast<float>( std::chrono::duration_cast<std::chrono::milliseconds>( elapsed ).count() );
    MemUtil->toggleTracking( false );
}

TEST_F( MemoryPoolTests, SpeedBenchmarkNewDeleteUnTracked )
{
    MemUtil->toggleTracking( false );

    MU_MEASURE_SCOPE;

    auto start = std::chrono::steady_clock::now();
    for( std::size_t i = 0; i < m_samplesCount; ++i )
    {
        MU_MEASURE_SCOPE;
        std::byte* allocatedMemory = new std::byte[m_sampels[i]];
        delete[] allocatedMemory;
    }
    auto elapsed = std::chrono::steady_clock::now() - start;
    m_untrackedTimeNewDelete = static_cast<float>( std::chrono::duration_cast<std::chrono::milliseconds>( elapsed ).count() );
}

TEST_F( MemoryPoolTests, SpeedBenchmarkNewTracked )
{
    MU_MEASURE_SCOPE;

    MemUtil->toggleTracking( true );
    auto start = std::chrono::steady_clock::now();
    for( std::size_t i = 0; i < m_samplesCount; ++i )
    {
        MU_MEASURE_SCOPE;
        m_ptrs[i] = new std::byte[m_sampels[i]];
    }
    auto elapsed = std::chrono::steady_clock::now() - start;
    m_trackedNew = static_cast<float>( std::chrono::duration_cast<std::chrono::milliseconds>( elapsed ).count() );

    for( std::size_t i = 0; i < m_samplesCount; ++i )
    {
        delete m_ptrs[i];
    }
    MemUtil->toggleTracking( false );
}

TEST_F( MemoryPoolTests, SpeedBenchmarkNewUntracked )
{
    MemUtil->toggleTracking( false );

    MU_MEASURE_SCOPE;

    auto start = std::chrono::steady_clock::now();
    for( std::size_t i = 0; i < m_samplesCount; ++i )
    {
        MU_MEASURE_SCOPE;
        m_ptrs[i] = new std::byte[m_sampels[i]];
    }
    auto elapsed = std::chrono::steady_clock::now() - start;
    m_untrackedNew = static_cast<float>( std::chrono::duration_cast<std::chrono::milliseconds>( elapsed ).count() );

    for( std::size_t i = 0; i < m_samplesCount; ++i )
    {
        delete m_ptrs[i];
    }
}

TEST_F( MemoryPoolTests, SpeedBenchmarkDeleteTracked )
{
    MU_MEASURE_SCOPE;

    MemUtil->toggleTracking( true );

    for( std::size_t i = 0; i < m_samplesCount; ++i )
    {
        MU_MEASURE_SCOPE;
        m_ptrs[i] = new std::byte[m_sampels[i]];
    }

    auto start = std::chrono::steady_clock::now();
    for( std::size_t i = 0; i < m_samplesCount; ++i )
    {
        delete m_ptrs[i];
    }
    auto elapsed = std::chrono::steady_clock::now() - start;
    m_trackedDelete = static_cast<float>( std::chrono::duration_cast<std::chrono::milliseconds>( elapsed ).count() );
    MemUtil->toggleTracking( false );
}

TEST_F( MemoryPoolTests, SpeedBenchmarkDeleteUntracked )
{
    MemUtil->toggleTracking( false );

    MU_MEASURE_SCOPE;

    for( std::size_t i = 0; i < m_samplesCount; ++i )
    {
        MU_MEASURE_SCOPE;
        m_ptrs[i] = new std::byte[m_sampels[i]];
    }

    auto start = std::chrono::steady_clock::now();
    for( std::size_t i = 0; i < m_samplesCount; ++i )
    {
        delete m_ptrs[i];
    }
    auto elapsed = std::chrono::steady_clock::now() - start;
    m_untrackedDelete = static_cast<float>( std::chrono::duration_cast<std::chrono::milliseconds>( elapsed ).count() );
}

void MemoryPoolTests::TearDownTestSuite()
{
    printf( "%18s %4.0f ms\n", "Without tracking: ", m_untrackedTimeNewDelete );
    printf( "%18s %4.0f ms\n", "With tracking: ", m_trackedTimeNewDelete );
    printf( "Tracked is %2.0f%% longer than untracked.\n\n", ( 100.0 * m_trackedTimeNewDelete / m_untrackedTimeNewDelete ) - 100.0f );

    printf( "%18s %4.0f ms\n", "New Without tracking: ", m_untrackedNew );
    printf( "%18s %4.0f ms\n", "New With tracking: ", m_trackedNew );
    printf( "Tracked is %2.0f%% longer than untracked.\n\n", ( 100.0 * m_trackedNew / m_untrackedNew ) - 100.0f );

    printf( "%18s %4.0f ms\n", "Delete Without tracking: ", m_untrackedDelete );
    printf( "%18s %4.0f ms\n", "Delete With tracking: ", m_trackedDelete );
    printf( "Tracked is %2.0f%% longer than untracked.\n\n", ( 100.0 * m_trackedDelete / m_untrackedDelete ) - 100.0f );
}

std::uint64_t MemoryPoolTests::getRandom( std::uint64_t from, std::uint64_t to )
{
    static std::random_device rand_dev;
    static std::mt19937 generator( rand_dev() );
    std::uniform_int_distribution<std::uint64_t> distr( from, to );
    return distr( generator );
}