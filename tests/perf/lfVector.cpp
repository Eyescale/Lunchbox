
/* Copyright (c) 2011-2016, Stefan Eilemann <stefan.eilemann@epfl.ch>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
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

#pragma warning(push)
#pragma warning(disable : 4985) // http://www.softwareverify.com/blog/?p=671
#include <lunchbox/lfVector.h>
#pragma warning(pop)

#include <lunchbox/clock.h>
#include <lunchbox/init.h>
#include <lunchbox/monitor.h>
#include <lunchbox/thread.h>

#include <limits>

#define LOOPSIZE 200000

lunchbox::Clock _clock;
typedef lunchbox::LFVector<size_t> Vector_t;
float rTime_;
float wTime_;
float eTime_;
float cTime_;
float pTime_;

lunchbox::Monitor<size_t> stage_;
#define STAGESIZE 10000;

class Reader : public lunchbox::Thread
{
public:
    Reader()
        : vector(0)
    {
    }
    virtual ~Reader() {}
    virtual void run()
    {
        size_t stage = 0;
        while (true)
        {
            stage += STAGESIZE;
            stage_.waitGE(stage);
            if (stage_ == std::numeric_limits<size_t>::max())
                return;

            if (vector)
            {
                for (size_t i = 0; i < LOOPSIZE; ++i)
                {
                    if (i < vector->size())
                    {
                        const size_t value = (*vector)[i];
                        TESTINFO(i == value || value == 0,
                                 i << " - " << value << " - " << (*vector)[i]
                                   << ": " << *vector);
                    }
                }
                rTime_ = _clock.getTimef();
            }
            ++stage_;
        }
    }

    Vector_t* vector;
    // cppcheck-suppress operatorEqVarError
    Reader& operator=(const Reader&) { return *this; }
};

class Writer : public lunchbox::Thread
{
public:
    Writer()
        : vector(0)
    {
    }
    virtual ~Writer() {}
    virtual void run()
    {
        size_t stage = 0;
        while (true)
        {
            stage += STAGESIZE;
            stage_.waitGE(stage);
            if (stage_ == std::numeric_limits<size_t>::max())
                return;

            if (vector)
            {
                for (size_t i = 0; i < LOOPSIZE; ++i)
                {
                    if (i < vector->size())
                        (*vector)[i] = i;
                }
                wTime_ = _clock.getTimef();
            }
            ++stage_;
        }
    }

    Vector_t* vector;
    // cppcheck-suppress operatorEqVarError
    Writer& operator=(const Writer&) { return *this; }
};

class Pusher : public lunchbox::Thread
{
public:
    Pusher()
        : vector(0)
    {
    }
    virtual ~Pusher() {}
    virtual void run()
    {
        size_t stage = 0;
        while (true)
        {
            stage += STAGESIZE;
            stage_.waitGE(stage);
            if (stage_ == std::numeric_limits<size_t>::max())
                return;

            if (vector)
            {
                while (vector->size() < LOOPSIZE)
                    vector->push_back(0);
                pTime_ = _clock.getTimef();
            }
            ++stage_;
        }
    }

    Vector_t* vector;
    // cppcheck-suppress operatorEqVarError
    Pusher& operator=(const Pusher&) { return *this; }
};

class Copier : public lunchbox::Thread
{
public:
    Copier()
        : vector(0)
    {
    }
    virtual ~Copier() {}
    virtual void run()
    {
        Vector_t copy;
        copy = *vector;
        *vector = copy;
        TEST(copy.size() >= vector->size());
        cTime_ = _clock.getTimef();
    }
    Vector_t* vector;

    // cppcheck-suppress operatorEqVarError
    Copier& operator=(const Copier&) { return *this; }
};

class Eraser : public lunchbox::Thread
{
public:
    Eraser()
        : vector(0)
    {
    }
    virtual ~Eraser() {}
    virtual void run()
    {
        const size_t pos = vector->size() / 2;
        Vector_t::iterator i = vector->begin() + pos;
        Vector_t::iterator j = vector->erase(i);
        TEST(j != vector->end());
        TEST(i == j);
        eTime_ = _clock.getTimef();
    }
    Vector_t* vector;

    // cppcheck-suppress operatorEqVarError
    Eraser& operator=(const Eraser&) { return *this; }
};

class Flusher : public lunchbox::Thread
{
public:
    Flusher()
        : vector(0)
    {
    }
    virtual ~Flusher() {}
    virtual void run()
    {
        size_t i = 0xFFFFFFFFu;
        while (vector->pop_back(i))
        {
            TEST(i == 0 || i >= vector->size());
        }
    }
    Vector_t* vector;
    // cppcheck-suppress operatorEqVarError
    Flusher& operator=(const Flusher&) { return *this; }
};

template <class V, class T>
void _runSerialTest()
{
    V vector;

    _clock.reset();
    while (vector.size() < LOOPSIZE * 10)
        vector.push_back(T(0));
    pTime_ = _clock.getTimef();

    _clock.reset();
    for (size_t i = 0; i < LOOPSIZE * 10; ++i)
    {
        if (i < vector.size())
            vector[i] = i;
    }
    wTime_ = _clock.getTimef();

    _clock.reset();
    typename V::const_iterator it = vector.begin();
    for (size_t i = 0; i < LOOPSIZE * 10; ++i)
    {
        const size_t value = *it;
        TESTINFO(i == value || value == 0, i << ", " << value);
        ++it;
        if (it == vector.end())
            it = vector.begin();
    }
    rTime_ = _clock.getTimef();

    _clock.reset();
    for (size_t i = 0; i < 10; ++i)
    {
        V copy;
        copy = vector;
        vector = copy;
        TEST(copy.size() >= vector.size());
    }
    cTime_ = _clock.getTimef();

    _clock.reset();
    const size_t pos = vector.size() / 2;
    typename V::iterator i = vector.begin() + pos;
    typename V::iterator j = vector.erase(i);
    TEST(j != vector.end());
    TEST(*j == 0 || *j >= pos);
    eTime_ = _clock.getTimef();

    _clock.reset();
    while (!vector.empty())
        vector.pop_back();
    const float fTime = _clock.getTimef();

    std::cerr << std::setw(11) << float(LOOPSIZE * 10) / rTime_ << ", "
              << std::setw(11) << float(LOOPSIZE * 10) / wTime_ << ", "
              << std::setw(11) << float(LOOPSIZE * 10) / pTime_ << ", "
              << std::setw(9) << float(10) / cTime_ << ", " << std::setw(9)
              << float(10) / eTime_ << ", " << std::setw(9)
              << float(LOOPSIZE * 10) / fTime << ", " << std::setw(3) << 0
              << ", " << std::setw(3) << 0 << std::endl;

    vector.push_back(42);
    i = vector.begin();
    i = vector.erase(i);
    TEST(i == vector.begin());
    TEST(vector.empty());

    vector.push_back(42);
    vector.push_back(17);
    vector.resize(1);
    TEST(vector.size() == 1);
    TEST(vector[0] == 42);

    vector.resize(10, 17);
    TEST(vector.size() == 10);
    TEST(vector[0] == 42);
    TEST(vector[1] == 17);
    TEST(vector[9] == 17);
}

int main(int, char**)
{
    const size_t nThreads = 16;

    std::cout << "       read,       write,        push,      copy,     erase, "
              << " flush/ms,  rd, other #threads" << std::endl;
    _runSerialTest<std::vector<size_t>, size_t>();
    _runSerialTest<Vector_t, size_t>();

    std::vector<Reader> readers(nThreads);
    std::vector<Writer> writers(nThreads);
    std::vector<Pusher> pushers(nThreads);

    stage_ = 1;
    size_t stage = 0;

    for (size_t l = 0; l < nThreads; ++l)
    {
        readers[l].start();
        writers[l].start();
        pushers[l].start();
    }
    lunchbox::sleep(10);

    for (size_t i = 1; i <= nThreads; i = i << 1)
        for (size_t j = 1; j <= nThreads; j = j << 1)
        {
            // concurrent read, write, push
            Vector_t vector;
            for (size_t k = 0; k < nThreads; ++k)
            {
                readers[k].vector = k < i ? &vector : 0;
                writers[k].vector = k < j ? &vector : 0;
                pushers[k].vector = k < j ? &vector : 0;
            }

            const size_t nextStage = ++stage * STAGESIZE;

            _clock.reset();
            stage_ = nextStage;
            stage_.waitEQ(nextStage + (3 * nThreads));
            TEST(vector.size() >= LOOPSIZE);

            // multi-threaded copy
            std::vector<Copier> copiers(j);

            _clock.reset();
            for (size_t k = 0; k < j; ++k)
            {
                copiers[k].vector = &vector;
                copiers[k].start();
            }
            for (size_t k = 0; k < j; ++k)
                copiers[k].join();

            for (size_t k = 0; k < vector.size(); ++k)
                TEST(vector[k] == k || vector[k] == 0);

            // multi-threaded erase
            std::vector<Eraser> erasers(j);

            _clock.reset();
            for (size_t k = 0; k < j; ++k)
            {
                erasers[k].vector = &vector;
                erasers[k].start();
            }
            for (size_t k = 0; k < j; ++k)
                erasers[k].join();

            for (size_t k = 0; k < vector.size(); ++k)
            {
                if (vector[k] == 0)
                    break;
                if (k > vector.size() / 2)
                {
                    TEST(vector[k] > vector[k - 1]);
                }
                else
                {
                    TEST(vector[k] == k);
                }
            }

            // multi-threaded pop_back
            const size_t fOps = vector.size();
            std::vector<Flusher> flushers(j);
            _clock.reset();
            for (size_t k = 0; k < j; ++k)
            {
                flushers[k].vector = &vector;
                flushers[k].start();
            }
            for (size_t k = 0; k < j; ++k)
                flushers[k].join();
            const float fTime = _clock.getTimef();
            TEST(vector.empty());

            std::cerr << std::setw(11) << float(i * LOOPSIZE) / rTime_ << ", "
                      << std::setw(11) << float(j * LOOPSIZE) / wTime_ << ", "
                      << std::setw(11) << float(LOOPSIZE) / pTime_ << ", "
                      << std::setw(9) << float(j) / cTime_ << ", "
                      << std::setw(9) << float(j) / eTime_ << ", "
                      << std::setw(9) << float(fOps) / fTime << ", "
                      << std::setw(3) << i << ", " << std::setw(3) << j
                      << std::endl;
        }

    stage_ = std::numeric_limits<size_t>::max();
    for (size_t k = 0; k < nThreads; ++k)
    {
        readers[k].join();
        writers[k].join();
        pushers[k].join();
    }

    return EXIT_SUCCESS;
}
