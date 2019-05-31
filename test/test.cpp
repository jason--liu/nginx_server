#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <iostream>

#pragma pack(1)
typedef struct _COMM_PKG_HEADER {
    unsigned short pkgLen;  // total pkg length 2 bytes
    unsigned short msgCode; // msg type code 2 bytes
    int            crc32;
} COMM_PKG_HEADER, *LPCOMM_PKG_HEADER;

typedef struct _STRUCT_REGISTERE {
    int  iType;
    char username[56];
    char passwork[40];
} STRUCT_REGISTER, *LPSTRUCT_REGISTER;

typedef struct _STRUCT_LOGIN {
    char username[56];
    char passwork[40];
} STRUCT_LOGIN, *LPSTRUCT_LOGIN;

#pragma pack()

static int g_iLenPkgHeader = sizeof(COMM_PKG_HEADER);

static int SendData(int sSocket, char* p_sendbuf, int ibuflen)
{
    int usend  = ibuflen; //要发送的数目
    int uwrote = 0;       //已发送的数目
    int tmp_sret;

    while (uwrote < usend)
    {
        tmp_sret = send(sSocket, p_sendbuf + uwrote, usend - uwrote, 0);
        if ((tmp_sret == -1) || (tmp_sret == 0))
        {
            //有错误发生了
            perror("send error!\n");
            return -1;
        }
        uwrote += tmp_sret;
    } // end while
    return uwrote;
}

static void SendPackage(int sClient)
{
    char* p_sendbuf = (char*)new char[g_iLenPkgHeader + sizeof(STRUCT_REGISTER)];

    LPCOMM_PKG_HEADER pinfohead;
    pinfohead          = (LPCOMM_PKG_HEADER)p_sendbuf;
    pinfohead->msgCode = 1;
    pinfohead->msgCode = htons(pinfohead->msgCode);
    pinfohead->crc32   = htonl(123);
    pinfohead->pkgLen  = htons(g_iLenPkgHeader + sizeof(STRUCT_REGISTER));

    LPSTRUCT_REGISTER pstruc_sendstruc = (LPSTRUCT_REGISTER)(p_sendbuf + g_iLenPkgHeader);
    pstruc_sendstruc->iType            = htonl(1000);
    strcpy(pstruc_sendstruc->username, "1234");

    if (SendData(sClient, p_sendbuf, g_iLenPkgHeader + sizeof(STRUCT_REGISTER)) == -1)
    {
        delete[] p_sendbuf;
        return;
    }
    delete[] p_sendbuf;

    // LOGIN
    p_sendbuf = (char*)new char[g_iLenPkgHeader + sizeof(STRUCT_LOGIN)];

    pinfohead          = (LPCOMM_PKG_HEADER)p_sendbuf;
    pinfohead->msgCode = 2;
    pinfohead->msgCode = htons(pinfohead->msgCode);
    pinfohead->crc32   = htonl(345);
    pinfohead->pkgLen  = htons(g_iLenPkgHeader + sizeof(STRUCT_LOGIN));

    LPSTRUCT_LOGIN pstruc_sendstruc2 = (LPSTRUCT_LOGIN)(p_sendbuf + g_iLenPkgHeader);
    strcpy(pstruc_sendstruc->username, "5678");
    if (SendData(sClient, p_sendbuf, g_iLenPkgHeader + sizeof(STRUCT_LOGIN)) == -1)
    {
        delete[] p_sendbuf;
        return;
    }
    delete[] p_sendbuf;

    std::cout << "nice we have send 2 package" << std::endl;
}

int main()
{
    static int ifinit  = 0;
    int        sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sClient < 0)
    {
        perror("create client failed !\n");
        return -1;
    }
    struct sockaddr_in server_in;
    memset(&server_in, 0, sizeof(sockaddr_in));
    server_in.sin_family      = AF_INET; // IPV4
    server_in.sin_port        = htons(10000);
    server_in.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sClient, (struct sockaddr*)&server_in, sizeof(sockaddr_in)) == -1)
    {
        perror("connect client failed !\n");
        return -1;
    }

    // int iSendRecvTimeOut = 5000;
    // if (setsockopt(sClient, SOL_SOCKET, SO_SNDTIMEO, (const char*)&iSendRecvTimeOut, sizeof(int)) == -1)
    // {
    //     perror("set send timeout failed !\n");
    //     close(sClient);
    //     return -1;
    // }
    // if (setsockopt(sClient, SOL_SOCKET, SO_RCVTIMEO, (const char*)&iSendRecvTimeOut, sizeof(int)) == -1)
    // {
    //     perror("set send timeout failed !\n");
    //     close(sClient);
    //     return -1;
    // }
    SendPackage(sClient);

    return 0;
}
