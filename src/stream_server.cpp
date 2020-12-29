#include "stream_server.h"

bool StreamServer::end = false;

StreamServer::StreamServer(const char *dev_name, const char *url, const char *out_format, int fps) {
    avdevice_register_all();
    avcodec_register_all();
    avformat_network_init();
    av_register_all();
    
    init_input(dev_name);

    init_display();

    init_output(out_format, url, fps);

    init_sws();

    packet = av_packet_alloc();
    if (!packet) {
        std::cerr << "[Server] Failed to allocate memory for Packet\n";
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, StreamServer::sig_handler);
    std::cout << "[Server] Server initialized\n";
}

StreamServer::~StreamServer() {
    if (out_fmt_ctx->pb)
        avio_close(out_fmt_ctx->pb);
    if (in_fmt_ctx->pb)
        avio_close(in_fmt_ctx->pb);
    if (in_fmt_ctx)
        avformat_free_context(in_fmt_ctx);
    if (out_fmt_ctx)
        avformat_free_context(out_fmt_ctx);
    if (packet)
        av_packet_free(&packet);
    if (in_frame)
        av_frame_free(&in_frame);
    if (out_frame)
        av_frame_free(&out_frame);
    if (in_codec_ctx)
        avcodec_free_context(&in_codec_ctx);
    if (out_codec_ctx)
        avcodec_free_context(&out_codec_ctx);
    if (rgb_frame_buf)
        av_free(rgb_frame_buf);
    if (sws_in_2_rgb)
        sws_freeContext(sws_in_2_rgb);
    if (out_frame_buf)
        av_free(rgb_frame_buf);
    if (sws_rgb_2_out)
        sws_freeContext(sws_rgb_2_out);
}

void StreamServer::init_input(const char *dev_name) {
    AVInputFormat *pInputFormat = av_find_input_format("v4l2");

    if (avformat_open_input(&in_fmt_ctx, dev_name, pInputFormat, NULL) != 0) {
        std::cerr << "[Server] Failed to open Web Camera.\n";
        exit(EXIT_FAILURE);
    }
    
    if (avformat_find_stream_info(in_fmt_ctx, NULL) < 0 ) {
        std::cerr << "[Server] Failed to get string info\n";
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        if (in_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1) {
        std::cerr << "[Server] Failed to find video stream input\n";
        exit(EXIT_FAILURE);
    }

    in_codec = avcodec_find_decoder(in_fmt_ctx->streams[video_stream_index]->codecpar->codec_id);
    in_codec_ctx = avcodec_alloc_context3(in_codec);
    if (!in_codec_ctx) {
        std::cerr << "[Server] Failed to allocate memory for input Codec Context\n";
        exit(EXIT_FAILURE);
    }
    
    if (avcodec_parameters_to_context(in_codec_ctx, in_fmt_ctx->streams[video_stream_index]->codecpar) < 0) {
        std::cerr << "[Server] Failed to copy input codec parameters to codec context\n";
        exit(EXIT_FAILURE);
    }

    if (avcodec_open2(in_codec_ctx, in_codec, NULL) < 0) {
        std::cerr << "[Server] Failed to open input codec\n";
        exit(EXIT_FAILURE);
    }

    in_frame = av_frame_alloc();
    if (!in_frame) {
        std::cerr << "[Server] Failed to allocate memory for input Frame\n";
        exit(EXIT_FAILURE);
    }
}

void StreamServer::init_display() {
    display_frame = av_frame_alloc();
    if (!display_frame) {
        std::cerr << "[Server] Failed to allocate out frame.\n";
        exit(EXIT_FAILURE);
    }
    display_frame->width = in_codec_ctx->width;
    display_frame->height = in_codec_ctx->height;
    display_frame->format = display_fmt;
}

void StreamServer::init_output(const char *out_format, const char *url, int fps) {
    if (avformat_alloc_output_context2(&out_fmt_ctx, NULL, out_format, NULL) != 0) {
        std::cerr << "[Server] Failed to allocate momory for output Format Context\n";
        exit(EXIT_FAILURE);
    }
    int ret = -1;
    while (ret != 0) {
        ret = avio_open2(&out_fmt_ctx->pb, url, AVIO_FLAG_WRITE, NULL, NULL);
        if (ret != 0)
            std::cerr << "[Server] Failed to open io Context\n";
    }

    out_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    out_codec_ctx = avcodec_alloc_context3(out_codec);
    if (!out_codec_ctx) {
        std::cerr << "[Server] Failed to allocate output codec context\n";
        exit(EXIT_FAILURE);
    }

    out_stream = avformat_new_stream(out_fmt_ctx, out_codec);
    if (!out_stream) {
        std::cerr << "[Server] Failed to get output stream\n";
        exit(EXIT_FAILURE);
    }

    out_codec_ctx->codec_tag = 0;
    out_codec_ctx->codec_id = AV_CODEC_ID_H264;
    out_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    out_codec_ctx->width = in_codec_ctx->width;
    out_codec_ctx->height = in_codec_ctx->height;
    out_codec_ctx->framerate = (AVRational){fps, 1};
    out_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    out_codec_ctx->time_base = (AVRational){1, fps};
    out_codec_ctx->gop_size = 10;
    out_codec_ctx->max_b_frames = 1;

    if (avcodec_parameters_from_context(out_stream->codecpar, out_codec_ctx) != 0) {
        std::cerr << "[Server] Failed to copy parameters to output codec stream.\n";
        exit(EXIT_FAILURE);
    }

    AVDictionary *codec_options = NULL;
    av_dict_set(&codec_options, "preset", "superfast", 0);
    av_dict_set(&codec_options, "tune", "zerolatency", 0);

    if (avcodec_open2(out_codec_ctx, out_codec, &codec_options) != 0) {
        std::cerr << "[Server] Failed to open encoder.\n";
        exit(EXIT_FAILURE);
    }

    out_stream->codecpar->extradata = out_codec_ctx->extradata;
    out_stream->codecpar->extradata_size = out_codec_ctx->extradata_size;

    if (avformat_write_header(out_fmt_ctx, NULL) != 0) {
        std::cerr << "[Server] Failed to write header.\n";
        exit(EXIT_FAILURE);
    }

    out_frame = av_frame_alloc();
    if (!out_frame) {
        std::cerr << "[Server] Failed to allocate out frame.\n";
        exit(EXIT_FAILURE);
    }
    out_frame->width = out_codec_ctx->width;
    out_frame->height = out_codec_ctx->height;
    out_frame->format = out_codec_ctx->pix_fmt;
}

void StreamServer::init_sws() {
    sws_in_2_rgb = sws_getContext(in_codec_ctx->width,
                                    in_codec_ctx->height,
                                    in_codec_ctx->pix_fmt,
                                    out_codec_ctx->width,
                                    out_codec_ctx->height,
                                    display_fmt,
                                    SWS_BILINEAR,
                                    NULL, NULL, NULL);

    int picture_size = av_image_get_buffer_size(display_fmt,
                                                out_codec_ctx->width,
                                                out_codec_ctx->height,
                                                1);

    rgb_frame_buf = (uint8_t*)av_malloc(picture_size);
    if (av_image_fill_arrays(display_frame->data,
                                display_frame->linesize,
                                rgb_frame_buf,
                                display_fmt,
                                out_codec_ctx->width,
                                out_codec_ctx->height,
                                1)
        < 0) {
        std::cerr << "[Server] Failed to fill in display frame buffer\n";
        exit(EXIT_FAILURE);
    }

    sws_rgb_2_out = sws_getContext(in_codec_ctx->width,
                                    in_codec_ctx->height,
                                    display_fmt,
                                    out_codec_ctx->width,
                                    out_codec_ctx->height,
                                    out_codec_ctx->pix_fmt,
                                    SWS_BILINEAR,
                                    NULL, NULL, NULL);

    picture_size = av_image_get_buffer_size(out_codec_ctx->pix_fmt,
                                            out_codec_ctx->width,
                                            out_codec_ctx->height,
                                            32);

    out_frame_buf = (uint8_t*)av_malloc(picture_size);
    if (av_image_fill_arrays(out_frame->data,
                                out_frame->linesize,
                                out_frame_buf,
                                out_codec_ctx->pix_fmt,
                                out_codec_ctx->width,
                                out_codec_ctx->height,
                                1)
        < 0) {
        std::cerr << "[Server] Failed to fill in output frame buffer\n";
        exit(EXIT_FAILURE);
    }
}

bool StreamServer::decode_input_frame() {
    if (packet->stream_index != video_stream_index)
        return false;
        
    int ret;
    ret = avcodec_send_packet(in_codec_ctx, packet);
    if (ret != 0) {
        std::cerr << "[Server] Failed to send packet to decoder.\n";
        exit(EXIT_FAILURE);
    }

    ret = avcodec_receive_frame(in_codec_ctx, in_frame);
    if (ret != 0) {
        std::cerr << "[Server] Failed to receive frame\n";
        exit(EXIT_FAILURE);
    }

    av_packet_unref(packet);
    av_init_packet(packet);
    return true;
}

void StreamServer::cv2_display_frame() {
    int ret = sws_scale(sws_in_2_rgb,
                        (const uint8_t *const *)in_frame->data,
                        in_frame->linesize,
                        0,
                        in_codec_ctx->height,
                        display_frame->data,
                        display_frame->linesize);
    if (ret <= 0) {
        std::cerr << "[Server] Failed to convert pixel format from input\n";
        exit(EXIT_FAILURE);
    }

    cv::Mat mat(display_frame->height, display_frame->width, CV_8UC3, display_frame->data[0], display_frame->linesize[0]);
    cv::imshow("Your Screen", mat);
    cv::waitKey(5);
}

bool StreamServer::encode_output_frame(int64_t pts) {
    int ret = sws_scale(sws_rgb_2_out,
                        (const uint8_t *const *)display_frame->data,
                        display_frame->linesize,
                        0,
                        in_codec_ctx->height,
                        out_frame->data,
                        out_frame->linesize);
    if (ret <= 0) {
        std::cerr << "[Server] Failed to convert pixel format from display\n";
        exit(EXIT_FAILURE);
    }

    out_frame->pts = pts;
    
    ret = avcodec_send_frame(out_codec_ctx, out_frame);
    if (ret != 0) {
        std::cerr << "[Server] Failed to send frame.\n";
        exit(EXIT_FAILURE);
    }

    ret = avcodec_receive_packet(out_codec_ctx, packet);
    if (ret == AVERROR(EAGAIN))
        return false;
    if (ret != 0) {
        std::cerr << ret << '\n';
        std::cerr << "[Server] Failed to receive packet\n";
        exit(EXIT_FAILURE);
    }
    packet->pts = av_rescale_q(packet->pts, out_codec_ctx->time_base, out_stream->time_base);
    packet->dts = av_rescale_q(packet->dts, out_codec_ctx->time_base, out_stream->time_base);
    return true;
}

void StreamServer::run() {
    int64_t pts = 0;
    while (av_read_frame(in_fmt_ctx, packet) >= 0 && !end) {
        if (decode_input_frame() == false)
            continue;        

        // display
        cv2_display_frame();
        
        // encode
        if (encode_output_frame(pts++) == false)
            continue;

        // send frame
        av_interleaved_write_frame(out_fmt_ctx, packet);

        av_packet_unref(packet);
        av_init_packet(packet);
    }
    av_write_trailer(out_fmt_ctx);
}

void StreamServer::sig_handler(int signal) {
    end = true;
}