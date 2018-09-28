const b = new Buffer()
b.alloc(64 * 1024)
let bytes = {
    read: 0
}
const stdin = new TTY(0, len => {
    bytes.read += len
}, () => {
    print(`count:\n${JSON.stringify(bytes, null, '  ')}`)
    stdin.close()
}, () => {
    print('count.stdin.onClose')
})
stdin.setup(b)
const r = stdin.resume()
if (r !== 0) {
    print(`count.stdin.resume.error: ${r}`)
}
