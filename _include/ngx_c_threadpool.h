#ifndef NGX_C_THREADPOOL_HH_
#define NGX_C_THREADPOOL_HH_

#include <vector>
#include <pthread.h>
#include <atomic>

class CThreadPool {
public:
    CThreadPool();
    ~CThreadPool();

public:
    bool Create(int threadNum);
    void StopAll();
    void Call(int irmqc);
private:
    static void* ThreadFunc(void *threadData);

private:
    struct ThreadItem {
        pthread_t    _Handle;
        CThreadPool* _pThis;
        bool         ifrunning;
        ThreadItem(CThreadPool* pthis) : _pThis(pthis), ifrunning(false) {}
        ~ThreadItem() {}
    };

private:
    static pthread_mutex_t m_pthreadMutex;
    static pthread_cond_t  m_pthreadCond;
    static bool            m_shutdown;
    int                    m_iThreadNum;

    std::atomic<int>         m_iRunningThreadNum;
    time_t                   m_iLastEmgTime;
    std::vector<ThreadItem*> m_threadVector; // thread container
};

#endif
