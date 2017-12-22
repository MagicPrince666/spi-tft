#ifndef __GBK_UTF8_H__
#define __GBK_UTF8_H__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <iconv.h>

/*
    文件描述：
    实现gbk与utf8间的转换
*/

int u2g(const char *inbuf, size_t inlen, char *outbuf, size_t outlen);
int g2u(const char *inbuf, size_t inlen, char *outbuf, size_t outlen);

#ifdef __cplusplus
}
#endif

#endif
