
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *               2011-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <pthread.h>

#include "log.h"

#include "clock.h"
#include "perThread.h"
#include "scopedMutex.h"
#include "thread.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#ifdef WIN32_API
#  include <process.h>
#endif

#ifdef _MSC_VER
#  define atoll _atoi64
#  define snprintf _snprintf
#  define getpid _getpid
#else
#  include <unistd.h>
#endif

namespace lunchbox
{
static unsigned getLogTopics();

namespace
{
class LogTable
{
public:
    LogTable( const LogLevel _level, const std::string& _name )
            : level( _level ), name( _name ) {}

    LogLevel level;
    std::string name;
};

#define LOG_TABLE_ENTRY( name ) LogTable( LOG_ ## name, std::string( #name ))
#define LOG_TABLE_SIZE (5)

static LogTable _logTable[ LOG_TABLE_SIZE ] =
{
    LOG_TABLE_ENTRY( ERROR ),
    LOG_TABLE_ENTRY( WARN ),
    LOG_TABLE_ENTRY( INFO ),
    LOG_TABLE_ENTRY( VERB ),
    LOG_TABLE_ENTRY( ALL )
};
}

int      Log::level  = Log::getLogLevel( getenv( "LB_LOG_LEVEL" ));
unsigned Log::topics = getLogTopics();
Clock    _defaultClock;
Clock*   _clock = &_defaultClock;
Lock     LogBuffer::_lock;

static PerThread< Log > _logInstance;

#ifdef NDEBUG
static std::ostream* _logStream = &std::cout;
#else
static std::ostream* _logStream = &std::cerr;
#endif
static std::ostream* _logFile = 0;

int Log::getLogLevel( const char* text )
{
    if( text )
    {
        const int num = atoi( text );
        if( num > 0 && num <= LOG_ALL )
            return num;

        for( uint32_t i = 0; i < LOG_TABLE_SIZE; ++i )
            if( _logTable[i].name == text )
                return _logTable[i].level;
    }

#ifdef NDEBUG
    return LOG_WARN;
#else
    return LOG_INFO;
#endif
}

std::string& Log::getLogLevelString()
{
    for( uint32_t i=0; i<LOG_TABLE_SIZE; ++i )
        if( _logTable[i].level == level )
                return _logTable[i].name;

    return _logTable[0].name;
}

unsigned getLogTopics()
{
    Log::level = Log::getLogLevel( getenv( "LB_LOG_LEVEL" ));
    const char *env = getenv( "LB_LOG_TOPICS" );

    if( env )
        return atoll(env);

    if( Log::level == LOG_ALL )
        return 0xffffffffu;

    return 0;
}

Log& Log::instance()
{
    Log* log = _logInstance.get();
    if( !log )
    {
        log = new Log();
        _logInstance = log;
    }

    return *log;
}

Log& Log::instance( const char* file, const int line )
{
    Log& log = instance();
    log.setLogInfo( file, line );
    return log;
}

void Log::exit()
{
    Log* log = _logInstance.get();
    _logInstance = 0;
    delete log;
}

void Log::reset()
{
    exit();

    delete _logFile;
    _logFile = 0;

#ifdef NDEBUG
    _logStream = &std::cout;
#else
    _logStream = &std::cerr;
#endif
}

void Log::setOutput( std::ostream& stream )
{
    _logStream = &stream;
    exit();
}

bool Log::setOutput( const std::string& file )
{
    std::ostream* oldLog = _logStream;
    std::ofstream* newLog = new std::ofstream( file.c_str( ));

    if( newLog->is_open( ))
    {
        setOutput( *newLog );
        *oldLog << "Redirected log to " << file << std::endl;

        delete _logFile;
        _logFile = newLog;
        return true;
    }

    LBWARN << "Can't open log file " << file << ": " << sysError << std::endl;
    delete newLog;
    return false;
}

void Log::setClock( Clock* clock )
{
    if( clock )
        _clock = clock;
    else
        _clock = &_defaultClock;
}

const Clock& Log::getClock()
{
    return *_clock;
}

std::ostream& Log::getOutput()
{
    return *_logStream;
}


void LogBuffer::setThreadName( const std::string& name )
{
    LBASSERT( !name.empty( ));
    snprintf( _thread, 12, "%s", name.c_str( ));
    _thread[11] = 0;
}

LogBuffer::int_type LogBuffer::overflow( LogBuffer::int_type c )
{
    if( c == EOF )
        return EOF;

    if( _newLine )
    {
        if( !_noHeader )
        {
            //assert( _thread[0] );
            _stringStream << getpid()  << " " << _thread << " " << _file << ":"
                          << _line << " " << _clock->getTime64() << " ";
        }

        for( int i=0; i<_indent; ++i )
            _stringStream << "    ";
        _newLine = false;
    }

    _stringStream << (char)c;
    return c;
}

int LogBuffer::sync()
{
    if( !_blocked )
    {
        const std::string& string = _stringStream.str();
        {
            ScopedMutex< Lock > mutex( _lock );
            _stream.write( string.c_str(), string.length( ));
            _stream.rdbuf()->pubsync();
        }
        _stringStream.str( "" );
    }
    _newLine = true;
    return 0;
}


std::ostream& indent( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->indent();
    return os;
}
std::ostream& exdent( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->exdent();
        return os;
}

std::ostream& disableFlush( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->disableFlush();
    return os;
}
std::ostream& enableFlush( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->enableFlush();
    return os;
}
std::ostream& forceFlush( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->forceFlush();
    return os;
}

std::ostream& disableHeader( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->disableHeader();
    return os;
}
std::ostream& enableHeader( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->enableHeader();
    return os;
}

}
