#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "stream_client.h"
#include "stream_server.h"

int main(int argc, char **argv) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " [device name] [target ip] [target port] [output format] [listen port]\n";
        exit(EXIT_FAILURE);
    }
    std::string dev_name(argv[1]);
    std::string server_url = "tcp://" + std::string(argv[2]) + ":" + std::string(argv[3]);
    std::string output_format(argv[4]);
    std::string client_url = "tcp://127.0.0.1:" + std::string(argv[5]);

    pid_t pid;
    pid = fork();
    if (pid == 0) {
        StreamClient stream_client(client_url.c_str());
        stream_client.run();
    }
    sleep(1);
    StreamServer stream_server(dev_name.c_str(), server_url.c_str(), output_format.c_str(), 30);
    stream_server.run();

    waitpid(-1, NULL, 0);    
    return 0;
}