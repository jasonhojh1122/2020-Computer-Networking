# Computer Networking Project

This is the final project for NTU CSIE computer networking class.

In phase 1, a simple HTTP server is implemented.

## Installation

To compile the phase 1 version, please use:

```
make stable
```

An executable named *main* will be generated.

## Usage

To run the phase 1 code, please use:

```
./main [port] [root]
```

The web server will listen on `port` and serve `root` as the root directory of the website.


## Example

Some simple web pages are saved under `./web` in this project.

Run`./main 8080 ./web` and connect to `127.0.0.1/index.html` in your web browser to see the index page of the demo.

Connect to `127.0.0.1/large.html` to test for some large images.

## Demo 

A demo server is hosted on `seekrealthing.net`.
The server is located at the east-coast of America, so there may be some delay.