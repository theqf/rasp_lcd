#ifndef H264DECODER_H_
#define H264DECODER_H_

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/pixdesc.h>
#include <libavutil/opt.h>
}
#include <string>
#include <thread>
#include <functional>
#include <mutex>
#include <sstream>

#define RPI 1


class H264Decoder {
private:
	AVCodecContext *m_pCodecCtx_ = nullptr;
	AVCodec *videoCodec = nullptr;
    AVFrame    *m_pFrameRGB = nullptr;
    uint8_t *m_rgbBuffer,*m_yuvBuffer = nullptr;
    struct SwsContext *m_img_convert_ctx = nullptr;
    bool has_init_sws = false;
    int dest_h = 0;
    int dest_w = 0;
    void *outHandle = nullptr;
    std::function<void(void*, uint8_t *, int, uint8_t*,int,uint8_t*,int,int,int,uint32_t)> yuvOutFun_ = nullptr;  //解码后数据输出函数 arg1 句柄，arg2 数据,arg3长度
    std::function<void(void*, uint8_t *, int, int, uint32_t)> RGBOutFun = nullptr;
private:
    void init_sws(int width, int height);
public:
	H264Decoder();
	virtual ~H264Decoder();

    bool init(bool hard, int dw = 240, int dh = 240);
	void videoDecode(uint8_t  *buf,unsigned int size,uint32_t timestamp);

	void setYuvCallBack(std::function<void(void*, uint8_t *, int, uint8_t*,int,uint8_t*,int,int,int,uint32_t)> f);
	void setRGBCallBack(std::function<void(void*, uint8_t *, int, int, uint32_t)> f);

};

#endif
