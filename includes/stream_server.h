#ifndef _StreamServer_H
#define _StreamServer_H
#include <unistd.h>
#include <signal.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <exception>

class StreamServer {
public:
    StreamServer(const char *dev_name, const char *url, const char *out_format, int fps);
    ~StreamServer();

    void run();

private:
    AVFormatContext     *in_fmt_ctx = NULL;
    AVCodecContext      *in_codec_ctx = NULL;
    AVCodec             *in_codec = NULL;
    AVFrame             *in_frame = NULL;

    AVFormatContext     *out_fmt_ctx = NULL;
    AVCodecContext      *out_codec_ctx = NULL;
    AVCodec             *out_codec = NULL;
    AVFrame             *out_frame = NULL;
    AVStream            *out_stream = NULL;
    
    AVPacket            *packet = NULL;

    struct SwsContext   *pSwsContext = NULL;
    uint8_t             *pRGBFrameBuf = NULL;

    int video_stream_index = -1;

    static bool end;

    void init_input(const char *dev_name);
    void init_output(const char *out_format, const char *url, int fps);
    void init_sws();

    static void sig_handler(int signal);
};

#endif