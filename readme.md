# Computer Networking Project

This is the final project for NTU CSIE computer networking class.
There are two parts of this project. The first part is a http web server. The second part is a video streaming app.

## Prerequisite

This project requires the following libraries:

- libcurl-dev
- libsqlite3-dev
- opencv
- libavdevice
- libavformat
- libavcodec
- libswscale
- libavutil

## Installation

After cloning the repository, create a build directory.

```
mkdir build && cd build
```
Then build the project by:
```
cmake ..
make
```

## Web server

### Usage
First, cd into the directory with executions.
```
cd ./bin
```


Before running the web server, please make sure you have the database file `project.db` in the same directory of the execution. If not, then run
```
sqlite3 ./project.db < ../../init.sql
```

Then to run the web server, use:

```
./web_main [port] ../../web
```

The web server will listen on `port` and use `../web` as the root directory of the website.

### Example

After running `./bin/web_main 8080 ../web`, go to your web browser and type `127.0.0.1:8080/index.html`, then the index page will be rendered.
Following are the pages implemented in this project:
- /index.html
  - The index page with links to other pages.
- /login
  - The login page if you are logged out.
  - If you are logged in, it will redirect you to the index page.
- /signup
  - Signup page to create a new account.
  - If you are logged in, it will redirect you to the index page.
- /bulletin
  - A bulletin for user with account to leave some message.

## Video streaming application

### Usage

To run the steraming application, use:

```
./bin/stream_main [device name] [remote ip] [remote port] [output format] [local ip] [local port]
```

And you can press Ctrl+C to stop the application.

### Example

For example you may use the following command on computer A with IP address `192.168.0.10`
```
./bin/stream_main /dev/video0 192.168.0.13 9999 flv 192.168.0.10 8888
```
And run the following command on computer B with IP address `192.168.0.13`
```
./bin/stream_main /dev/video0 192.168.0.10 8888 flv 192.168.0.10 9999
```
Then A and B may see each other in live streaming.
 