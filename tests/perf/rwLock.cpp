
/* Copyright (c) 2011-2017, Stefan Eilemann <eile@eyescale.ch>
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

#define TEST_RUNTIME 600 // seconds, needed for NighlyMemoryCheck
#include <lunchbox/test.h>

#include <lunchbox/atomic.h>
#include <lunchbox/clock.h>
#include <lunchbox/debug.h>
#include <lunchbox/init.h>
#include <lunchbox/spinLock.h>

#include <iostream>

#pragma warning(push)
#pragma warning(disable : 4985) // ceil: attributes not present on previous...
#include <limits>
#pragma warning(pop)

#define MAXTHREADS 256
#define TIME 500 // ms

lunchbox::Clock _clock;
bool _running = false;

template <class T, uint32_t hold>
class WriteThread : public lunchbox::Thread
{
public:
    WriteThread()
        : lock(0)
        , ops(0)
        , sTime(0.)
    {
    }

    T* lock;
    size_t ops;
    double sTime;

    virtual void run()
    {
        ops = 0;
        sTime = 0.;
        while (LB_LIKELY(_running))
        {
            lock->set();
            TEST(lock->isSetWrite());
            // cppcheck-suppress duplicateExpression
            if (hold > 0) // static, optimized out
            {
                const double begin = _clock.getTimed();
                lunchbox::sleep(hold);
                sTime += _clock.getTimef() - begin;
            }
            lock->unset();

            ++ops;
        }
    }
};

template <class T, uint32_t hold>
class ReadThread : public lunchbox::Thread
{
public:
    ReadThread()
        : lock(0)
        , ops(0)
        , sTime(0.)
    {
    }

    T* lock;
    size_t ops;
    double sTime;

    void run() override
    {
        ops = 0;
        sTime = 0.;
        while (LB_LIKELY(_running))
        {
            lock->setRead();
            TEST(lock->isSetRead());
            // cppcheck-suppress duplicateExpression
            if (hold > 0) // static, optimized out
            {
                const double begin = _clock.getTimed();
                lunchbox::sleep(hold);
                sTime += _clock.getTimef() - begin;
            }
            lock->unsetRead();

            ++ops;
        }
    }
};

template <class T, uint32_t hold>
void _test()
{
    const size_t nThreads = 16;

    T lock;
    lock.set();

    WriteThread<T, hold> writers[MAXTHREADS];
    ReadThread<T, hold> readers[MAXTHREADS];

    std::cout << "               Class, write ops/ms,  read ops/ms, w threads, "
              << "r threads" << std::endl;
    for (size_t nWrite = 0; nWrite <= nThreads;
         nWrite = (nWrite == 0) ? 1 : nWrite << 1)
    {
        for (size_t i = 1; i <= nThreads; i = i << 1)
        {
            if (i < nWrite)
                continue;

            const size_t nRead = i - nWrite;
            _running = true;
            for (size_t j = 0; j < nWrite; ++j)
            {
                writers[j].lock = &lock;
                TEST(writers[j].start());
            }
            for (size_t j = 0; j < nRead; ++j)
            {
                readers[j].lock = &lock;
                TESTINFO(readers[j].start(), j);
            }
            lunchbox::sleep(10); // let threads initialize

            _clock.reset();
            lock.unset();
            lunchbox::sleep(TIME); // let threads run
            _running = false;

            for (size_t j = 0; j < nWrite; ++j)
                TEST(writers[j].join());
            for (size_t j = 0; j < nRead; ++j)
                TEST(readers[j].join());
            const double time = _clock.getTimed();

            TEST(!lock.isSet());
            lock.set();

            size_t nWriteOps = 0;
            double wTime = time * double(nWrite);
            for (size_t j = 0; j < nWrite; ++j)
            {
                nWriteOps += writers[j].ops;
                wTime -= writers[j].sTime;
            }
            if (nWrite > 0)
                wTime /= double(nWrite);
            if (wTime == 0.f)
                wTime = std::numeric_limits<double>::epsilon();

            size_t nReadOps = 0;
            double rTime = time * double(nRead);
            for (size_t j = 0; j < nRead; ++j)
            {
                nReadOps += readers[j].ops;
                rTime -= readers[j].sTime;
            }
            if (nRead > 0)
                rTime /= double(nRead);
            if (rTime == 0.f)
                rTime = std::numeric_limits<double>::epsilon();

            std::cout << std::setw(20) << lunchbox::className(lock) << ", "
                      << std::setw(12) << 3 * nWriteOps / wTime << ", "
                      << std::setw(12) << 3 * nReadOps / rTime << ", "
                      << std::setw(9) << nWrite << ", " << std::setw(9) << nRead
                      << std::endl;
        }
    }
}

int main(int argc, char** argv)
{
    TEST(lunchbox::init(argc, argv));
    //    lunchbox::sleep( 5000 );

    std::cerr << "0 ms in locked region" << std::endl;
    _test<lunchbox::SpinLock, 0>();
#if 0 // time collection not yet correct
    std::cerr << "1 ms in locked region" << std::endl;
    _test< lunchbox::SpinLock, 1 >();
    std::cerr << "2 ms in locked region" << std::endl;
    _test< lunchbox::SpinLock, 2 >();
    std::cerr << "4 ms in locked region" << std::endl;
    _test< lunchbox::SpinLock, 4 >();
#endif

    TEST(lunchbox::exit());
    return EXIT_SUCCESS;
}
