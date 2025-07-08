#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "request.h"
#include "io_helper.h"

char default_root[] = ".";

// Shared buffer and synchronization
int *buffer = NULL;
int buffer_head = 0, buffer_tail = 0, buffer_count = 0;
int buffer_size = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_nonempty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_nonfull = PTHREAD_COND_INITIALIZER;

void put_request(int conn_fd) {
	pthread_mutex_lock(&lock);
	while (buffer_count == buffer_size)
		pthread_cond_wait(&cond_nonfull, &lock);

	buffer[buffer_tail] = conn_fd;
	buffer_tail = (buffer_tail + 1) % buffer_size;
	buffer_count++;

	pthread_cond_signal(&cond_nonempty);
	pthread_mutex_unlock(&lock);
}

int get_request() {
	pthread_mutex_lock(&lock);
	while (buffer_count == 0)
		pthread_cond_wait(&cond_nonempty, &lock);

	int conn_fd = buffer[buffer_head];
	buffer_head = (buffer_head + 1) % buffer_size;
	buffer_count--;

	pthread_cond_signal(&cond_nonfull);
	pthread_mutex_unlock(&lock);
	return conn_fd;
}

void *worker_thread(void *arg) {
	while (1) {
		int conn_fd = get_request();
		request_handle(conn_fd);
		close_or_die(conn_fd);

	}
	return NULL;

}


//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
    int thread_count = 1;
    buffer_size = 1;
    char *schedalg = "FIFO";

    while ((c = getopt(argc, argv, "d:p:t:b:s:")) != -1) {
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
	case 't':
	    thread_count = atoi(optarg);
	    break;
	case 'b':
	    buffer_size = atoi(optarg);
	    break;
	case 's':
	    schedalg = optarg;
	    break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg]\n");
	    exit(1);
	}
}

    // Validate thread and buffer inputs
    if (thread_count <= 0) {
	fprintf(stderr, "Invalid thread count: must be greater than 0\n");
	exit(1);
    }

    if (buffer_size <= 0) {
	fprintf(stderr, "Invalid buffer size: must be greater than 0\n");
	exit(1);
    }

    // Allocate buffer dynamically
    buffer = malloc(sizeof(int) * buffer_size);
    if (buffer == NULL) {
	fprintf(stderr, "Failed to allocate buffer\n");
	exit(1);
    }

    // run out of this directory
    chdir_or_die(root_dir);

    // Create a thread pool
    pthread_t tids[thread_count];
    for (int i = 0; i < thread_count; i++) {
    	pthread_create(&tids[i], NULL, worker_thread, NULL);

    }

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
	struct sockaddr_in client_addr;
	int client_len = sizeof(client_addr);
	int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
	put_request(conn_fd);

    }
    free(buffer);
    return 0;
}
