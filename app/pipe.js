const BUFFER_SIZE = 64 * 1024
const HIGH_WATER = 1 * 1024 * 1024

const b = new Buffer()
b.alloc(BUFFER_SIZE)

let stats = {
    stdin: {
        read: 0,
        error: 0,
        pause: 0,
        close: 0,
        ondata: 0,
        resume: 0
    },
    stdout: {
        incomplete: 0,
        full: 0,
        error: 0,
        written: 0,
        drain: 0,
        close: 0,
        maxQueue: 0
    }
}
let stdin
let stdout

// write stream
stdout = new TTY(1, () => {
    // onclose
    stats.stdout.close++
    print(`pipe.stdout:\n${JSON.stringify(stats.stdout, null, '  ')}`)
}, () => {
    // ondrain
    stats.stdout.drain++
    const queueSize = stdout.queueSize()
    if (queueSize > stats.stdout.maxQueue) stats.stdout.maxQueue = queueSize
    stdin.resume()
    stats.stdin.resume++
    stdout.draining = false
    if (stdout.closing) stdout.close()
})
stdout.setup(b)

// read stream
stdin = new TTY(0, len => {
    stats.stdin.ondata++
    // ondata
    stats.stdin.read += len
    const r = stdout.write(len)
    if (r < 0) {
        stats.stdout.error++
        print(`pipe.stdout.write.error: ${r}`)
    } else if (r < len) {
        stats.stdout.incomplete++
        stats.stdout.written += len
        const queueSize = stdout.queueSize()
        if (queueSize > stats.stdout.maxQueue) stats.stdout.maxQueue = queueSize
        if (queueSize > HIGH_WATER) {
            stdout.draining = true
            stdin.pause()
            stats.stdin.pause++
        }
    } else {
        stats.stdout.full++
        stats.stdout.written += r
    }
}, err => {
    // onend
    if (err) {
        stats.stdin.error++
        print(`pipe.stdin.read.error: ${err}`)
    }
    stdin.pause()
    stats.stdin.pause++
    stdin.close()
}, () => {
    // onClose
    stats.stdin.close++
    if (stdout.draining) {
        stdout.closing = true
    } else {
        stdout.close()
    }
    print(`pipe.stdin:\n${JSON.stringify(stats.stdin, null, '  ')}`)
})
stdin.setup(b)
stdin.resume()
stats.stdin.resume++
