
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifdef _MSC_VER
#  include <process.h>
#  define atoll _atoi64
#  define snprintf _snprintf
#  define getpid _getpid
#else
#  include <unistd.h>
#endif

namespace lunchbox
{
static unsigned getLogTopics();
static Clock    _defaultClock;
static Clock*   _clock = &_defaultClock;
static Lock     _lock; // The write lock

namespace detail
{
/** @internal The string buffer used for logging. */
class Log : public std::streambuf
{
public:
    explicit Log( std::ostream& stream )
        : _indent(0), _blocked(0), _noHeader(0),
          _newLine(true), _stream(stream)
    { _file[0] = 0; }

    virtual ~Log() {}

    void indent() { ++_indent; }
    void exdent() { --_indent; }

    void disableFlush() { ++_blocked; assert( _blocked < 100 ); }
    void enableFlush()
    {
        assert( _blocked );// Too many enableFlush on log stream
        --_blocked;
    }

    void disableHeader() { ++_noHeader; } // use counted variable to allow
    void enableHeader()  { --_noHeader; } //   nested enable/disable calls

    void setThreadName( const std::string& name )
    {
        LBASSERT( !name.empty( ));
        _thread = name;
    }

    const std::string& getThreadName() const { return _thread; }

    void setLogInfo( const char* f, const int line )
    {
        LBASSERT( f );
        std::string file( f );
        const size_t length = file.length();

        if( length > 29 )
            file = file.substr( length - 29, length );

        snprintf( _file, 35, "%29s:%-4d", file.c_str(), line );
    }

protected:
    virtual int_type overflow( Log::int_type c ) override
    {
        if( c == EOF )
            return EOF;

        if( _newLine )
        {
            if( !_noHeader )
            {
                _stringStream << getpid()  << "." << _thread << " " << _file
                              << " " << _clock->getTime64() << " ";
            }

            for( int i=0; i<_indent; ++i )
                _stringStream << "    ";
            _newLine = false;
        }

        _stringStream << (char)c;
        return c;
    }

    virtual int sync() override
    {
        if( !_blocked )
        {
            const std::string& string = _stringStream.str();
            {
                ScopedMutex< lunchbox::Lock > mutex( _lock );
                _stream.write( string.c_str(), string.length( ));
                _stream.rdbuf()->pubsync();
            }
            _stringStream.str( "" );
        }
        _newLine = true;
        return 0;
    }

private:
    Log( const Log& );
    Log& operator = ( const Log& );

    /** Short thread name. */
    std::string _thread;

    /** The current file logging. */
    char _file[35];

    /** The current indentation level. */
    int _indent;

    /** Flush reference counter. */
    int _blocked;

    /** The header disable counter. */
    int _noHeader;

    /** The flag that a new line has started. */
    bool _newLine;

    /** The temporary buffer. */
    std::ostringstream _stringStream;

    /** The wrapped ostream. */
    std::ostream& _stream;
};
}

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

static PerThread< Log > _logInstance;

#ifdef NDEBUG
static std::ostream* _logStream = &std::cout;
#else
static std::ostream* _logStream = &std::cerr;
#endif
static std::ostream* _logFile = 0;

Log::Log()
    : std::ostream( new detail::Log( getOutput( )))
    , impl_( dynamic_cast< detail::Log* >( rdbuf( )))
{}

Log::~Log()
{
    impl_->pubsync();
    delete impl_;
}

void Log::indent()
{
    impl_->indent();
}

void Log::exdent()
{
    impl_->exdent();
}

void Log::disableFlush()
{
    impl_->disableFlush();
}

void Log::enableFlush()
{
    impl_->enableFlush();
}

void Log::forceFlush()
{
    impl_->pubsync();
}

void Log::disableHeader()
{
    impl_->disableHeader();
}

void Log::enableHeader()
{
    impl_->enableHeader();
}

void Log::setLogInfo( const char* file, const int line )
{
    impl_->setLogInfo( file, line );
}

void Log::setThreadName( const std::string& name )
{
    impl_->setThreadName( name );
}

const std::string& Log::getThreadName() const
{
    return impl_->getThreadName();
}

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
