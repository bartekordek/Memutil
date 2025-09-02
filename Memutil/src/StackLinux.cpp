#include <MemUtil/StackLinux.hpp>

#if defined( MU_LINUX )
    #include <MemUtil/STL_Imports/STD_iostream.hpp>
    #include <MemUtil/STL_Imports/STD_array.hpp>
    #include "Import_boost_stacktrace.hpp"
    #include <unwind.h>
    #include <backtrace.h>
    #include <cxxabi.h>
using native_frame_ptr_t = void*;

struct unwind_state
{
    std::size_t frames_to_skip;
    native_frame_ptr_t* current;
    native_frame_ptr_t* end;
};

inline _Unwind_Reason_Code unwind_callback( ::_Unwind_Context* context, void* arg )
{
    // Note: do not write `::_Unwind_GetIP` because it is a macro on some platforms.
    // Use `_Unwind_GetIP` instead!
    unwind_state* const state = static_cast<unwind_state*>( arg );
    if( state->frames_to_skip )
    {
        --state->frames_to_skip;
        return _Unwind_GetIP( context ) ? ::_URC_NO_REASON : ::_URC_END_OF_STACK;
    }

    *state->current = reinterpret_cast<native_frame_ptr_t>( _Unwind_GetIP( context ) );

    ++state->current;
    if( !*( state->current - 1 ) || state->current == state->end )
    {
        return ::_URC_END_OF_STACK;
    }
    return ::_URC_NO_REASON;
}

namespace MU
{

inline int libbacktrace_full_callback( void* data, uintptr_t /*pc*/, const char* filename, int lineno, const char* function )
{
    SLineInfo* line = static_cast<SLineInfo*>( data );
    if( filename )
    {
        int status = 0;
        const char* demangled = abi::__cxa_demangle( function, nullptr, nullptr, &status );
        line->FunctionName = demangled == nullptr ? function : demangled;
        line->FileName = filename;
        line->Number = lineno;
    }
    return 0;
}

inline void libbacktrace_error_callback( void* /*data*/, const char* /*msg*/, int /*errnum*/ ) noexcept
{
    // Do nothing, just return.
}

backtrace_state* CStackLinux::s_backTrace{ nullptr };

CStackLinux::CStackLinux():
    CIStack()
{
}

CStackLinux::CStackLinux( const CStackLinux& arg ):
    CIStack( arg )
{
    m_stackFrames = arg.m_stackFrames;
    m_data = arg.m_data;
}

CStackLinux::CStackLinux( CStackLinux&& arg ):
    CIStack( arg )
{
    m_stackFrames = std::move( arg.m_stackFrames );
    m_data = std::move( arg.m_data );
}

CStackLinux& CStackLinux::operator=( const CStackLinux& arg )
{
    if( this != &arg )
    {
        CIStack::operator=( arg );
        m_stackFrames = arg.m_stackFrames;
        m_data = arg.m_data;
    }

    return *this;
}

CStackLinux& CStackLinux::operator=( CStackLinux&& arg )
{
    if( this != &arg )
    {
        CIStack::operator=( arg );
        m_stackFrames = std::move( arg.m_stackFrames );
        m_data = std::move( arg.m_data );
    }

    return *this;
}

void CStackLinux::fetch()
{
    std::size_t skip = 1u;

    native_frame_ptr_t* out_frames = m_data.data();
    unwind_state state = { skip, out_frames, out_frames + G_MaxStackSize };

    ::_Unwind_Backtrace( &unwind_callback, &state );
}

backtrace_state* CStackLinux::getState()
{
    if( s_backTrace == nullptr )
    {
        s_backTrace = backtrace_create_state( nullptr, 0, nullptr, nullptr );
    }
    return s_backTrace;
}

void CStackLinux::decode()
{
    for( std::size_t i = 0u; i < G_MaxStackSize; ++i )
    {
        void* add = m_data[i];
        ::backtrace_pcinfo( getState(), reinterpret_cast<uintptr_t>( add ), libbacktrace_full_callback, libbacktrace_error_callback,
                            &m_stackFrames[i] );
    }
}

bool CStackLinux::operator==( const CStackLinux& arg ) const
{
    return m_stackFrames == arg.m_stackFrames;
}

const StackContents& CStackLinux::getStackLines() const
{
    return m_stackFrames;
}

CStackLinux::~CStackLinux()
{
}

}  // namespace MU

#endif  // #if defined( MU_LINUX )
