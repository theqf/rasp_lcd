#include "FFmpegGetFile.h"

using namespace std;

void FFmpegGetFile::threadFun(){
    int err = 0;
    if ((err = avformat_find_stream_info(fileFormatContext, NULL)) < 0) {
        return;
    }

    AVCodecContext *video_codec_ctx = NULL;
    int video_stream_idx = -1;

    int ret;
    AVStream *st;
    AVCodec *dec = NULL;

    ret = av_find_best_stream(fileFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        cerr << "Could not find stream in input device" << endl;
        return;
    }
    video_stream_idx = ret;
    st = fileFormatContext->streams[video_stream_idx];

    int frame_rate=st->avg_frame_rate.num/st->avg_frame_rate.den;
    int sleepTime = 1000/frame_rate;

    dec = avcodec_find_decoder(st->codecpar->codec_id);
    if (!dec) {
        cerr << "Failed to find codec" << endl;
        return ;
    }
    video_codec_ctx = avcodec_alloc_context3(dec);
    if (!video_codec_ctx) {
        cerr << "Failed to allocate the codec context" << endl;
        return ;
    }

    if ((ret = avcodec_parameters_to_context(video_codec_ctx, st->codecpar)) < 0) {
        cerr << "Failed to copy codec parameters to decoder context";
        return ;
    }

    if ((ret = avcodec_open2(video_codec_ctx, dec, NULL)) < 0) {
        cerr << "Failed to open codec" << endl;
        return;
    }


    if(video_codec_ctx->codec_id == AV_CODEC_ID_H264){
        printf("AV_CODEC_ID_H264  \n");
    }
    bsfc =  av_bitstream_filter_init("h264_mp4toannexb");

    while(running){
        AVPacket *packet = av_packet_alloc();
        while (running && av_read_frame(fileFormatContext, packet) >= 0) {
            int32_t begin = TimeUtil::currentTimeInDayMillis();
            if (packet->stream_index != video_stream_idx) {
                continue;
            }
            uint8_t *pOutBuf ;
            int pOutBufSize = 0;
            av_bitstream_filter_filter(bsfc, video_codec_ctx, NULL, &pOutBuf, &pOutBufSize, packet->data, packet->size, packet->flags & AV_PKT_FLAG_KEY);
            if(h264OutFun){
                h264OutFun(pOutBuf, pOutBufSize, packet->flags & AV_PKT_FLAG_KEY);
            }
            free(pOutBuf);
            av_packet_unref(packet);
            int32_t end = TimeUtil::currentTimeInDayMillis();
            if(end - begin < sleepTime){
                this_thread::sleep_for(chrono::milliseconds(sleepTime - (end - begin)));
            }
        }
        av_packet_free(&packet);
        SeekFrame(fileFormatContext ,st,0);
    }

    av_bitstream_filter_close(bsfc);
    avcodec_close(video_codec_ctx);
    avformat_close_input(&fileFormatContext);
}

bool FFmpegGetFile::start(string pathName){
    fileFormatContext = avformat_alloc_context();
    AVInputFormat *inputFormat = av_find_input_format("MP4");
    if (inputFormat == NULL) {
        cerr << "no input format" << endl;
        return false;
    }
    AVDictionary *avOption = NULL;

    int err = avformat_open_input(&fileFormatContext, pathName.c_str(), inputFormat, NULL);
    if (err != 0) {
        return false;
    }

    running = true;
    videoRecordThread = thread(&FFmpegGetFile::threadFun, this);
    return true;
}
void FFmpegGetFile::join(){
    if (videoRecordThread.joinable()){
        videoRecordThread.join();
    }
}
void FFmpegGetFile::stop(){
    running = false;
    if(videoRecordThread.joinable()){
        videoRecordThread.join();
    }

}
void FFmpegGetFile::SeekFrame(AVFormatContext* context,AVStream* ist, int64_t seekFrame)//跳转到指定位置
{
    int defaultStreamIndex = av_find_default_stream_index(context);
    if(ist->index == defaultStreamIndex)
    {
        auto time_base = context->streams[defaultStreamIndex]->time_base;
        auto seekTime = context->streams[defaultStreamIndex]->start_time + av_rescale(seekFrame, time_base.den, time_base.num);
        int ret ;
        if(seekTime > ist->cur_dts)
        {
            ret = av_seek_frame(context, defaultStreamIndex, seekTime, AVSEEK_FLAG_ANY);
        }
        else
        {
            ret = av_seek_frame(context, defaultStreamIndex, seekTime, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD);
        }
    }
}

void FFmpegGetFile::setH264OutCallBack(std::function<void (uint8_t*,int,bool)> f)
{
    h264OutFun = f;
}