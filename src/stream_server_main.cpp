#include "stream_server.h"
#include <string>

int main(int argc, char **argv) {
    std::string dev_name(argv[1]);
    std::string url(argv[2]);
    std::string out_format(argv[3]);
    
    StreamServer stream_server(dev_name.c_str(), url.c_str(), out_format.c_str(), 30);
    stream_server.run();
}