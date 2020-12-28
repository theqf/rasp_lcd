
#ifndef SPSPARSER_H
#define SPSPARSER_H

#include <stdio.h>
#include <math.h>

class SpsParser
{


public:
    SpsParser();
    ~SpsParser();

	int parseSPSFrame(char *frameBits, int *videoWidth, int *videoHeight);

private:
	int h264_decode_seq_parameter_set(char * buf,unsigned int nLen,int &Width,int &Height);
	int u(unsigned int BitCount,char * buf,unsigned int &nStartBit);
	int Se(char *pBuff, unsigned int nLen, unsigned int &nStartBit);
	unsigned int Ue(char *pBuff, unsigned int nLen, unsigned int &nStartBit);
};

#endif


