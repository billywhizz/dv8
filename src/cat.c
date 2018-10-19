#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

ssize_t safe_write(int fd, const void *buf, size_t count)
{
	ssize_t n;

	do {
		n = write(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

ssize_t bb_full_write(int fd, const void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len > 0) {
		cc = safe_write(fd, buf, len);

		if (cc < 0)
			return cc;		/* write() returns -1 on failure. */

		total += cc;
		buf = ((const char *)buf) + cc;
		len -= cc;
	}

	return total;
}

static ssize_t bb_full_fd_action(int src_fd, int dst_fd, size_t size)
{
	int status = -1;
	size_t total = 0;
    void* buffer = calloc(BUFSIZ, 1);

	if (src_fd < 0) goto out;
	while (!size || total < size)
	{
		ssize_t wrote, xread;
		
		xread = safe_read(src_fd, buffer,
				(!size || size - total > BUFSIZ) ? BUFSIZ : size - total);

		if (xread > 0) {
			/* A -1 dst_fd means we need to fake it... */
			wrote = (dst_fd < 0) ? xread : bb_full_write(dst_fd, buffer, xread);
			if (wrote < xread) {
                fprintf(stderr, "wrote less bytes than read\n");
				break;
			}
			total += wrote;
		} else if (xread < 0) {
            fprintf(stderr, "write error: %li\n", xread);
			break;
		} else if (xread == 0) {
			/* All done. */
			status = 0;
			break;
		}
	}
		
out:
    free(buffer);
	return status ? status : total;
}

int main(int argc, char *argv[]) {
    int infd = open("./test.bin", O_RDONLY);
    if (infd > 0) {
        posix_fadvise(infd, 0, 0, POSIX_FADV_SEQUENTIAL);
        bb_full_fd_action(infd, STDOUT_FILENO, 0);
    }
}

