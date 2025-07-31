#ifndef __CQUEUEBASE_H__
#define __CQUEUEBASE_H__

#include <deque>
using namespace std;

template<typename T>
class CQueueBase {

typedef unsigned int uint;

public:
    CQueueBase() { pthread_mutex_init(&m_pMutex, NULL); }
    virtual ~CQueueBase() { DeleteAll(); pthread_mutex_destroy(&m_pMutex); }

public:
    uint Size()
    {
        try {
            Lock();
            uint nSize = m_queue.size();
            Unlock();
            return nSize;
        } catch(...) {
            Unlock();
            return 0;
        }
    }

    bool IsEmpty()
    {
        try {
            Lock();
            bool bEmpty = m_queue.empty();
            Unlock();
            return bEmpty;
        } catch(...) {
            Unlock();
            return 0;
        }
    }

    bool DeleteAll()
    {
        if( IsEmpty() )
            return true;
        try {
            Lock();
            m_queue.clear();
            Unlock();
            return true;
        } catch(...) {
            Unlock();
            return 0;
        }
    }

    int Push(T t)
    {
        try {
            Lock();
            m_queue.push_back(t);
            Unlock();
            return(1);
        } catch(...) {
            Unlock();
            return 0;
        }
    }

    T Pop()
    {
        T t;
//        memset(&t, 0, sizeof(T)); //T -> POINTER ISSUE MYUNG
        try {
            Lock();

			if (m_queue.empty()) {
				Unlock();
		        return nullptr; 
    		}

            t = m_queue.front();
            m_queue.erase(m_queue.begin());
            Unlock();
            return t;
        } catch(...) {
            Unlock();
            return t;
        }
    }

protected:
    void Lock() { pthread_mutex_lock(&m_pMutex); }
    void Unlock() { pthread_mutex_unlock(&m_pMutex); }

protected:
    deque<T>            m_queue;

    pthread_mutex_t     m_pMutex;
};

#endif	// __CQUEUEBASE_H__

