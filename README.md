# Multi-threaded Web Server (CS360 Assignment 2)

This is a multi-threaded web server written in C for the CS360 Operating Systems course. It extends a single-threaded web server to handle concurrent requests using a thread pool and bounded buffer, based on the producer-consumer model.

## Features

- Handles multiple concurrent HTTP requests
- Thread pool (configurable via `-t` flag)
- Shared bounded buffer (configured via `-b` flag)
- Implements `GET` requests for static and CGI content
- Graceful synchronization using mutex and condition variables

## Compilation

Navigate to the `src/` folder and run:

```bash
make clean
make
```

## Usage

```bash
./wserver -d <html_folder> -p <port> -t <num_threads> -b <buffer_size>
```

### Example:

```bash
./wserver -d ~/htmlfolder -p 10000 -t 4 -b 8
```

### Then open your browser and visit:

```bash
[./wserver -d ~/htmlfolder -p 10000 -t 4 -b 8
](http://localhost:10000/index.html
http://localhost:10000/products.html)
```

## Testing
```bash
./wclient localhost 10000 /index.html
```

## Threading Details

- Producer: main thread accepts connections and inserts file descriptors into a circular buffer.
- Consumers: worker threads remove descriptors from the buffer and handle requests using request_handle().

## Concepts Practiced

- POSIX threads (pthread)
- Mutexes and condition variables
- Bounded buffer / producer-consumer pattern
- Network socket programming
- CGI and static content handling
