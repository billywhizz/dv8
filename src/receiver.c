#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define MAXMSG 100
#define QUEUENAME "/foobarbaz"

int main(int argc, char** argv) {
  int BUFSIZE = 65536;
  struct mq_attr attr;
  attr.mq_flags = 0;
  attr.mq_maxmsg = MAXMSG;
  attr.mq_msgsize = BUFSIZE;
  attr.mq_curmsgs = 0;
  mq_unlink(QUEUENAME);
  mqd_t receiver = mq_open(QUEUENAME, O_CREAT | O_RDONLY, 0644, &attr);
  if (receiver == -1) {
      fprintf(stderr, "errno(mq_open) %i %s\n", errno, strerror(errno));
      return 1;
  }
  char* buf = calloc(BUFSIZE + 1, 1);
  ssize_t bytes_read = 0;
  ssize_t total_bytes = 0;
  //while (total_bytes < 1 * 1024 * 1024 * 1024) {
		bytes_read = mq_receive(receiver, buf, BUFSIZE + 1, 0);
    if (bytes_read > 0) {
      total_bytes += bytes_read;
    } else {
      fprintf(stderr, "errno(mq_receive) %i %s\n", errno, strerror(errno));
    }
  //}
  mq_unlink(QUEUENAME);
  return 0;
}