#ifndef NGX_COMM_HH_
#define NGX_COMM_HH_

#define _PKG_MAX_LENGTH 30000 // max length of a package

// package state
#define _PKG_HD_INIT 0
#define _PKG_HD_RECVING 1
#define _PKG_BD_INIT 2
#define _PKG_BD_RECVING 3

#define _DATA_BUFSIZE_ 20 // receive package head

#pragma pack(1)
typedef struct _COMM_PKG_HEADER {
    unsigned short pkgLen;  // total pkg length 2 bytes
    unsigned short msgCode; // msg type code 2 bytes
    int            crc32;
} COMM_PKG_HEADER, *LPCOMM_PKG_HEADER;
#pragma pack()

#endif
