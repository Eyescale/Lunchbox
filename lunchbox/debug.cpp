
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *               2009-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "debug.h"

#include "os.h" // must come before atomic.h, ordering issue
#include "atomic.h"

#include <errno.h>

#define LB_BACKTRACE_DEPTH 256
#ifdef _WIN32
#  include "scopedMutex.h"
#  define LB_SYMBOL_LENGTH 256
#  include <DbgHelp.h>
#  pragma comment(lib, "DbgHelp.lib")
#else
#  include <cxxabi.h>
#  include <execinfo.h>
#  include <stdlib.h>
#  include <string.h>
#endif

namespace lunchbox
{

void abort()
{
    LBERROR << "  in: " << backtrace << std::endl;

    // if LB_ABORT_WAIT is set, spin forever to allow identifying and debugging
    // crashed nodes.
    if( getenv( "LB_ABORT_WAIT" ))
        while( true ) ;

    ::abort();
}

void checkHeap()
{
#ifdef _MSC_VER
    static a_int32_t count( 0 );
    if( ( ++count % 10000 ) == 0 && _heapchk() != _HEAPOK )
    {
        LBERROR << disableFlush << "Abort: heap corruption detected"<< std::endl
                << "    Set breakpoint in " << __FILE__ << ':' << __LINE__ + 1
                << " to debug" << std::endl << enableFlush;
    }
#else
#endif
}

namespace
{
static void backtrace_( std::ostream& os, const size_t ignoreHead )
{
#ifdef _WIN32
    // Sym* functions from DbgHelp are not thread-safe...
    static Lock lock;
    ScopedMutex<> mutex( lock );

    typedef USHORT (WINAPI *CaptureStackBackTraceType)( __in ULONG, __in ULONG,
                                                        __out PVOID*,
                                                        __out_opt PULONG );
    CaptureStackBackTraceType backtraceFunc = (CaptureStackBackTraceType)
       GetProcAddress(LoadLibrary("kernel32.dll"), "RtlCaptureStackBackTrace");
    if( !backtraceFunc )
        return os;

    SymSetOptions( SYMOPT_UNDNAME | SYMOPT_LOAD_LINES );
    HANDLE hProcess = GetCurrentProcess();
    if( !SymInitialize( hProcess, 0, TRUE ))
        return os;

    void* stack[ LB_BACKTRACE_DEPTH ];
    unsigned short frames = (backtraceFunc)( 0, LB_BACKTRACE_DEPTH, stack, 0 );

    SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc( sizeof(SYMBOL_INFO) +
                                        (LB_SYMBOL_LENGTH+-1)*sizeof(char), 1 );
    symbol->MaxNameLen   = LB_SYMBOL_LENGTH;
    symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

    for( unsigned short i = ignoreHead; i < frames; ++i )
    {
        os << "\n  " << frames-i-1 << ": ";
        if ( !SymFromAddr( hProcess, (DWORD64)stack[i], 0, symbol ))
            os << "Unknown symbol";
        else
        {
            os << symbol->Name << " - ";
            IMAGEHLP_LINE64 line = { sizeof(IMAGEHLP_LINE64) };
            if( !SymGetLineFromAddr64( hProcess, (DWORD64)stack[i], 0, &line ))
                os << std::hex << "0x" << symbol->Address << std::dec;
            else
                os << line.FileName << ":" << line.LineNumber;
        }
    }
    os << std::endl;
    free( symbol );
    SymCleanup( hProcess );
#else
    void* callstack[ LB_BACKTRACE_DEPTH ];
    const int frames = ::backtrace( callstack, LB_BACKTRACE_DEPTH );
    char** names = ::backtrace_symbols( callstack, frames );
    for( int i = ignoreHead + 1; i < frames; ++i )
    {
        std::string name = names[ i ];
#  ifdef __linux__
        const size_t symbolPos = name.find( "(_" );
#  else
        const size_t symbolPos = name.find( " _" );
#  endif
        if( symbolPos != std::string::npos )
            name = name.substr( symbolPos+1, name.length( ));

#  ifdef __linux__
        const size_t spacePos = name.find( '+' );
#  else
        const size_t spacePos = name.find( ' ' );
#  endif
        if( spacePos != std::string::npos )
            name = name.substr( 0, spacePos );

        int status;
        char* demangled = abi::__cxa_demangle( name.c_str(), 0, 0, &status);

        os << "\n  " << frames-i-1 << ": ";
        if( symbolPos == std::string::npos || spacePos == std::string::npos )
            os << names[ i ];
        else
        {
            if( demangled )
            {
                os << demangled;
                free( demangled );
            }
            else
                os << name;
        }
    }
    os << std::endl;
    ::free( names );
#endif
}
}

std::string backtrace( const size_t ignoreHead )
{
    std::ostringstream os;
    backtrace_( os, ignoreHead + 1/*cut self*/ );
    return os.str();
}

std::ostream& backtrace( std::ostream& os )
{
    backtrace_( os, 1 /*cut self*/ );
    return os;
}

std::string demangleTypeID( const char* mangled )
{
#ifdef _WIN32
    return std::string( mangled );
#else
    int status;
    char* name = abi::__cxa_demangle( mangled, 0, 0, &status );
    const std::string result = name;
    if( name )
        free( name );

    return (status==0) ? result : mangled;
#endif
}

std::ostream& sysError( std::ostream& os )
{
#ifdef _WIN32
    const DWORD error = GetLastError();
    char text[512] = "";
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, text, 511, 0 );
    const size_t length = strlen( text );
    if( length>2 && text[length-2] == '\r' )
        text[length-2] = '\0';

    return os << text << " (" << error << ")";
#else
    return os << strerror( errno ) << " (" << errno << ")";
#endif
}

std::string sysError()
{
    std::ostringstream os;
    os << sysError;
    return os.str();
}

}
