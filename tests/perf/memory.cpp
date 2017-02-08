
/* Copyright (c) 2015, Stefan.Eilemann@epfl.ch
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

#define TEST_RUNTIME 600 // seconds
#include <lunchbox/test.h>

#include <boost/lexical_cast.hpp>
#include <lunchbox/lunchbox.h>
#include <string.h>

enum Task
{
    TASK_INITIAL,
    TASK_MEMCPY,
    TASK_MEMMOVE,
    TASK_MEMSET
};
lunchbox::Monitor<Task> task;
static const size_t nThreads = 32;
static const size_t maxSize = 4 * size_t(LB_1GB);

using boost::lexical_cast;

class TaskThread : public lunchbox::Thread
{
public:
    TaskThread()
        : size(maxSize)
        , memcpyTime(0)
        , memmoveTime(0)
        , memsetTime(0)
    {
    }

    void run() override
    {
        // alloc and touch all pages once
        void* const from = ::malloc(size);
        void* const to = ::malloc(size);
        ::memset(from, 0, size);
        ::memset(to, 0, size);

        lunchbox::Clock clock;

        task.waitGE(TASK_MEMCPY);
        clock.reset();
        // cppcheck-suppress redundantCopy
        ::memcpy(to, from, size);
        memcpyTime = clock.getTimef();

        task.waitGE(TASK_MEMMOVE);
        clock.reset();
        // cppcheck-suppress redundantCopy
        ::memmove(to, from, size);
        memmoveTime = clock.getTimef();

        task.waitGE(TASK_MEMSET);
        clock.reset();
        // cppcheck-suppress redundantCopy
        ::memset(to, 42, size);
        memsetTime = clock.getTimef();

        ::free(to);
        ::free(from);
    }

    size_t size;
    float memcpyTime;
    float memmoveTime;
    float memsetTime;
};

int main(int argc, char** argv)
{
    TEST(lunchbox::init(argc, argv));

    TaskThread thread;
    task = TASK_MEMSET;
    thread.run();
    std::cout << "#threads, memcpy, memmove, memset (GB/s)" << std::endl;

    TaskThread threads[nThreads];
    for (size_t i = 1; i <= nThreads; i = i << 1)
    {
        task = TASK_INITIAL;
        for (size_t j = 0; j < i; ++j)
        {
            threads[j].size = maxSize / i;
            threads[j].start();
        }

        lunchbox::sleep(500);
        task = TASK_MEMCPY;
        lunchbox::sleep(500);
        task = TASK_MEMMOVE;
        lunchbox::sleep(500);
        task = TASK_MEMSET;

        float memcpyTime = 0.f;
        float memmoveTime = 0.f;
        float memsetTime = 0.f;

        for (size_t j = 0; j < i; ++j)
        {
            threads[j].join();
            memcpyTime = std::max(memcpyTime, threads[j].memcpyTime);
            memmoveTime = std::max(memmoveTime, threads[j].memmoveTime);
            memsetTime = std::max(memsetTime, threads[j].memsetTime);
        }
        const float sizeGB = double(maxSize) / 1024. / 1024. / 1024.;
        std::cout << i << ", " << sizeGB / memcpyTime * 1000.f << ", "
                  << sizeGB / memmoveTime * 1000.f << ", "
                  << sizeGB / memsetTime * 1000.f << std::endl;
    }

    std::cout << std::endl
              << "size, memcpy, memmove, memset (GB/s)" << std::endl;
    TaskThread worker;
    for (size_t i = maxSize; i != 0; i = i >> 1)
    {
        task = TASK_MEMSET;
        worker.size = i;
        worker.run();

        const float sizeGB = double(i) / 1024. / 1024. / 1024.;
        const std::string size =
            i >= LB_1GB
                ? lexical_cast<std::string>(i >> 30) + " GB"
                : i >= LB_1MB
                      ? lexical_cast<std::string>(i >> 20) + " MB"
                      : i >= LB_1KB ? lexical_cast<std::string>(i >> 10) + " KB"
                                    : lexical_cast<std::string>(i) + " B";

        std::cout << size << ", " << sizeGB / worker.memcpyTime * 1000.f << ", "
                  << sizeGB / worker.memmoveTime * 1000.f << ", "
                  << sizeGB / worker.memsetTime * 1000.f << std::endl;
    }

    TEST(lunchbox::exit());
    return EXIT_SUCCESS;
}
