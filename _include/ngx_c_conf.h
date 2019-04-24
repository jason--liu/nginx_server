#ifndef NGX_C_CONF_HH_
#define NGX_C_CONF_HH_

#include <vector>
#include <string>
#include "ngx_global.h"

class CConfig {
private:
    CConfig();

public:
    ~CConfig();

public:
    static CConfig* m_instance;

public:
    static CConfig* GetInstance()
    {
        if (m_instance == NULL)
        {
            if (m_instance == NULL)
            {
                m_instance = new CConfig();
                static CGarRecyle cl;
            }
        }
        return m_instance;
    }
    class CGarRecyle {
    public:
        ~CGarRecyle()
        {
            if (CConfig::m_instance)
            {
                delete CConfig::m_instance;
                CConfig::m_instance = NULL;
            }
        }
    };

public:
    bool Load(const char* pconfName);
    const char* GetString(const char* p_itemname);
    int GetIntDefault(const char* p_itemname, const int def);

public:
    std::vector<LPCConfItem> m_ConfigItemList; // store config info list
};

#endif
