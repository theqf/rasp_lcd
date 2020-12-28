#include "H264Decoder.h"
#include "SpsParser.h"
#include "TimeUtil.h"
using namespace std;

H264Decoder::H264Decoder() {
}

bool H264Decoder::init(bool hard, int dw, int dh) {
    m_pCodecCtx_ = nullptr;
    videoCodec = nullptr;
    outHandle = nullptr;
    dest_h = dh;
    dest_w = dw;
    av_register_all();
    if (hard) {
#ifdef MAC
        videoCodec = avcodec_find_decoder_by_name("vda");
        if (!videoCodec) {
            videoCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
        }
#endif
#ifdef RPI
        videoCodec = avcodec_find_decoder_by_name("h264_mmal");
        if (!videoCodec) {
            videoCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
        }
#endif
    } else {
        videoCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!videoCodec) {
            return false;
        }
    }

    m_pCodecCtx_ = avcodec_alloc_context3(videoCodec);
    if (!m_pCodecCtx_) {
        return false;
    }

    if (avcodec_open2(m_pCodecCtx_, videoCodec,NULL) != 0){
        return false;
    }
    return true;
}

H264Decoder::~H264Decoder() {
    if (m_pCodecCtx_) avcodec_close(m_pCodecCtx_);
    if (m_pFrameRGB) av_frame_free(&m_pFrameRGB);
    if (m_rgbBuffer) av_free(m_rgbBuffer);
    if (m_yuvBuffer) av_free(m_yuvBuffer);
    if (m_img_convert_ctx) sws_freeContext(m_img_convert_ctx);
}

void H264Decoder::videoDecode(uint8_t *buf, unsigned int size, uint32_t timestamp){
    /*SpsParser psp;
    int w,h;
    if((buf[4] & 0x1f) == 7 || (buf[4] & 0x1f) == 8){
        psp.parseSPSFrame((char*)buf,&w,&h);
        std::cout<<"w:"<<w<<"   h:"<<h<<std::endl;
    }*/

    AVPacket packet;
    av_init_packet(&packet);
    packet.data = buf;
    packet.size = size;
    AVFrame *pFrame_ = av_frame_alloc();// Allocate video frame
    if(pFrame_ == NULL){
        return;
    }
    if(0 != avcodec_send_packet(m_pCodecCtx_,&packet)){
        return;
    }

    while(0== avcodec_receive_frame( m_pCodecCtx_, pFrame_)){
        if (!has_init_sws){
            has_init_sws = true;
            init_sws(m_pCodecCtx_->width, m_pCodecCtx_->height);
        }
        avpicture_fill((AVPicture *) m_pFrameRGB, m_rgbBuffer, AV_PIX_FMT_RGB24, dest_w, dest_h);
        sws_scale(m_img_convert_ctx,
                  (uint8_t const * const *) pFrame_->data,
                  pFrame_->linesize, 0, m_pCodecCtx_->height, m_pFrameRGB->data,
                  m_pFrameRGB->linesize);

        if(RGBOutFun != nullptr){
            RGBOutFun(this->outHandle, m_rgbBuffer, dest_w, dest_h, timestamp);
        }

        if(yuvOutFun_ != nullptr){
            yuvOutFun_(this->outHandle, pFrame_->data[0], pFrame_->linesize[0], pFrame_->data[1], pFrame_->linesize[1], pFrame_->data[2],
                       pFrame_->linesize[2], m_pCodecCtx_->width, m_pCodecCtx_->height, timestamp);
        }
    }
    av_frame_free(&pFrame_);
}

void H264Decoder::init_sws(int width, int height)
{
    m_pFrameRGB = av_frame_alloc();
    int numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, dest_w, dest_h);
    m_rgbBuffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    int yuvSize = width * height * 3 / 2;
    m_yuvBuffer = (uint8_t *)av_malloc(yuvSize);
    m_img_convert_ctx = sws_getContext(width, height, AV_PIX_FMT_YUV420P, dest_w, dest_h, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
}

void H264Decoder::setYuvCallBack(std::function<void(void*, uint8_t *, int, uint8_t*,int,uint8_t*,int,int,int,uint32_t)> f)
{
    yuvOutFun_ = f;
}

void H264Decoder::setRGBCallBack(std::function<void(void*, uint8_t *, int, int, uint32_t)> f)
{
    RGBOutFun = f;
}