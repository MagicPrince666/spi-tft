#include "gbk_utf8.h"

extern "C" {

static int code_convert(const char *from_charset, const char *to_charset, const char *inbuf, size_t inlen,
		char *outbuf, size_t outlen) {
	iconv_t cd;
	//const char **pin = (const char **)&inbuf;
	char **pout = &outbuf;
	
	cd = iconv_open(to_charset, from_charset);
	if (cd == 0)
		return -1;
	memset(outbuf, 0x00, outlen);
	//这行有bug 出在&inbuf的编码上
	if (-1 == iconv(cd, (const char **)&inbuf, &inlen, pout, &outlen))
		return -1;

	if(cd > 0)
		iconv_close(cd);
	*pout = '\0';

	return 0;
}


/*
	函数名称：u2g
	函数功能：utf8转gbk2312
	传入参数：
				char *inbuf		输入字符串-utf8字符串
				size_t inlen	输入字符串长度
				char *outbuf	生成的gbk字符串数据存储到该指针指向的存储空间 
				size_t outlen	gbk字符串存储空间大小
	传出数据：0 运行正确 非0 运行失败
*/
int u2g(const char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}

/*
	函数名称：g2u
	函数功能：gbk2312转utf8
	传入参数：
				char *inbuf		输入字符串-gbk2312字符串
				size_t inlen	输入字符串长度
				char *outbuf	生成的utf8字符串数据存储到该指针指向的存储空间 
				size_t outlen	utf8字符串存储空间大小
	传出数据：0 运行正确 非0 运行失败
*/
int g2u(const char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}

#if 0
int main(void) {
	char *s = "中国";
	int fd = open("test.txt", O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
	char buf[10];
	u2g(s, strlen(s), buf, sizeof(buf));
	write(fd, buf, strlen(buf));
	close(fd);

	fd = open("test.txt2", O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
	char buf2[10];
	g2u(buf, strlen(buf), buf2, sizeof(buf2));
	write(fd, buf2, strlen(buf2));
	close(fd);
	return 1;
}
#endif
}
