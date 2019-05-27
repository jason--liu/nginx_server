#ifndef NGX_C_MEMORY_HH_
#define NGX_C_MEMORY_HH_
#include <stddef.h> //NULL

class CMemory {
private:
    CMemory() {}

public:
    ~CMemory(){};

private:
    static CMemory* m_instance;

public:
    static CMemory* GetInstance()
    {
        if (m_instance == NULL)
        {
            if (m_instance == NULL)
            {
                // lock
                m_instance = new CMemory();
                static CGarRecycle cr;
            }
            // unlock
        }
        return m_instance;
    }
    class CGarRecycle {
    public:
        ~CGarRecycle()
        {
            if (CMemory::m_instance)
            {
                delete CMemory::m_instance;
                CMemory::m_instance = NULL;
            }
        }
    };

public:
    void* AllocMemory(int memCount, bool ifmemset);
    void FreeMemory(void* point);
};

#endif
