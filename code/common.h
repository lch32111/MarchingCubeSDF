#ifndef __COMMON_H__
#define __COMMON_H__

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <unordered_map>
#include <condition_variable>
#include <queue>

#define PID 3.14159265359
#define PIF 3.14159265359f
#define GET_RADIAND(degree) (degree) * PID / 180.0
#define GET_RADIANF(degree) (degree) * PIF / 180.f

#ifndef ALLOCA
	#if defined(_MSC_VER)
		#include <malloc.h>
		#define ALLOCA _alloca
	#elif defined(__GNUC__) || defined(__clang__)
		#include <alloca.h>
		#define ALLOCA alloca
	#endif
#endif

typedef void(*Job)(void*);
struct QueueElement
{
    Job function;
    void* argument;
};

class ThreadPool
{
public:

    enum
    {
        SHUTDOWN_IMMEDIATE = (1 << 0),
        SHUTDOWN_GRACEFULLY = (1 << 1)
    };

    ThreadPool()
        : _shutdownFlag(0)
    {
        _threadCount = std::thread::hardware_concurrency();
        _threads.resize(_threadCount);
        for (int i = 0; i < _threadCount; ++i)
        {
            _threads[i] = std::thread(&ThreadPool::_threadpoolWorkerFunction, this);
        }
    }

    ~ThreadPool()
    {
        if (_threadCount != 0)
        {
            Join(SHUTDOWN_IMMEDIATE);
        }
    }

    void Join(int flag)
    {
        _mutex.lock();

        _shutdownFlag = flag;
        _condition.notify_all();

        _mutex.unlock();

        for (std::thread& t : _threads)
        {
            t.join();
        }

        _threads.clear();
        _threadCount = 0;
    }

    void EnqueueJob(Job f, void* argument)
    {
        _mutex.lock();

        _queue.push({ f, argument });
        _condition.notify_one();

        _mutex.unlock();
    }

    size_t GetThreadCount() const { return _threads.size(); }

private:

    int _shutdownFlag;

    int _threadCount;
    std::vector<std::thread> _threads;
    std::queue<QueueElement> _queue;

    std::mutex _mutex;
    std::condition_variable _condition;

    void _threadpoolWorkerFunction()
    {
        ThreadPool* tp = this;

        while (true)
        {
            std::unique_lock<std::mutex> ul(tp->_mutex);

            tp->_condition.wait
            (
                ul,
                [tp]
                {
                    return (tp->_queue.size() > 0 || tp->_shutdownFlag != 0);
                }
            );

            if (tp->_shutdownFlag == SHUTDOWN_IMMEDIATE ||
                (tp->_shutdownFlag == SHUTDOWN_GRACEFULLY && tp->_queue.size() == 0))
            {
                break;
            }

            QueueElement qe = tp->_queue.front();
            tp->_queue.pop();

            ul.unlock();

            qe.function(qe.argument);
        }
    }
};

#if _WIN32 || _WIN64
// win32 specific function. You don't need to use these functions on another platform
void str_widen(const char* str, int strLenWithNULL, wchar_t* buffer, int bufferByteSize);
void str_narrow(const wchar_t* str, int wcsLenWithNULL, char* buffer, int bufferByteSize);
#endif

FILE* open_file(const char* utf8Path, const char* mode);
bool file_read_until_total_size(FILE* fp, int64_t total_size, void* buffer);
void file_open_fill_buffer(const char* path, std::vector<char>& buffer);

bool is_file_exist(const char* utf8_path);

#endif