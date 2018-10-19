#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>

int epoll_register(int events, int efd, int socket) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = socket;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, socket, &ev) == -1) {
        return 0;
    }
    return 1;
}

void epoll_monitor(int efd, int socket, int timeout) {
    struct epoll_event events[EPOLL_MAXEVENTS];
    int n = epoll_wait(efd, events, EPOLL_MAXEVENTS, timeout);
    while (n-- > 0) {
        if (events[n].events & EPOLLRDHUP) {
            printf("peer socket is closed\n");
            release_epoll(events[n].data.fd);
        } else if (events[n].events & EPOLLIN) {
            if (events[n].data.fd == socket) { //listening socket
                printf("accept_connection");
                accept_connection(socket, &register_epoll);
            } else { //get data
                accept_data(events[n].data.fd);
            }
        }
    }
}

int main(int argc, char* argv[])
 {
        if (argc != 2) {
            fprintf(stderr, "Usage: %s port\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        if ((efd = epoll_create1(EPOLL_CLOEXEC))==-1) {
            fprintf(stderr, "epoll setup is failed \n");
            exit(EXIT_FAILURE);
        }
        int socket = setup_server_socket(argv[1]);
        if (!epoll_register(EPOLLIN|EPOLLRDHUP|EPOLLET, efd, socket)) {
                fprintf(stderr, "add server socket to epoll is failed");
                exit(EXIT_FAILURE);
        }
        prepare_welcome_msg();
        int timeout = 0;
        while(!to_quit) {
            timeout = g_queue_is_empty(&in_queue) && g_queue_is_empty(&out_queue) ? 1000:0;
            epoll_monitor(efd, socket, timeout);
            handle_in_queue();
            handle_out_queue();
        }
        close(socket);
        close(efd);
        close_queue();
        printf("exit!\n");
}
