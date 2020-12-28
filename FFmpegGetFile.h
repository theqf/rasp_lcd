#ifndef FFMPEGGETFILE_H
#define FFMPEGGETFILE_H

#include "TimeUtil.h"
extern "C"
{
#include <libavutil/imgutils.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
};
#include <functional>
#include <thread>
#include <iostream>
#include <sstream>
#include <chrono>

class FFmpegGetFile {
private:
    AVFormatContext * fileFormatContext;
    AVBitStreamFilterContext* bsfc;
    bool running = false;
    std::thread videoRecordThread;
    std::function<void (uint8_t*,int,bool)> h264OutFun = nullptr;  //y,yp,u,up,v,vp
private:
    void threadFun();
public:
    bool start(std::string pathName);
    void join();
    void stop();
    void SeekFrame(AVFormatContext* context,AVStream* ist, int64_t seekFrame);
    void setH264OutCallBack(std::function<void (uint8_t*,int,bool)> f);
};


#endif
