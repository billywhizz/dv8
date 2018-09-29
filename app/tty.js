const stdout = new TTY(1)
let r = stdout.write('hello stdout\n')
print(`r = ${r}`)
const stderr = new TTY(2)
r = stderr.write('hello stderr\n')
print(`r = ${r}`)
let bytes = 0
const b = new Buffer()
b.alloc(64 * 1024)
const stdin = new TTY(0, len => {
    bytes += len
})
stdin.setup(b)

const t3 = new Timer()
t3.start(() => {
    print(`Bytes: ${bytes}`)
    bytes = 0
}, 1000, 1000)
