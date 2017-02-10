/* Copyright (c) 2005-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "fork.h"

#include "debug.h"
#include "log.h"
#include "os.h"

#include <errno.h>
#include <signal.h>
#include <sstream>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace lunchbox
{
namespace
{
#ifndef _WIN32
Strings _buildCommandLine(const std::string& command)
{
    Strings commandLine;
    const size_t length = command.size();
    const char* string = command.c_str();
    bool inTicks = false;
    size_t bufferPos = 0;
    std::vector<char> buffer(length + 1);

    commandLine.clear();

    // tokenize command line
    for (size_t i = 0; i < length; i++)
    {
        const char c = string[i];
        switch (c)
        {
        case ' ':
            if (inTicks)
                buffer[bufferPos++] = c;
            else
            {
                buffer[bufferPos] = '\0';
                commandLine.push_back(&buffer[0]);
                bufferPos = 0;
            }
            break;

        case '"':
            inTicks = !inTicks;
            break;

        case '\\':
            i++;
            buffer[bufferPos++] = string[i];
            break;

        default:
            buffer[bufferPos++] = c;
            break;
        }
    }

    if (bufferPos > 0)
    {
        buffer[bufferPos++] = '\0';
        commandLine.push_back(&buffer[0]);
    }

    return commandLine;
}
#endif
}

bool fork(const std::string& command)
{
    if (command.empty())
        return false;

#ifdef _WIN32
    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(STARTUPINFO));

    PROCESS_INFORMATION procInfo;
    ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

    const char* cmdLine = command.c_str();

    startupInfo.cb = sizeof(STARTUPINFO);
    const bool success =
        CreateProcess(0, LPSTR(cmdLine), // program, command line
                      0, 0,              // process, thread attributes
                      FALSE,             // inherit handles
                      0,                 // creation flags
                      0,                 // environment
                      0,                 // current directory
                      &startupInfo, &procInfo);

    if (!success)
    {
        LBERROR << "CreateProcess failed: " << sysError << std::endl;
        return false;
    }

    // WaitForInputIdle( procInfo.hProcess, 1000 );
    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);
    return true;
#else
    const Strings commandLine = _buildCommandLine(command);

    struct sigaction act;
    setZero(&act, sizeof(act));
    act.sa_handler = SIG_DFL;
    act.sa_flags = SA_NOCLDWAIT;
    ::sigaction(SIGCHLD, &act, &act);

    const int result = ::fork();
    switch (result)
    {
    case 0: // child
        break;

    case -1: // error
        LBWARN << "Could not fork child process:" << sysError << std::endl;
        return false;

    default: // parent
        return true;
    }

    // child
    const size_t argc = commandLine.size();
    std::vector<char*> argv(argc + 1);
    std::ostringstream stringStream;

    for (size_t i = 0; i < argc; i++)
    {
        argv[i] = (char*)commandLine[i].c_str();
        stringStream << commandLine[i] << " ";
    }

    argv[argc] = 0;

    LBDEBUG << "Executing: " << stringStream.str() << std::endl;
    int nTries = 10;
    while (nTries--)
    {
        execvp(argv[0], &argv[0]);
        LBWARN << "Error executing '" << argv[0] << "': " << sysError
               << std::endl;
        if (errno != ETXTBSY)
            break;
    }

    ::exit(EXIT_FAILURE);
    return true; // not reached
#endif
}
}
