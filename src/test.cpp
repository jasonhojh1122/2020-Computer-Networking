#include "webcam.h"
#include <string>

int main() {
    std::string cam_name = "/dev/video0";
    Webcam webcam(cam_name);
    while(webcam.readVideoFrame()) {
        continue;
    }
}