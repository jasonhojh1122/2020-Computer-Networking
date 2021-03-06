#include "stream_client.h"

bool StreamClient::end = false;

StreamClient::StreamClient(const char *url) {
    avdevice_register_all();
    avcodec_register_all();
    avformat_network_init();
    av_register_all();

    std::stringstream ss;
    ss << url << "?listen=1" << "?listen_timeout=" << TIMEOUT << "?timeout=" << TIMEOUT * 1000;
    
    std::cout << "[Client] Waiting for server output.\n";

    std::cout << "[Client] " << ss.str() << std::endl;

    if (avformat_open_input(&pFormatContext, ss.str().c_str(), NULL, NULL) != 0) {
        std::cerr << "[Client] Failed to open url.\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "[Client] Receive video stream.\n";
    
    if (avformat_find_stream_info(pFormatContext, NULL) < 0 ) {
        std::cerr << "[Client] Failed to get string info\n";
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < pFormatContext->nb_streams; i++) {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            pCodecParameters = pFormatContext->streams[i]->codecpar;
        }
    }

    if (video_stream_index == -1) {
        std::cerr << "[Client] Failed to find video stream input\n";
        exit(EXIT_FAILURE);
    }

    pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext) {
        std::cerr << "[Client] Failed to allocate memory for Codec Context\n";
        exit(EXIT_FAILURE);
    }
    
    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
        std::cerr << "[Client] Failed to copy codec parameters to codec context\n";
        exit(EXIT_FAILURE);
    }

    if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
        std::cerr << "[Client] Failed to open codec\n";
        exit(EXIT_FAILURE);
    }

    pFrame = av_frame_alloc();
    pRGBFrame = av_frame_alloc();
    if (!pFrame || !pRGBFrame) {
        std::cerr << "[Client] Failed to allocate memory for Frame\n";
        exit(EXIT_FAILURE);
    }

    pPacket = av_packet_alloc();
    if (!pPacket) {
        std::cerr << "[Client] Failed to allocate memory for Packet\n";
        exit(EXIT_FAILURE);
    }

    pSwsContext = sws_getContext(pCodecContext->width,
                                    pCodecContext->height,
                                    pCodecContext->pix_fmt,
                                    pCodecContext->width,
                                    pCodecContext->height,
                                    dst_fmt,
                                    SWS_BICUBIC,
                                    NULL, NULL, NULL);

    int picture_size = av_image_get_buffer_size(dst_fmt, pCodecContext->width, pCodecContext->height, 16);

    pRGBFrameBuf = (uint8_t*)av_malloc(picture_size);
    if (av_image_fill_arrays(pRGBFrame->data,
                                pRGBFrame->linesize,
                                pRGBFrameBuf,
                                dst_fmt,
                                pCodecContext->width,
                                pCodecContext->height,
                                1)
        < 0) {
        std::cerr << "[Client] Failed to fill in RGBFrame buffer\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "[Client] Codec: " << pCodec->name << '\n';
    std::cout << "[Client] Resolution: " << pCodecParameters->width << " x " << pCodecParameters->height << '\n';
}

StreamClient::~StreamClient() {
    avformat_close_input(&pFormatContext);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&pRGBFrame);
    avcodec_free_context(&pCodecContext);
    av_free(pRGBFrameBuf);
    sws_freeContext(pSwsContext);
}

AVPixelFormat StreamClient::getPixelFormat() {
    return pCodecContext->pix_fmt;
}

int StreamClient::getHeight() {
    return pCodecParameters->height;
}

int StreamClient::getWidth() {
    return pCodecContext->width;
}

void StreamClient::run() {
    int a = 0;
    while (av_read_frame(pFormatContext, pPacket) >= 0 && ~end) {
        if (pPacket->stream_index == video_stream_index) {
            decodePacket();
        }
    }
}

void StreamClient::decodePacket() {
    int ret;
    ret = avcodec_send_packet(pCodecContext, pPacket);
    if (ret < 0) {
        std::cerr << "[Client] Failed to send Packet for decoding\n";
        exit(EXIT_FAILURE);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(pCodecContext, pFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            std::cerr << "[Client] Failed to decode Packet\n";
            exit(EXIT_FAILURE);
        }
        
        int a = sws_scale(pSwsContext,
                    (const uint8_t *const *)pFrame->data,
                    pFrame->linesize,
                    0,
                    pCodecContext->height,
                    pRGBFrame->data,
                    pRGBFrame->linesize);

        if (a <= 0) {
            std::cerr << "[Client] Failed to convert pixel format\n";
            exit(EXIT_FAILURE);
        }

        cv::Mat mat(pCodecContext->height, pCodecContext->width, CV_8UC3, pRGBFrame->data[0], pRGBFrame->linesize[0]);
        cv::imshow("Target Screen", mat);
        cv::waitKey(5);
    }
}

void StreamClient::sig_handler(int signal) {
    end = true;
}