
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <iostream>
#include <lunchbox/clock.h>
#include <lunchbox/lfQueue.h>
#include <lunchbox/test.h>
#include <lunchbox/thread.h>

#define RUNTIME 1000 /*ms*/

lunchbox::LFQueue<uint64_t> queue(1024);

class ReadThread : public lunchbox::Thread
{
public:
    virtual ~ReadThread() {}
    virtual void run()
    {
        uint64_t nOps = 0;
        uint64_t nEmpty = 0;
        uint64_t item = 0xffffffffffffffffull;

        lunchbox::Clock clock;
        while (clock.getTime64() < RUNTIME)
        {
            if (queue.getFront(item))
            {
                TEST(item == nOps);
                uint64_t item2 = 0xffffffffffffffffull;
                TEST(queue.pop(item2));
                TEST(item2 == item);
                ++nOps;
            }
            TEST(item + 1 == nOps);
            ++nEmpty;
        }
        const float time = clock.getTimef();
        std::cout << 2 * nOps / time << " reads/ms, " << nEmpty / time
                  << " empty/ms" << std::endl;
    }
};

int main(int, char**)
{
    ReadThread reader;
    uint64_t nOps = 0;
    uint64_t nEmpty = 0;

    TEST(reader.start());

    lunchbox::Clock clock;
    while (clock.getTime64() < RUNTIME)
    {
        while (queue.push(nOps))
            ++nOps;
        ++nEmpty;
    }
    const float time = clock.getTimef();

    TEST(reader.join());
    std::cout << nOps / time << " writes/ms, " << nEmpty / time << " full/ms"
              << std::endl;

    return EXIT_SUCCESS;
}
