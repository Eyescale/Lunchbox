
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Daniel Nachbaur <danielnachbaur@gmail.com>
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

/**
 * @file lunchbox/log.h
 * This file contains logging classes. The macros LBERROR, LBWARN, LBINFO and
 * LBVERB output messages at their respective logging level, if the level is
 * active. They use a per-thread lunchbox::Log instance, which is a
 * std::ostream. LBVERB is always inactive in release builds.
 */

#ifndef LUNCHBOX_LOG_H
#define LUNCHBOX_LOG_H

#include <lunchbox/api.h>
#include <lunchbox/compiler.h>
#include <lunchbox/types.h>

#include <assert.h>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace lunchbox
{
    /** The logging levels. @version 1.0 */
    enum LogLevel
    {
        LOG_ERROR = 1, //!< Output critical errors
        LOG_WARN,      //!< Output potentially critical warnings
        LOG_INFO,      //!< Output informational messages
        LOG_VERB,      //!< Be noisy
        LOG_ALL
    };

    /**
     * The logging topics.
     *
     * @sa net/log.h, client/log.h
     * @version 1.0
     */
    enum LogTopic
    {
        LOG_EXCEPTION = 0x01,    //!< Log exception within LBTHROW
        LOG_PLUGIN    = 0x02,    //!< Log plugin handling
        LOG_CUSTOM    = 0x10,    //!< Log topics for other namespaces start here
        LOG_ANY       = 0xffffu  //!< Log all Lunchbox topics
    };

    /** @internal The string buffer used for logging. */
    class LogBuffer : public std::streambuf
    {
    public:
        explicit LogBuffer( std::ostream& stream )
                : _indent(0), _blocked(0), _noHeader(0),
                  _newLine(true), _stream(stream)
            { _thread[0] = 0; _file[0] = 0; }
        virtual ~LogBuffer() {}

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

        LUNCHBOX_API void setThreadName( const std::string& name );
        const char* getThreadName() const { return _thread; }

        LUNCHBOX_API void setLogInfo( const char* file, const int line );

    protected:
        virtual int_type overflow( LogBuffer::int_type c ) override;
        virtual int sync() override;

    private:
        LogBuffer( const LogBuffer& );
        LogBuffer& operator = ( const LogBuffer& );

        /** Short thread name. */
        char _thread[12];

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

        /** The write lock. */
        static Lock _lock;
    };

    /** The logging class. @internal */
    class Log : public std::ostream
    {
    public:

        Log() : std::ostream( &_logBuffer ), _logBuffer( getOutput( )){}
        virtual ~Log() { _logBuffer.pubsync(); }

        void indent() { _logBuffer.indent(); }
        void exdent() { _logBuffer.exdent(); }
        void disableFlush() { _logBuffer.disableFlush(); }
        void enableFlush()  { _logBuffer.enableFlush();  }
        void forceFlush()  { _logBuffer.pubsync();  }
        void disableHeader() { _logBuffer.disableHeader(); }
        void enableHeader()  { _logBuffer.enableHeader();  }

        /** The current log level. */
        static LUNCHBOX_API int level;

        /** The current log topics. */
        static LUNCHBOX_API unsigned topics;

        /** The per-thread logger. */
        static LUNCHBOX_API Log& instance();

        /** The per-thread logger. */
        static LUNCHBOX_API Log& instance( const char* file, const int line );

        /** Exit the log instance for the current thread. */
        static LUNCHBOX_API void exit();

        /** @internal */
        static LUNCHBOX_API void reset();

        /** The string representation of the current log level. */
        static std::string& getLogLevelString();

        /** @return the log level of a string representation. @version 1.3.2 */
        static LUNCHBOX_API int getLogLevel( const char* level );

        /** Change the output stream. @version 1.4*/
        static LUNCHBOX_API void setOutput( std::ostream& stream );

        /** Change the output stream to the given file. @version 1.5.1 */
        static LUNCHBOX_API bool setOutput( const std::string& file );

        /** Get the current output stream. @internal */
        static LUNCHBOX_API std::ostream& getOutput ();

        /**
         * Set the reference clock.
         *
         * The clock will be used instantly by all log outputs. Use 0 to reset
         * the clock to the default clock.
         *
         * @param clock the reference clock.
         */
        static LUNCHBOX_API void setClock( Clock* clock );

        static const Clock& getClock(); //!< @internal

        /** @internal */
        void setThreadName( const std::string& name )
            { _logBuffer.setThreadName( name ); }

        /** @internal */
        const char* getThreadName() const { return _logBuffer.getThreadName(); }

    private:
        LogBuffer _logBuffer;

        Log( const Log& );
        Log& operator = ( const Log& );

        void setLogInfo( const char* file, const int line )
            { _logBuffer.setLogInfo( file, line ); }
    };

    /**
     * Increases the indentation level of the Log stream, causing subsequent
     * lines to be intended by four characters.
     * @version 1.0
     */
    LUNCHBOX_API std::ostream& indent( std::ostream& os );
    /** Decrease the indent of the Log stream. @version 1.0 */
    LUNCHBOX_API std::ostream& exdent( std::ostream& os );

    /** Disable flushing of the Log stream. @version 1.0 */
    LUNCHBOX_API std::ostream& disableFlush( std::ostream& os );
    /** Re-enable flushing of the Log stream. @version 1.0 */
    LUNCHBOX_API std::ostream& enableFlush( std::ostream& os );
    /** Flush the Log stream regardless of the auto-flush state. @version 1.0 */
    LUNCHBOX_API std::ostream& forceFlush( std::ostream& os );

    /** Disable printing of the Log header for subsequent lines. @version 1.0 */
    LUNCHBOX_API std::ostream& disableHeader( std::ostream& os );
    /** Re-enable printing of the Log header. @version 1.0 */
    LUNCHBOX_API std::ostream& enableHeader( std::ostream& os );
}

/** Output an error message to the per-thread Log stream. @version 1.0 */
#define LBERROR (lunchbox::Log::level >= lunchbox::LOG_ERROR) &&    \
    lunchbox::Log::instance( __FILE__, __LINE__ )
/** Output a warning message to the per-thread Log stream. @version 1.0 */
#define LBWARN  (lunchbox::Log::level >= lunchbox::LOG_WARN)  &&    \
    lunchbox::Log::instance( __FILE__, __LINE__ )
/** Output an informational message to the per-thread Log. @version 1.0 */
#define LBINFO  (lunchbox::Log::level >= lunchbox::LOG_INFO)  &&    \
    lunchbox::Log::instance( __FILE__, __LINE__ )

#ifdef NDEBUG
#  define LBVERB if( false )                                    \
        lunchbox::Log::instance( __FILE__, __LINE__ )
#else
/** Output a verbatim message to the per-thread Log stream. @version 1.0 */
#  define LBVERB (lunchbox::Log::level >= lunchbox::LOG_VERB)  &&    \
    lunchbox::Log::instance( __FILE__, __LINE__ )
#endif

/**
 * Output a message pertaining to a topic to the per-thread Log stream.
 * @version 1.0
 */
#define LBLOG(topic)  (lunchbox::Log::topics & (topic))  &&  \
    lunchbox::Log::instance( __FILE__, __LINE__ )

/**
 * Log a std::exception if topic LOG_EXCEPTION is set before throwing exception.
 * @version 1.7.1
 */
#define LBTHROW(exc)                                                \
    {                                                               \
        LBLOG(lunchbox::LOG_EXCEPTION) << exc.what() << std::endl;  \
        throw exc;                                                  \
    }

#endif //LUNCHBOX_LOG_H
