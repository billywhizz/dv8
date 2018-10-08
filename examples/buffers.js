require('./base.js')

const buffers = []

function allocateBuffer() {
    const b = new Buffer()
    b.items = b.alloc(1024)
    buffers.push(b)
    setTimeout(() => {
        const bb = buffers.pop()
        if (bb) bb.free()
    }, 1000)
}

function showMem() {
    print(JSON.stringify(memoryUsage(), null, '  '))
}

const t1 = setInterval(allocateBuffer, 1000)
const t2 = setInterval(gc, 5000)
const t3 = setInterval(showMem, 1000)