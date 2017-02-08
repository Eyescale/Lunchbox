
/* Copyright (c) 2006-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *               2012-2013, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#define TEST_RUNTIME 300 // seconds
#include <iostream>
#include <lunchbox/clock.h>
#include <lunchbox/refPtr.h>
#include <lunchbox/referenced.h>
#include <lunchbox/test.h>
#include <lunchbox/thread.h>

#include <boost/intrusive_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#define NTHREADS 24
#define NREFS 200000

class Foo : public lunchbox::Referenced
{
public:
    Foo() {}
private:
    virtual ~Foo() {}
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive&, unsigned int)
    {
    }
};

typedef lunchbox::RefPtr<Foo> FooPtr;
FooPtr foo;

class TestThread : public lunchbox::Thread
{
public:
    virtual void run()
    {
        FooPtr myFoo;
        for (size_t i = 0; i < NREFS; ++i)
        {
            myFoo = foo;
            foo = myFoo;
            myFoo = 0;
        }
    }
};

typedef boost::intrusive_ptr<Foo> BoostPtr;
BoostPtr bFoo;

class BThread : public lunchbox::Thread
{
public:
    virtual void run()
    {
        BoostPtr myBoost;
        for (size_t i = 0; i < NREFS; ++i)
        {
            myBoost = bFoo;
            bFoo = myBoost;
            myBoost = 0;
        }
    }
};

class Bar : public lunchbox::Referenced
{
public:
    Bar() {}
    virtual ~Bar() {}
};

typedef boost::shared_ptr<Bar> BarPtr;
BarPtr bBar;

class BarThread : public lunchbox::Thread
{
public:
    virtual void run()
    {
        BarPtr myBar;
        for (size_t i = 0; i < NREFS; ++i)
        {
            myBar = bBar;
            bBar = myBar;
            myBar.reset();
        }
    }
};

int main(int, char**)
{
    foo = new Foo;

    TestThread threads[NTHREADS];
    lunchbox::Clock clock;
    for (size_t i = 0; i < NTHREADS; ++i)
        TEST(threads[i].start());

    for (size_t i = 0; i < NTHREADS; ++i)
        TEST(threads[i].join());

    const float time = clock.getTimef();
    std::cout << time << " ms for " << 3 * NREFS
              << " lunchbox::RefPtr operations"
              << " in " << NTHREADS << " threads ("
              << time / (3 * NREFS * NTHREADS) * 1000000 << "ns/op)"
              << std::endl;

    TEST(foo->getRefCount() == 1);

    bFoo = new Foo;
    BThread bThreads[NTHREADS];
    clock.reset();
    for (size_t i = 0; i < NTHREADS; ++i)
        TEST(bThreads[i].start());

    for (size_t i = 0; i < NTHREADS; ++i)
        TEST(bThreads[i].join());

    const float bTime = clock.getTimef();
    std::cout << bTime << " ms for " << 3 * NREFS
              << " boost::intrusive_ptr ops "
              << "in " << NTHREADS << " threads ("
              << bTime / (3 * NREFS * NTHREADS) * 1000000 << "ns/op)"
              << std::endl;

    TEST(bFoo->getRefCount() == 1);

    boost::intrusive_ptr<Foo> boostFoo(foo.get());
    TEST(foo->getRefCount() == 2);

    boostFoo = 0;
    TEST(foo->getRefCount() == 1);

    bBar = BarPtr(new Bar);
    BarThread barThreads[NTHREADS];

    clock.reset();
    for (size_t i = 0; i < NTHREADS; ++i)
        TEST(barThreads[i].start());

    for (size_t i = 0; i < NTHREADS; ++i)
        TEST(barThreads[i].join());

    const float barTime = clock.getTimef();
    std::cout << barTime << " ms for " << 3 * NREFS
              << " boost::shared_ptr ops in " << NTHREADS << " threads ("
              << barTime / (3 * NREFS * NTHREADS) * 1000000 << "ns/op)"
              << std::endl;

    bBar = boost::make_shared<Bar>();
    clock.reset();
    for (size_t i = 0; i < NTHREADS; ++i)
        TEST(barThreads[i].start());

    for (size_t i = 0; i < NTHREADS; ++i)
        TEST(barThreads[i].join());

    const float barTime2 = clock.getTimef();
    std::cout << barTime2 << " ms for " << 3 * NREFS
              << " boost::shared_ptr ops in " << NTHREADS << " threads ("
              << barTime2 / (3 * NREFS * NTHREADS) * 1000000
              << "ns/op) using make_shared" << std::endl;

    foo = 0;
    return EXIT_SUCCESS;
}
