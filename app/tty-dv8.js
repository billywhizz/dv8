let bytes = 0
const b = new Buffer()
b.alloc(64 * 1024)
const stdin = new TTY(0, len => {
    print(`len: ${len}`)
    bytes += len
})
stdin.setup(b)
const t = new Timer()
t.start(() => {
    print(`Bytes: ${bytes}`)
    bytes = 0
}, 1000, 1000)
