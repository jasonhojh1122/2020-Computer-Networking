#include "webcam.h"

Webcam::Webcam(std::string& dev_name) {
    avdevice_register_all();
    avcodec_register_all();
    av_register_all();

    AVInputFormat *pInputFormat = av_find_input_format("v4l2");

    pFormatContext = avformat_alloc_context();
    if (!pFormatContext) {
        std::cerr << "Failed to allocate memory for Format Context\n";
        exit(EXIT_FAILURE);
    }

    if (avformat_open_input(&pFormatContext, dev_name.c_str(), pInputFormat, NULL) != 0) {
        std::cerr << "Failed to open Web Camera.\n";
        exit(EXIT_FAILURE);
    }

    if (avformat_find_stream_info(pFormatContext, NULL) < 0 ) {
        std::cerr << "Failed to get string info\n";
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < pFormatContext->nb_streams; i++) {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            pCodecParameters = pFormatContext->streams[i]->codecpar;            
        }
    }

    if (video_stream_index == -1) {
        std::cerr << "Failed to find video stream input\n";
        exit(EXIT_FAILURE);
    }

    pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext) {
        std::cerr << "Failed to allocate memory for Codec Context\n";
        exit(EXIT_FAILURE);
    }

    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
        std::cerr << "Failed to copy codec parameters to codec context\n";
        exit(EXIT_FAILURE);
    }

    if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
        std::cerr << "Failed to open codec\n";
        exit(EXIT_FAILURE);
    }

    pFrame = av_frame_alloc();
    pRGBFrame = av_frame_alloc();
    if (!pFrame || !pRGBFrame) {
        std::cerr << "Failed to allocate memory for Frame\n";
        exit(EXIT_FAILURE);
    }

    pPacket = av_packet_alloc();
    if (!pPacket) {
        std::cerr << "Failed to allocate memory for Packet\n";
        exit(EXIT_FAILURE);
    }

    pSwsContext = sws_alloc_context();
    if (!pSwsContext) {
        std::cerr << "Failed to allocate memory for pSwsContext\n";
        exit(EXIT_FAILURE);
    }

    if (sws_init_context(pSwsContext, NULL, NULL) < 0 ) {
        std::cerr << "Failed to initialize pSwsContext\n";
        exit(EXIT_FAILURE);
    }
    
    pSwsContext = sws_getContext(pCodecContext->width,
                                    pCodecContext->height,
                                    pCodecContext->pix_fmt,
                                    pCodecContext->width,
                                    pCodecContext->height,
                                    AV_PIX_FMT_BGR24,
                                    SWS_BICUBIC,
                                    NULL, NULL, NULL);
    if (!pSwsContext) {
        std::cerr << "Failed to get SwsContext\n";
        exit(EXIT_FAILURE);
    }

    int picture_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, pCodecContext->width, pCodecContext->height, 1);
    pRGBFrameBuf = (uint8_t*)av_malloc(picture_size);
    if (av_image_fill_arrays(pRGBFrame->data,
                                pRGBFrame->linesize,
                                pRGBFrameBuf,
                                AV_PIX_FMT_BGR24,
                                pCodecContext->width,
                                pCodecContext->height,
                                1)
        < 0) {
        std::cerr << "Failed to fill in RGBFrame buffer\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Resolution " << pCodecParameters->width << "x" << pCodecParameters->height << '\n';
    std::cout << "Codec: " << pCodec->name << '\n';
}

Webcam::~Webcam() {
    avformat_close_input(&pFormatContext);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&pRGBFrame);
    avcodec_free_context(&pCodecContext);
    av_free(pRGBFrameBuf);
    sws_freeContext(pSwsContext);
}

bool Webcam::readVideoFrame() {
    if (av_read_frame(pFormatContext, pPacket) < 0)
        return false;
    if (pPacket->stream_index != video_stream_index)
        return false;
    decodePacket();
    av_packet_unref(pPacket);
}

void Webcam::run() {
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        if (pPacket->stream_index == video_stream_index) {
            decodePacket();
        }
    }
}

void Webcam::decodePacket() {
    int ret;
    ret = avcodec_send_packet(pCodecContext, pPacket);
    if (ret < 0) {
        std::cerr << "Failed to send Packet for decoding\n";
        exit(EXIT_FAILURE);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(pCodecContext, pFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            std::cerr << "Failed to decode Packet\n";
            exit(EXIT_FAILURE);
        }
        std::cout << "frame received\n";
        sws_scale(pSwsContext,
                    pFrame->data,
                    pFrame->linesize,
                    pCodecContext->width,
                    pCodecContext->height,
                    pRGBFrame->data,
                    pRGBFrame->linesize);
        std::cout << "scaled\n";
        cv::Mat mat(pCodecContext->height, pCodecContext->width, CV_8UC3, pRGBFrame->data[0], pRGBFrame->linesize[0]);
        cv::imshow("frame", mat);
        cv::waitKey(0);
    }
}