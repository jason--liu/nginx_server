#include <stdarg.h>
#include <unistd.h> //usleep
#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_threadpool.h"
#include "ngx_c_memory.h"
#include "ngx_macro.h"

pthread_mutex_t CThreadPool::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  CThreadPool::m_pthreadCond  = PTHREAD_COND_INITIALIZER;
bool            CThreadPool::m_shutdown     = false;

CThreadPool::CThreadPool()
{
    m_iRunningThreadNum = 0;
    m_iLastEmgTime      = 0;
}
CThreadPool::~CThreadPool() {}
bool CThreadPool::Create(int threadNum)
{
    ThreadItem* pNew;
    int         err;
    m_iThreadNum = threadNum;

    for (int i = 0; i < m_iThreadNum; i++)
    {
        m_threadVector.push_back(pNew = new ThreadItem(this));
        err = pthread_create(&pNew->_Handle, NULL, ThreadFunc, pNew);
        if (err != 0)
        {
            ngx_log_stderr(err, "create thread failed %d err=%d", i, err);
            return false;
        } else
        {
        }
    } // end for
    std::vector<ThreadItem*>::iterator iter;
lblfor:
    for (iter = m_threadVector.begin(); iter != m_threadVector.end(); iter++)
    {
        if ((*iter)->ifrunning == false)
        {
            usleep(100 * 1000);
            goto lblfor;
        }
    }
    return true;
}
void* CThreadPool::ThreadFunc(void* threadData)
{
    ThreadItem*  pThread        = static_cast<ThreadItem*>(threadData);
    CThreadPool* pThreadPoolObj = pThread->_pThis;

    char*    jobbuf   = NULL;
    CMemory* p_memory = CMemory::GetInstance();
    int      err;

    pthread_t tid = pthread_self();
    while (true)
    {
        err = pthread_mutex_lock(&m_pthreadMutex);
        if (err != 0)
            ngx_log_stderr(err, "pthread mutex lock error");
        while ((jobbuf = g_socket.outMsgRecvQueue()) == NULL && m_shutdown == false)
        {
            if (pThread->ifrunning == false)
                pThread->ifrunning = true;
            pthread_cond_wait(&m_pthreadCond, &m_pthreadMutex);
        }
        err = pthread_mutex_unlock(&m_pthreadMutex);
        if (err != 0)
            ngx_log_stderr(err, "pthread mutex unlock error");
        if (m_shutdown)
        {
            if (jobbuf != NULL)
            {
                p_memory->FreeMemory(jobbuf);
            }
            break;
        }
        pThreadPoolObj->m_iRunningThreadNum++;
        // ngx_log_stderr(0, "thread start %d", tid);
        // sleep(5);
        // ngx_log_stderr(0, "thread start %d", tid);
        p_memory->FreeMemory(jobbuf);
        pThreadPoolObj->m_iRunningThreadNum--;
    }
    return NULL;
}

void CThreadPool::StopAll()
{
    if (m_shutdown == true)
        return;

    m_shutdown = true;
    int err    = pthread_cond_broadcast(&m_pthreadCond);
    if (err != 0)
    {
        ngx_log_stderr(0, "broadcast error %d", err);
        return;
    }

    std::vector<ThreadItem*>::iterator iter;
    for (iter = m_threadVector.begin(); iter != m_threadVector.end(); iter++)
        pthread_join((*iter)->_Handle, NULL);

    pthread_mutex_destroy(&m_pthreadMutex);
    pthread_cond_destroy(&m_pthreadCond);

    for (iter = m_threadVector.begin(); iter != m_threadVector.end(); iter++)
    {
        if (*iter)
            delete *iter;
    }
    m_threadVector.clear();
    ngx_log_stderr(0, "StopAll success");
    return;
}

void CThreadPool::Call(int irmqc)
{
    int err = pthread_cond_signal(&m_pthreadCond);
    if (err != 0)
        ngx_log_stderr(err, "call failed");

    if (m_iThreadNum == m_iRunningThreadNum)
    {
        time_t currtime = time(NULL);
        if (currtime - m_iLastEmgTime > 10)
        {
            m_iLastEmgTime = currtime;
            ngx_log_stderr(0, "thread pool need to expand");
        }
    }
    return;
}
