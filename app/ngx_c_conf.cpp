#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "ngx_c_conf.h"
#include "ngx_func.h"

CConfig* CConfig::m_instance = NULL;

CConfig::CConfig() {}

CConfig::~CConfig() {}

bool CConfig::Load(const char* pconfName)
{
    FILE* fp;
    fp = fopen(pconfName, "r");
    if (fp == NULL)
    {
        printf("Load config file failed\n %s", strerror(errno));
        return false;
    }

    char linebuf[501];
    while (!feof(fp))
    {
        if (fgets(linebuf, 500, fp) == NULL)
            continue;

        if (linebuf[0] == 0)
            continue;

        // process comment line
        if (*linebuf == ';' || *linebuf == ' ' || *linebuf == '#' || *linebuf == '\n' || *linebuf == '\t')
            continue;

    lblprocstring:
        // cut off end return tab space
        if (strlen(linebuf) > 0)
        {
            if (linebuf[strlen(linebuf) - 1] == 10 || linebuf[strlen(linebuf) - 1] == 13
                || linebuf[strlen(linebuf) - 1] == 32)
            {
                linebuf[strlen(linebuf) - 1] = 0;
                goto lblprocstring;
            }
        }
        if (linebuf[0] == 0)
            continue;
        if (*linebuf == '[') // jump [
            continue;

        // here is "ListenPort = 5678"
        char* ptmp = strchr(linebuf, '=');
        if (ptmp != NULL)
        {
            // now ptmp point to '='
            LPCConfItem p_confitem = new CConfItem;
            memset(p_confitem, 0, sizeof(CConfItem));
            strncpy(p_confitem->ItemName, linebuf, (int)(ptmp - linebuf)); // copy left side of = to ItemName
            strcpy(p_confitem->ItemContent, ptmp + 1);                     // copy right side of = to ItemContent

            Rtrim(p_confitem->ItemName);
            Ltrim(p_confitem->ItemName);
            Rtrim(p_confitem->ItemContent);
            Ltrim(p_confitem->ItemContent);

            m_ConfigItemList.push_back(p_confitem);
        } // endif
    }     // end while(!feof(fp))
    fclose(fp);
    return true;
}

const char* CConfig::GetString(const char* p_itemname)
{

    std::vector<LPCConfItem>::iterator pos;
    for (pos = m_ConfigItemList.begin(); pos != m_ConfigItemList.end(); ++pos)
    {
        if (strcasecmp((*pos)->ItemName, p_itemname) == 0)
            return (*pos)->ItemContent;
    }
    return NULL;
}

int CConfig::GetIntDefault(const char* p_itemname, const int def)
{
    std::vector<LPCConfItem>::iterator pos;
    for (pos = m_ConfigItemList.begin(); pos != m_ConfigItemList.end(); ++pos)
    {
        if (strcasecmp((*pos)->ItemName, p_itemname) == 0)
            return atoi((*pos)->ItemContent);
    }
    return def;
}
