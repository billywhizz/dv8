const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO, pipe } = require('./thru.js')
pipe((buf, len, stdin, stdout) => {
    const r = stdout.write(len)
    if (r < 0) {
        stdout.close()
        return 0
    }
    if (r < len && stdout.queueSize() >= (64 * 1024)) stdin.pause()
    return len
}, UV_TTY_MODE_RAW, UV_TTY_MODE_RAW, 'foobar')
