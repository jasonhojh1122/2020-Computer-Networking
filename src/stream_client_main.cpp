#include "stream_client.h"
#include <string>

int main(int argc, char **argv) {
    StreamClient stream_client(argv[1]);
    stream_client.run();

}