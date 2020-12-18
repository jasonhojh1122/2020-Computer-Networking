#include "webcam.h"
#include "stream_server.h"
#include "stream_client.h"
#include <string>

int main(int argc, char **argv) {
    /*
    Webcam webcam("/dev/video0");
    webcam.run();
    */

    // const char *dev_name, const char *url, const char *out_format, int fps
    /*
    std::string dev_name(argv[1]);
    std::string url(argv[2]);
    std::string out_format(argv[3]);
    StreamServer stream_server(dev_name.c_str(), url.c_str(), out_format.c_str(), 30);
    stream_server.run();
    */
    StreamClient stream_client(argv[1]);
    stream_client.run();

}