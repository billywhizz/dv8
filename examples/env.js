print(JSON.stringify(memoryUsage(), null, '  '))
const BUFSIZE = ( 64 * 1024 * 1024 )
let b = new Buffer()
b.bytes = b.alloc(BUFSIZE)

const t1 = setInterval(() => {
    b = new Buffer()
    b.bytes = b.alloc(BUFSIZE)
}, 1)

let runs = parseInt(args[4] || '5', 10)
const t2 = setInterval(() => {
    gc()
    print(`thread: ${threadId}\n${JSON.stringify(memoryUsage(), null, '  ')}`)
    const buf = new Uint8Array(b.bytes)
    let len = b.bytes.byteLength
    while(len--) {
        buf[len] = 1
    }
    t2.bb = b
    len = b.bytes.byteLength
    let counter = 0
    while(len--) {
        counter += buf[len]
    }
    if (!--runs) {
        clearTimeout(t1)
        clearTimeout(t2)
    }
}, 1000)