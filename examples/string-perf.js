const b = new Buffer()
const ab = b.alloc(65536)
const bytes = new Uint8Array(ab)
const dv = new DataView(ab)

const values = [
    'Host: 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789',
    'Content-Length: 100',
    'Content-Type: application/json',
    'Accept: application/json',
    'Host: 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789',
    'Content-Length: 100',
    'Content-Type: application/json',
    'Accept: application/json',
    'Host: 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789',
    'Content-Length: 100',
    'Content-Type: application/json',
    'Accept: application/json',
    'Host: 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789',
    'Content-Length: 100',
    'Content-Type: application/json',
    'Accept: application/json',
    'Host: 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789',
    'Content-Length: 100',
    'Content-Type: application/json',
    'Accept: application/json',
    'User-Agent: foo'
]

function encodeHeaders2() {
    let off = 2
    let soff = 128
    dv.setUint8(0, values.length)
    values.forEach(v => {
        dv.setUint16(off, v.length)
        off += 2
        b.write(v, soff)
        soff += v.length
    })
}

function encodeHeaders1() {
    let off = 2
    dv.setUint8(0, values.length)
    values.forEach(v => {
        dv.setUint16(off, v.length)
        off += 2
        b.write(v, off)
        off += v.length
    })
}

const lengths = new Uint16Array(values.length)

function parseHeaders2(headers) {
    let off = 2
    let len = 0
    let total = 0
    const nheaders = dv.getUint8(0)
    let nh = nheaders
    let curr = 0
    while (nh--) {
        len = dv.getUint16(off)
        lengths[curr++] = len
        total += len
        off += 2
    }
    const str = b.read(128, total)
    nh = nheaders
    off = 0
    curr = 0
    while (nh--) {
        len = lengths[curr]
        headers[curr] = str.substring(off, off + len)
        off += len
        curr++
    }
}

function parseHeaders1(headers) {
    let off = 2
    let len = 0
    let curr = 0
    let nh = dv.getUint8(0)
    while (nh--) {
        len = dv.getUint16(off)
        off += 2
        headers[curr++] = b.read(off, len)
        off += len
    }
}

function parseHeaders3(headers) {
    return b.parse(headers)
}

function run1() {
    const headers = []
    encodeHeaders1()
    let runs = parseInt(args[2] || '1000', 10)
    const start = Date.now()
    while(runs--) {
        parseHeaders1(headers)
    }
    print(`run1: ${Date.now() - start}`)
    print(JSON.stringify(headers, null, '  '))
}

function run2() {
    const headers = []
    encodeHeaders2()
    let runs = parseInt(args[2] || '1000', 10)
    const start = Date.now()
    while(runs--) {
        parseHeaders2(headers)
    }
    print(`run2: ${Date.now() - start}`)
    print(JSON.stringify(headers, null, '  '))
}

function run3() {
    const headers = ['','','','','','','','','','','','','','','','','','','','','','','','',]
    encodeHeaders2()
    let runs = parseInt(args[2] || '1000', 10)
    const start = Date.now()
    while(runs--) {
        parseHeaders3(headers)
    }
    print(`run3: ${Date.now() - start}`)
    print(JSON.stringify(headers, null, '  '))
}

run1()
run2()
run3()