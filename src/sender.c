#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define MAXMSG 100

int main(int argc, char** argv) {
  int BUFSIZE = 65536;
  struct mq_attr attr;
  attr.mq_flags = 0;
  attr.mq_maxmsg = MAXMSG;
  attr.mq_msgsize = BUFSIZE;
  attr.mq_curmsgs = 0;
  mqd_t sender = mq_open("/foo_bar", O_CREAT | O_WRONLY, 0644, &attr);
  if (sender == -1) {
      fprintf(stderr, "errno(mq_open) %i %s\n", errno, strerror(errno));
      return 1;
  }
  char* buf = calloc(BUFSIZE, 1);
  int status = 0;
  ssize_t total_bytes = 0;
  //while (total_bytes < 1 * 1024 * 1024 * 1024) {
		status = mq_send(sender, buf, BUFSIZE, 0);
    if (status == 0) {
      total_bytes += BUFSIZE;
    } else {
      fprintf(stderr, "errno(mq_send) %i %s\n", errno, strerror(errno));
      usleep(1000);
    }
  //}
  return 0;
}