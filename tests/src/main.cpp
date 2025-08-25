#include "Gtest.hpp"
#include <MemUtil/Import_tracy.hpp>
#include <MemUtil/STL_Imports/STD_thread.hpp>
#include <MemUtil/STL_Imports/STD_chrono.hpp>

namespace MU
{
class Memutil;

extern Memutil* g_instance;
}

int main( int argc, char** argv )
{
    MU_SET_THREAD_NAME( "MainLoop" );
    ::testing::InitGoogleTest( &argc, argv );
    const int result = RUN_ALL_TESTS();

    std::this_thread::sleep_for( std::chrono::seconds( 6u ) );
    return result;
}