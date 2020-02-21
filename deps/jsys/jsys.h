#ifndef JSYS_H
#define JSYS_H

#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/timerfd.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <termios.h>
#include <sys/signalfd.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <sys/resource.h> /* getrusage */
#include <signal.h>

#define JSYS_VERSION "v0.0.2"
#define JSYS_READABLE (EPOLLIN | EPOLLET)
#define JSYS_WRITABLE EPOLLOUT

enum jsys_descriptor_types {
  JSYS_UNKNOWN = 0,
  JSYS_SOCKET = 1,
  JSYS_TTY = 2,
  JSYS_SIGNAL = 3,
  JSYS_TIMER = 4,
  JSYS_UDP = 5
};

typedef struct jsys_descriptor jsys_descriptor;
typedef struct jsys_loop jsys_loop;
typedef struct jsys_process jsys_process;
typedef struct jsys_stream_context jsys_stream_context;
typedef struct jsys_stream_settings jsys_stream_settings;

typedef int    (*jsys_stream_on_end) (jsys_descriptor*);
typedef int    (*jsys_stream_on_data)(jsys_descriptor*, size_t);
typedef int    (*jsys_stream_on_connect)(jsys_descriptor*);
typedef void   (jsys_free)(void*, const char*);
typedef void*  (jsys_alloc)(size_t, size_t, const char*);
typedef int    (jsys_callback)(jsys_descriptor *);

//TODO: data structure to allow descriptor list to grow and shrink
struct jsys_loop {
  int fd;
  int state;
  jsys_callback *callback;
  jsys_callback *onidle;
  jsys_descriptor** descriptors;
  struct epoll_event *events;
  sigset_t set;
  size_t max_events;
  int maxfds;
  int count;
  jsys_free *free;
  jsys_alloc *alloc;
};

struct jsys_process {
  int pid;
  int status;
  jsys_callback *on_exit;
  const char* file;   /* Path to program to execute. */
  char** args;
  char** env;
  const char* cwd;
  unsigned int flags;
  int stdio_count;
  jsys_descriptor* stdio;
  uid_t uid;
  gid_t gid;
};

struct jsys_descriptor {
  int fd;
  int type;
  uint32_t closed;
  uint32_t events;
  jsys_loop *loop;
  struct epoll_event* event;
  jsys_callback *callback;
  uint8_t closing;
  void* data;
  void* ptr;
};

struct jsys_stream_settings {
  jsys_stream_on_data on_data;
  jsys_stream_on_connect on_connect;
  jsys_stream_on_end on_end;
  void* data;
  int buffers;
};

struct jsys_stream_context {
  void* data;
  size_t offset;
  struct iovec* in;
  struct iovec* out;
  jsys_loop* loop;
  jsys_stream_settings* settings;
  int current_buffer;
  int buffers;
  int padding;
};

static const char* jsys_version_string();

// process
static int jsys_spawn(jsys_loop*, jsys_process*, int*);

// descriptors
static jsys_descriptor* jsys_descriptor_create(jsys_loop*);
static int jsys_descriptor_is_readable(jsys_descriptor *descriptor);
static int jsys_descriptor_is_writable(jsys_descriptor *descriptor);
static int jsys_descriptor_is_error(jsys_descriptor *descriptor);
static int jsys_descriptor_free(jsys_descriptor *descriptor);
static int jsys_descriptor_non_blocking(jsys_descriptor *descriptor);
static const char* jsys_descriptor_name(jsys_descriptor* descriptor);

// loop
static jsys_loop* jsys_loop_create(int, size_t, int);
static jsys_loop* jsys_loop_create_with_allocator(int, size_t, int, jsys_free*, jsys_alloc*);
static int jsys_loop_init(jsys_loop *loop, int flags, size_t max_events, int);
static int jsys_loop_run(jsys_loop *loop);
static int jsys_loop_run_once(jsys_loop *loop, int timeout);
static int jsys_loop_stop(jsys_loop *loop);
static int jsys_loop_free(jsys_loop *loop);
static int jsys_loop_add_flags(jsys_loop *loop, jsys_descriptor *descriptor, uint32_t flags);
static int jsys_loop_mod_flags(jsys_descriptor *descriptor, uint32_t flags);
static int jsys_loop_add(jsys_loop *loop, jsys_descriptor *descriptor);
static int jsys_loop_remove(jsys_loop *loop, jsys_descriptor *descriptor);

// timers
static jsys_descriptor* jsys_timer_create(jsys_loop*, uint64_t t0, uint64_t ti);
static int jsys_timer_init(jsys_descriptor *timer, uint64_t t0, uint64_t ti);
static uint64_t jsys_timer_read(jsys_descriptor *timer);
static int jsys_timer_reset(jsys_descriptor *timer, uint64_t t0, uint64_t ti);

// stream
static jsys_stream_context* jsys_stream_context_create(jsys_loop* loop, int buffers);

// sockets
static jsys_descriptor* jsys_sock_create(jsys_loop*, int domain, int type);

// tcp
static int jsys_socket_option(jsys_descriptor* sock, int level, int optname, int val);

static int jsys_tcp_connect(jsys_descriptor *sock, uint16_t port, char* address);
static int jsys_tcp_set_nodelay(jsys_descriptor *client, int on);
static int jsys_tcp_bind_reuse(jsys_descriptor *sock, uint16_t port, in_addr_t address);
static int jsys_tcp_listen(jsys_descriptor *sock, int backlog);

static int jsys_tcp_bind(jsys_descriptor *sock, uint16_t port, in_addr_t address, int, int);
static int jsys_tcp_accept(jsys_descriptor *server, jsys_descriptor *client);
static int jsys_tcp_pause(jsys_descriptor *sock);
static int jsys_tcp_resume(jsys_descriptor *sock);
static int jsys_tcp_write(jsys_descriptor *client, struct iovec* iov);
static int jsys_tcp_write_len(jsys_descriptor *client, struct iovec* iov, int len);
static int jsys_tcp_writev(jsys_descriptor *client, struct iovec* iov, int records);
static int jsys_tcp_shutdown(jsys_descriptor* client);
static int jsys_tcp_on_server_event(jsys_descriptor *server);
static int jsys_tcp_on_client_event(jsys_descriptor *client);
static int jsys_tcp_on_accept_event(jsys_descriptor *server);

// tty
static jsys_descriptor* jsys_tty_create(jsys_loop*, int);
static int jsys_tty_init(jsys_descriptor *tty, int);

// signals
static int jsys_signal_add(jsys_loop* loop, int signum);
static jsys_descriptor* jsys_signal_watcher_create(jsys_loop* loop);

// utilities
static int jsys_dump_error(jsys_loop* loop, const char* message, int r);
static void jsysfree(void*, const char*);
static void* jsysalloc(size_t elements, size_t size, const char*);
static struct epoll_event* epoll_event_create(jsys_loop* loop, int amount);
static int jsys_usage(struct rusage*);
static void jsys_reset_signals();
static void jsys_print_sigset(FILE *of, const char *prefix, const sigset_t *sigset);
static int jsys_print_sigmask(FILE *of, const char *msg);
static int jsys_ip4_addr(const char* ip, int port, struct sockaddr_in* addr);

static int jsys_udp_bind(jsys_descriptor *sock, uint16_t port, char* address, int reuse_address, int reuse_port);
static int jsys_udp_bind_reuse(jsys_descriptor *sock, uint16_t port, char* address);

static int jsys_udp_send_len(jsys_descriptor *client, struct iovec* iov, char* addr, int port, int len);
static int jsys_udp_send(jsys_descriptor *client, struct iovec* iov, char* addr, int port);

int jsys_udp_bind_reuse(jsys_descriptor *sock, uint16_t port, char* address) {
  return jsys_udp_bind(sock, port, address, 1, 1);
}

int jsys_udp_bind(jsys_descriptor *sock, uint16_t port, char* address, int reuse_address, int reuse_port) {
  struct sockaddr_in server_addr;
  // todo: pass in flags so these can be optional. or just expose setsockopt?
  if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(int)) == -1) return -1;
  if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(int)) == -1) return -1;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_aton(address, &server_addr.sin_addr);
  bzero(&(server_addr.sin_zero), 8);
  if (bind(sock->fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) return -1;
  return 0;
}

int jsys_ip4_addr(const char* ip, int port, struct sockaddr_in* addr) {
  memset(addr, 0, sizeof(*addr));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  return inet_pton(AF_INET, ip, &(addr->sin_addr.s_addr));
}

void jsys_reset_signals() {
  // resets all signals to defaults and ignores SIGPIPE and SIGXFSZ
  struct sigaction act;
  memset(&act, 0, sizeof(act));
  for (unsigned nr = 1; nr < 32; nr += 1) {
    if (nr == SIGKILL || nr == SIGSTOP)
      continue;
    act.sa_handler = (nr == SIGPIPE || nr == SIGXFSZ) ? SIG_IGN : SIG_DFL;
    int r = sigaction(nr, &act, NULL);
    if (r != 0) {
      fprintf(stderr, "failed resetting signal\n");
      exit(1);
    }
  }
}

void jsys_print_sigset(FILE *of, const char *prefix, const sigset_t *sigset) {
    int sig, cnt;
    cnt = 0;
    for (sig = 1; sig < NSIG; sig++) {
        if (sigismember(sigset, sig)) {
            cnt++;
            fprintf(of, "%s%d (%s)\n", prefix, sig, strsignal(sig));
        }
    }
    if (cnt == 0)
        fprintf(of, "%s<empty signal set>\n", prefix);
}

int jsys_print_sigmask(FILE *of, const char *msg) {
    sigset_t currMask;
    if (msg != NULL)
        fprintf(of, "%s", msg);
    if (pthread_sigmask(SIG_BLOCK, NULL, &currMask) == -1)
        return -1;
    jsys_print_sigset(of, "\t\t", &currMask);
    return 0;
}

const char* jsys_version_string() {
  return JSYS_VERSION;
}

inline void* jsysalloc(size_t elements, size_t size, const char* tag) {
#if TRACE
  fprintf(stderr, "%s : alloc %lu elements of %lu\n", tag, elements, size);
#endif
  return calloc(elements, size);
}

inline void jsysfree(void* ptr, const char* tag) {
#if TRACE
  fprintf(stderr, "%s : free\n", tag);
#endif
  free(ptr);
  return;
}

int jsys_spawn(jsys_loop* loop, jsys_process* process, int* fds) {
  pid_t pid = fork();
  if (pid == -1) {
    perror("error forking");
    return pid;
  }
  if (pid == 0) {
    close(0);
    close(1);
    close(2);
    dup2(fds[0], 0);
    dup2(fds[1], 1);
    dup2(fds[2], 2);
    execvp(process->file, process->args);
    perror("error launching child process");
    exit(127);
  } else {
    process->pid = pid;
    process->status = 0;
    close(fds[0]);
    close(fds[1]);
    close(fds[2]);
  }
  return 0;
}

// descriptors implementation
jsys_descriptor* jsys_descriptor_create(jsys_loop* loop) {
  jsys_descriptor* descriptor = (jsys_descriptor*)loop->alloc(1, sizeof(jsys_descriptor), "jsys_descriptor");
  descriptor->callback = NULL;
  descriptor->closed = 0;
  descriptor->loop = NULL;
  descriptor->closing = 0;
  descriptor->data = NULL;
  descriptor->event = 0;
  descriptor->fd = 0;
  descriptor->type = JSYS_UNKNOWN;
  descriptor->events = 0;
  return descriptor;
}

int jsys_descriptor_is_readable(jsys_descriptor *descriptor) {
  return descriptor->events & EPOLLIN;
}

int jsys_descriptor_is_writable(jsys_descriptor *descriptor) {
  return descriptor->events & EPOLLOUT;
}

int jsys_descriptor_is_error(jsys_descriptor *descriptor) {
  return descriptor->events & EPOLLERR || descriptor->events & EPOLLHUP;
}

int jsys_descriptor_free(jsys_descriptor *descriptor) {
  int fd = descriptor->fd;
  int r = jsys_loop_remove(descriptor->loop, descriptor);
  if (r == -1) return r;
  r = close(descriptor->fd);
  if (r == -1) return r;
  memset((void*)descriptor, 0, sizeof(descriptor));
  descriptor->loop->free(descriptor, "jsys_descriptor");
  return fd;
}

int jsys_descriptor_non_blocking(jsys_descriptor* client) {
  int flags = fcntl(client->fd, F_GETFL, 0);
  if (flags == -1) return -1;
  flags |= O_NONBLOCK;
  if (fcntl(client->fd, F_SETFL, flags) == -1) return -1;
  return 0;
}

const char* jsys_descriptor_name(jsys_descriptor* descriptor) {
  if (descriptor->type == JSYS_SOCKET) return "socket";
  if (descriptor->type == JSYS_TTY) return "tty";
  if (descriptor->type == JSYS_SIGNAL) return "signal";
  if (descriptor->type == JSYS_TIMER) return "timer";
  return "unknown";
}

// loop implementation
// todo
jsys_loop* jsys_loop_create(int flags, size_t max_events, int maxfds) {
  return jsys_loop_create_with_allocator(flags, max_events, maxfds, jsysfree, jsysalloc);
}

jsys_loop* jsys_loop_create_with_allocator(int flags, size_t max_events, int maxfds, jsys_free *free, jsys_alloc *alloc) {
  jsys_loop* loop = (jsys_loop*)alloc(1, sizeof(jsys_loop), "jsys_loop");
  loop->count = 0;
  loop->callback = NULL;
  loop->events = 0;
  loop->fd = 0;
  loop->max_events = 0;
  loop->maxfds = 0;
  loop->state = 0;
  loop->alloc = alloc;
  loop->free = free;
  sigemptyset(&loop->set);
  //sigprocmask(SIG_SETMASK, &loop->set, NULL);
  loop->descriptors = (jsys_descriptor**)alloc(maxfds, sizeof(jsys_descriptor*), "descriptors_array");
  int r = jsys_loop_init(loop, flags, max_events, maxfds);
  if (r == -1) return NULL;
  return loop;
}

int jsys_loop_init(jsys_loop *loop, int flags, size_t max_events, int maxfds) {
  loop->fd = epoll_create1(flags);
  loop->state = 0;
  loop->max_events = max_events;
  loop->callback = NULL;
  loop->maxfds = maxfds;
  if (loop->fd == -1) return loop->fd;
  for (int i = 0; i < loop->maxfds; i++) {
    loop->descriptors[i] = NULL;
  }
  loop->events = epoll_event_create(loop, loop->max_events);
  return 0;
}

int jsys_loop_run(jsys_loop *loop) {
  int r = 0;
  loop->state = 1;
  while (loop->state && loop->count > 0) {
    r = jsys_loop_run_once(loop, -1); // todo: this will block forever
    if (r < 0) break;
  }
  return r;
}

int jsys_loop_run_once(jsys_loop *loop, int timeout) {
  struct epoll_event *events = loop->events;
  if (loop->count == 0) return -1;
  int r = epoll_pwait(loop->fd, events, (int)loop->max_events, timeout, &loop->set);
  if (r < 0) {
    fprintf(stderr, "epoll_pwait: %i\n", r);
    return r;
  }
  if (r == 0) return r;
  for (int i = 0; i < r; i++) {
    int fd = events[i].data.fd;
    if (fd > (loop->maxfds - 1) || fd < 0) {
      fprintf(stderr, "big fd\n");
      continue;
    }
    jsys_descriptor *descriptor = loop->descriptors[fd];
    if (descriptor != NULL) {
      if (descriptor->closed != 1) {
        descriptor->events = events[i].events;
        if (descriptor->callback != NULL) {
          // todo: check RC
          descriptor->callback(descriptor);
        } else {
          fprintf(stderr, "no callback: %i\n", fd);
        }
      } else if (descriptor->closed == 1) {
        fprintf(stderr, "descriptor closed: %i\n", fd);
        jsys_loop_remove(descriptor->loop, descriptor);
      } else {
        fprintf(stderr, "unknown descriptor status: %i\n", descriptor->closed);
        jsys_loop_remove(descriptor->loop, descriptor);
      }
    } else {
      fprintf(stderr, "descriptor not found: %i\n", fd);
      jsys_loop_remove(descriptor->loop, descriptor);
    }
  }
  return r;
}

int jsys_loop_stop(jsys_loop *loop) {
  loop->state = 0;
  return 0;
}

int jsys_usage(struct rusage* usage) {
  if (getrusage(RUSAGE_SELF, usage))
    return errno;
  return 0;
}

int jsys_loop_free(jsys_loop *loop) {
  for (int i = 0; i < loop->maxfds; i++) {
    if (loop->descriptors[i] != NULL) {
      const char* descriptor_type = jsys_descriptor_name(loop->descriptors[i]);
      int fd = loop->descriptors[i]->fd;
      int r = jsys_descriptor_free(loop->descriptors[i]);
      if (r == -1) continue;
      fprintf(stderr, "free %s (%i): %i\n", descriptor_type, fd, r);
    }
  }
  loop->free(loop->events, "epoll_event");
  loop->free(loop->descriptors, "descriptors_array");
  loop->free(loop, "jsys_loop");
  return 0;
}

int jsys_loop_add_flags(jsys_loop *loop, jsys_descriptor *descriptor, uint32_t flags) {
  struct epoll_event *event = epoll_event_create(loop, 1);
  loop->descriptors[descriptor->fd] = descriptor;
  event->data.fd = descriptor->fd;
  event->events = flags;
  descriptor->event = event;
  descriptor->events = flags;
  descriptor->loop = loop;
  loop->count++;
  return epoll_ctl(loop->fd, EPOLL_CTL_ADD, descriptor->fd, event);
}

struct epoll_event* epoll_event_create(jsys_loop* loop, int amount) {
  return (struct epoll_event *)loop->alloc(amount, sizeof(struct epoll_event), "epoll_event");
}

int jsys_loop_mod_flags(jsys_descriptor *descriptor, uint32_t flags) {
  // todo: this leaks?
  struct epoll_event *event = epoll_event_create(descriptor->loop, 1);
  event->events = flags;
  event->data.fd = descriptor->fd;
  descriptor->events = flags;
  descriptor->loop->free(descriptor->event, "epoll_event");
  descriptor->event = event;
  return epoll_ctl(descriptor->loop->fd, EPOLL_CTL_MOD, descriptor->fd, event);
}

int jsys_loop_add(jsys_loop *loop, jsys_descriptor *descriptor) {
  //return jsys_loop_add_flags(loop, descriptor, EPOLLIN | EPOLLET | EPOLLEXCLUSIVE);
  return jsys_loop_add_flags(loop, descriptor, EPOLLIN);
}

int jsys_loop_remove(jsys_loop *loop, jsys_descriptor *descriptor) {
  loop->descriptors[descriptor->fd] = NULL;
  loop->count--;
  int r = epoll_ctl(loop->fd, EPOLL_CTL_DEL, descriptor->fd, NULL);
  loop->free(descriptor->event, "epoll_event");
  return r;
}

// timers implementation
jsys_descriptor* jsys_timer_create(jsys_loop* loop, uint64_t t0, uint64_t ti) {
  jsys_descriptor* timer = jsys_descriptor_create(loop);
  int r = jsys_timer_init(timer, t0, ti);
  if (r == -1) return NULL;
  timer->type = JSYS_TIMER;
  return timer;
}

int jsys_timer_init(jsys_descriptor *timer, uint64_t t0, uint64_t ti) {
  timer->fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timer->fd == -1) return timer->fd;
  struct itimerspec ts;
  ts.it_interval.tv_sec = ti / 1000000000;
	ts.it_interval.tv_nsec = ti % 1000000000;
	ts.it_value.tv_sec = t0 / 1000000000;
	ts.it_value.tv_nsec = t0 % 1000000000;  
  return timerfd_settime(timer->fd, 0, &ts, NULL);
}

int jsys_timer_reset(jsys_descriptor *timer, uint64_t t0, uint64_t ti) {
  struct itimerspec ts;
  ts.it_interval.tv_sec = ti / 1000000000;
	ts.it_interval.tv_nsec = ti % 1000000000;
	ts.it_value.tv_sec = t0 / 1000000000;
	ts.it_value.tv_nsec = t0 % 1000000000;  
  return timerfd_settime(timer->fd, 0, &ts, NULL);
}

uint64_t jsys_timer_read(jsys_descriptor *timer) {
  uint64_t expirations = 0;
  ssize_t n = read(timer->fd, &expirations, sizeof expirations);
  if (n != sizeof expirations) return 0;
  return expirations;
}

jsys_descriptor* jsys_sock_create(jsys_loop* loop, int domain, int type) {
  jsys_descriptor* sock = jsys_descriptor_create(loop);
  if ((sock->fd = socket(domain, type | SOCK_NONBLOCK, 0)) == -1) return NULL;
  sock->type = JSYS_SOCKET;
  return sock;
}

int jsys_tcp_connect(jsys_descriptor *sock, uint16_t port, char* address) {
  struct sockaddr_in client_addr;
  client_addr.sin_family = AF_INET;
  client_addr.sin_port = htons(port);
  inet_pton(AF_INET, address, &client_addr.sin_addr.s_addr);
  bzero(&(client_addr.sin_zero), 8);
  int r = connect(sock->fd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
  if (r == -1 && errno != EINPROGRESS) {
    fprintf(stderr, "error: %s (%i)\n", strerror(errno), errno);
    perror("connect");
    return -1;
  }
  sock->callback = jsys_tcp_on_client_event;
  return 0;
}

int jsys_socket_option(jsys_descriptor* sock, int level, int optname, int val) {
  return setsockopt(sock->fd, level, optname, &val, sizeof(int));
}

int jsys_tcp_bind(jsys_descriptor *sock, uint16_t port, in_addr_t address, int reuse_address, int reuse_port) {

  struct sockaddr_in server_addr;
  // todo: pass in flags so these can be optional. or just expose setsockopt?
  if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(int)) == -1) return -1;
  if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(int)) == -1) return -1;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = address;
  bzero(&(server_addr.sin_zero), 8);
  if (bind(sock->fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) return -1;
  sock->callback = jsys_tcp_on_accept_event;
  return 0;
}

int jsys_tcp_bind_reuse(jsys_descriptor *sock, uint16_t port, in_addr_t address) {
  return jsys_tcp_bind(sock, port, address, 1, 1);
}

int jsys_tcp_listen(jsys_descriptor *sock, int backlog) {
  return listen(sock->fd, backlog);
}

int jsys_tcp_set_nodelay(jsys_descriptor *client, int on) {
  return setsockopt(client->fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
}

int jsys_tcp_accept(jsys_descriptor *server, jsys_descriptor *client) {
  client->fd = accept(server->fd, NULL, NULL);
  if (client->fd == -1) return client->fd;
  return jsys_descriptor_non_blocking(client);
}

int jsys_tcp_pause(jsys_descriptor *sock) {
  return jsys_loop_mod_flags(sock, JSYS_WRITABLE);
}

int jsys_tcp_resume(jsys_descriptor *sock) {
  return jsys_loop_mod_flags(sock, JSYS_READABLE);
}

int jsys_udp_send_len(jsys_descriptor *client, struct iovec* iov, char* address, int port, int len) {
  struct msghdr h;
  memset(&h, 0, sizeof h);
  struct sockaddr_in client_addr;
  client_addr.sin_family = AF_INET;
  client_addr.sin_port = htons(port);
  inet_aton(address, &client_addr.sin_addr);
  bzero(&(client_addr.sin_zero), 8);
  h.msg_name = &client_addr;
  h.msg_namelen = sizeof(struct sockaddr_in);
  h.msg_iov = iov;
  h.msg_iovlen = 1;
  return sendmsg(client->fd, &h, 0);
}

int jsys_udp_send(jsys_descriptor *client, struct iovec* iov, char* address, int port) {
  return jsys_udp_send_len(client, iov, address, port, iov->iov_len);
}

int jsys_tcp_write_len(jsys_descriptor *client, struct iovec* iov, int len) {
  ssize_t r = write(client->fd, iov->iov_base, len); // TODO buffer overrun - use MIN
  while (r == -1 && errno == EINTR) {
    r = write(client->fd, iov->iov_base, len);
  }
  if (r == -1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS)) {
    // TODO: figure out the return codes. consumer needs to know why write failed
    return -1;
  }
  if (r == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS)) {
    return jsys_tcp_pause(client);
  }
  return 0;
}

int jsys_tcp_write(jsys_descriptor *client, struct iovec* iov) {
  ssize_t r = write(client->fd, iov->iov_base, iov->iov_len);
  while (r == -1 && errno == EINTR) {
    r = write(client->fd, iov->iov_base, iov->iov_len);
  }
  if (r == -1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS)) {
    // TODO: figure out the return codes. consumer needs to know why write failed
    return -1;
  }
  if (r == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS)) {
    return jsys_tcp_pause(client);
  }
  return 0;
}

int jsys_tcp_writev(jsys_descriptor *client, struct iovec* iov, int records) {
  ssize_t r = writev(client->fd, iov, records);
  while (r == -1 && errno == EINTR) {
    r = writev(client->fd, iov, records);
  }
  if (r == -1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS)) {
    return -1;
  }
  if (r == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS)) {
    return jsys_tcp_pause(client);
  }
  return 0;
}

int jsys_tcp_shutdown(jsys_descriptor* client) {
  return shutdown(client->fd, SHUT_RDWR);
}

jsys_stream_context* jsys_stream_context_create(jsys_loop* loop, int buffers) {
  jsys_stream_context* context = (jsys_stream_context*)loop->alloc(1, sizeof(jsys_stream_context), "jsys_stream_context");
  context->current_buffer = 0;
  context->padding = 0;
  context->in = NULL;
  context->out = NULL;
  context->settings = NULL;
  context->offset = 0;
  context->buffers = buffers;
  context->loop = loop;
  return context;
}

int jsys_tcp_on_accept_event(jsys_descriptor *server) {
  errno = 0;
  jsys_descriptor* client = jsys_descriptor_create(server->loop);
  client->type = JSYS_SOCKET;
  int r = 0;
  jsys_stream_settings* settings = (jsys_stream_settings*)server->data;
  jsys_loop* loop = server->loop;
  while ((jsys_tcp_accept(server, client)) != -1) {
    r = jsys_loop_add(server->loop, client);
    if (r == -1) {
      // cleanup
    };
    jsys_stream_context* context = jsys_stream_context_create(server->loop, settings->buffers);
    context->settings = settings;
    context->offset = 0;
    client->data = context;
    client->callback = jsys_tcp_on_server_event;
    r = context->settings->on_connect(client);
    if (r == -1) {
      loop->free(context, "jsys_stream_context");
      jsys_descriptor_free(client);
      fprintf(stderr, "socket rejected\n");
      // cleanup
    }
    client = jsys_descriptor_create(loop);
    client->type = JSYS_SOCKET;
  }
  loop->free(client, "jsys_descriptor");
  if (r == -1 && errno != EAGAIN) return -1;
  return 0;
}

int jsys_tcp_on_client_event(jsys_descriptor *client) {
  int r = 0;
  if (jsys_descriptor_is_error(client)) {
    jsys_stream_context* context = (jsys_stream_context*)client->data;
    context->settings->on_end(client);
    return jsys_descriptor_free(client);
  }
  if (jsys_descriptor_is_writable(client)) {
    jsys_stream_settings* settings = (jsys_stream_settings*)client->data;
    jsys_stream_context* context = jsys_stream_context_create(client->loop, settings->buffers);
    context->settings = settings;
    context->offset = 0;
    context->data = settings;
    client->data = context;
    client->closing = 0;
    r = context->settings->on_connect(client);
    if (r == -1) return jsys_descriptor_free(client);
  }
  if (jsys_descriptor_is_readable(client)) {
    jsys_stream_context* context = (jsys_stream_context*)client->data;
    ssize_t bytes = 0;
    char* next = (char*)context->in->iov_base + context->offset;
    size_t len = context->in->iov_len - context->offset;
    while ((bytes = read(client->fd, next, len))) {
      if (bytes == -1) {
        if (errno == EAGAIN) {
          if (client->closing == 1) {
            jsys_tcp_shutdown(client);
          }
          return 0;
        }
        perror("read");
        break;
      }
      r = context->settings->on_data(client, (size_t)bytes + context->offset);
      if (r == -1) {
        // this indicates a protocol error from the calling library
        // we will drop out of the loop and the on_end callback will be 
        // called and the connection closed and descriptor removed from the loop
        break;
      }
      if (context->current_buffer > 0) {
        if (context->current_buffer == 1) {
          r = jsys_tcp_write(client, &context->out[context->current_buffer]);
        } else {
          r = jsys_tcp_writev(client, context->out, context->current_buffer);
        }
        if (r == -1) return -1;
      }
      next = (char*)context->in->iov_base + context->offset;
      len = context->in->iov_len - context->offset;
    }
    context->settings->on_end(client);
    return jsys_descriptor_free(client);
  }
  return r;
}

int jsys_tcp_on_server_event(jsys_descriptor *client) {
  int r = 0;
  if (jsys_descriptor_is_error(client)) {
    jsys_stream_context* context = (jsys_stream_context*)client->data;
    context->settings->on_end(client);
    return jsys_descriptor_free(client);
  }
  if (jsys_descriptor_is_writable(client)) {
    r = jsys_tcp_resume(client);
    if (r == -1) return jsys_descriptor_free(client);
  }
  if (jsys_descriptor_is_readable(client)) {
    jsys_stream_context* context = (jsys_stream_context*)client->data;
    ssize_t bytes = 0;
    char* next = (char*)context->in->iov_base + context->offset;
    size_t len = context->in->iov_len - context->offset;
    bytes = read(client->fd, next, len);
    if (bytes == -1) {
      if (errno == EAGAIN) {
        return 0;
      }
      perror("read");
      context->settings->on_end(client);
      return jsys_descriptor_free(client);
    }
    if (bytes == 0) {
      context->settings->on_end(client);
      return jsys_descriptor_free(client);
    }
    r = context->settings->on_data(client, (size_t)bytes + context->offset);
  }
  return r;
}

// tty implementation
jsys_descriptor* jsys_tty_create(jsys_loop* loop, int fd) {
  jsys_descriptor* tty = jsys_descriptor_create(loop);
  tty->type = JSYS_TTY;
  int r = jsys_tty_init(tty, fd);
  if (r == -1) return NULL;
  return tty;
}

int jsys_tty_init(jsys_descriptor *tty, int fd) {
  char path[256];
  //struct termios tmp = { c_cflag: 0 };
  int r = ttyname_r(fd, path, sizeof(path));
  if (r != 0) return r;
  int saved_flags = fcntl(fd, F_GETFL);
  int mode = saved_flags & O_ACCMODE;
  tty->fd = open(path, mode);
  if (tty->fd == -1) return tty->fd;
  //tmp.c_iflag &= (unsigned int)~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  //tmp.c_oflag |= (ONLCR);
  //tmp.c_cflag |= (CS8);
  //tmp.c_lflag &= (unsigned int)~(ECHO | ICANON | IEXTEN | ISIG);
  //tmp.c_cc[VMIN] = 1;
  //tmp.c_cc[VTIME] = 0;
  //r = tcsetattr(tty->fd, TCSADRAIN, &tmp);
  //if (r == -1) return r;
  r = ioctl(tty->fd, FIOCLEX);
  if (r == -1) return r;
  return jsys_descriptor_non_blocking(tty);
}

// signals implementation
int jsys_signal_add(jsys_loop *loop, int signum) {
  // set blocked signals to empty and save previous in loop->set
  sigprocmask(SIG_SETMASK, NULL, &loop->set);
  // add signals to loop->set
  sigaddset(&loop->set, signum);
  // set blocked signals to loop->set
  sigprocmask(SIG_SETMASK, &loop->set, NULL);
  return 0;
}

jsys_descriptor* jsys_signal_watcher_create(jsys_loop* loop) {
  jsys_descriptor* signal = jsys_descriptor_create(loop);
  signal->type = JSYS_SIGNAL;
  // get a file descriptor so we can watch for events on blocked signals
  signal->fd = signalfd(-1, &loop->set, SFD_NONBLOCK | SFD_CLOEXEC);
  if (signal->fd == -1) return NULL;
  return signal;
}

// utilities implementation
int jsys_dump_error(jsys_loop* loop, const char* message, int r) {
  jsys_loop_free(loop); // meh...
  fprintf(stderr, "error: %s (%i):\n%s\n", message, r, strerror(r));
  return r;
}

#endif