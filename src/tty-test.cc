#include <uv.h>
#include <stdlib.h>

static uv_buf_t b;
static uint64_t bytes = 0;
static uv_tty_t* _stdin;

static void on_close(uv_handle_t* handle) {
    int r = uv_loop_close(uv_default_loop());
    fprintf(stderr, "uv_loop_close: %i\n", r);
}

static void on_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t* buf) {
    if (nread == UV_EOF) {
        fprintf(stderr, "bytes: %lu\n", bytes);
        uv_close((uv_handle_t*)_stdin, on_close);
    } else if (nread > 0) {
        bytes += nread;
    } else if (nread < 0) {
        fprintf(stderr, "read_error: %u\n", nread);
    }
}

static void alloc_chunk(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
    buf->base = b.base;
    buf->len = b.len;
}

int main(int argc, char *argv[]) {
    _stdin = (uv_tty_t*)calloc(1, sizeof(uv_tty_t));
    uv_tty_init(uv_default_loop(), _stdin, 0, 1);
    b.base = (char*)calloc(65536, 1);
    b.len = 65536;
    int status = uv_read_start((uv_stream_t*)_stdin, alloc_chunk, on_read);
    bool more;
    do {
        uv_run(uv_default_loop(), UV_RUN_DEFAULT);
        more = uv_loop_alive(uv_default_loop());
        if (more) {
            continue;
        }
    } while (more == true);
    return 0;
}