const { HTTPParser, REQUEST } = module('httpParser', {})

const parser = new HTTPParser()
const rb = createBuffer(4096)
const wb = createBuffer(4096)

const bytes = new Uint8Array(wb.bytes)
const dv = new DataView(wb.bytes)

parser.setup(REQUEST, rb, wb)

parser.onError((code, message) => {
    print(`onError: ${code} ${message}`)
})

parser.onHeaders(() => {
    const [ major, minor, headerCount, method, upgrade, keepalive ] = bytes
    const urlLength = dv.getUint32(8)
    const headerStart = 12 + urlLength
    let off = headerStart
    let nh = headerCount
    let len = 0
    let k, v
    const headers = {}
    let hostname
    const url = wb.read(12, urlLength)
    while (nh--) {
        len = (bytes[off] << 8) + bytes[off + 1]
        off += 2
        k = wb.read(off, len)
        off += len
        len = (bytes[off] << 8) + bytes[off + 1]
        off += 2
        v = wb.read(off, len)
        if (k.toLowerCase() === 'host') hostname = v.split(':')[0]
        off += len
        headers[k] = v
    }
    print('onHeaders')
    print(JSON.stringify({
        major,
        minor,
        method,
        upgrade,
        keepalive,
        urlLength,
        url,
        hostname,
        headerCount,
        headers
    }, null, '  '))
})

parser.onRequest(() => {
    print('onRequest')
})

parser.onBody(len => {
    print(`onBody: ${len}`)
})

const tests = [
    'GET /foo HTTP/1.1\r\nHost: foo.bar\r\n\r\n',
    'GET /foobar?name=goo HTTP/1.1\r\nHost: foo.bar\r\nAccept: application/json\r\n\r\n',
    'POST /foo HTTP/1.1\r\nHost: foo.bar\r\nContent-Length: 5\r\n\r\nhello'
]

tests.forEach(request => {
    rb.write(request, 0)
    parser.execute(request.length)
})
