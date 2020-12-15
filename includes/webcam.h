#ifndef _WEBCAM_H
#define _WEBCAM_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <string>
#include <iostream>
#include <exception>

class Webcam {
public:
    Webcam(std::string& dev_name);
    ~Webcam();

    bool readVideoFrame();
    void run();

private:
    AVFormatContext     *pFormatContext = NULL;
    AVCodecContext      *pCodecContext = NULL;
    AVCodecParameters   *pCodecParameters = NULL;
    AVCodec             *pCodec = NULL;
    AVFrame             *pFrame = NULL;
    AVFrame             *pRGBFrame = NULL;
    AVPacket            *pPacket = NULL;

    struct SwsContext   *pSwsContext = NULL;
    uint8_t             *pRGBFrameBuf = NULL;

    int video_stream_index = -1;

    void decodePacket();

};

#endif