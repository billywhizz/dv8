const { Socket, TCP } = module('socket', {})
const { setSecure, addContext } = require('./lib/tls.js')
const { HTTPParser, RESPONSE } = module('httpParser', {})

const IP_ADDRESS = env.IP_ADDRESS || '127.0.0.1'
const PORT = parseInt(env.PORT || 3000)

addContext(`${IP_ADDRESS}:${PORT}`, false)

async function fetch(size, id) {
    return new Promise((ok, fail) => {
        const [ rb, wb ] = [ createBuffer(16384), createBuffer(16384) ]
        const sock = new Socket(TCP)
        
        function writeText(sock, str) {
            const len = str.length
            wb.write(str, 0)
            const r = sock.write(len)
            if (r < 0) return sock.close()
        }
        
        sock.onConnect(fd => {
            const parser = new HTTPParser()
            const work = createBuffer(16384)
            const view = new DataView(work.bytes)
            const bytes = new Uint8Array(work.bytes)
            let response = { }
            sock.setup(fd, rb, wb)
            sock.pause()
            parser.setup(rb, work)
            parser.onHeaders(() => {
                response.headers = work.read(16, view.getUint16(7))
                response.address = sock.address
                response.statusCode = view.getUint16(2)
                response.major = bytes[0]
                response.major = bytes[1]
                response.keepalive = !!bytes[4]
                response.bodyLength = 0
                const headers = response.headers.replace(/\r/g, '').split(/\n/).map(h => h.split(': '))
                response.headers = {}
                headers.forEach(h => response.headers[h[0].toLowerCase()] = h[1])
                response.totalLength = parseInt(response.headers['content-length'])
                let last = response.bodyLength
                let then = Date.now()
                response.timer = setInterval(() => {
                    const now = Date.now()
                    const elapsed = now - then
                    const bytesRead = response.bodyLength - last
                    const bps = Math.floor((bytesRead / (elapsed / 1000)) / (1024 * 1024))
                    print(`${id}: ${response.bodyLength} of ${response.totalLength}, ${Math.floor(response.bodyLength / response.totalLength * 100)} %, bps: ${bps}`)
                    last = response.bodyLength
                    then = now
                }, 1000)
            })
            parser.onBody(len => {
                response.bodyLength += len
            })
            parser.onResponse(() => {
                clearTimeout(response.timer)
                print(`${id}: ${response.bodyLength} of ${response.totalLength}, ${Math.floor(response.bodyLength / response.totalLength * 100)} %`)
                print(JSON.stringify(response, null, '  '))
                if (response.statusCode === 200 && response.totalLength === response.bodyLength) return ok()
                fail(new Error(`Status: ${response.statusCode}, Diff: ${response.totalLength - response.bodyLength}`))
            })
            //sock.onEnd(() => sock.close())
            sock.setNoDelay(true)
            sock.onError(fail)
            sock.setKeepAlive(true, 3000)
            sock.address = sock.remoteAddress()
            setSecure(sock, () => {
                writeText(sock, `GET /${size} HTTP/1.1\r\nConnection: close\r\nHost: dv8.billywhizz.io\r\n\r\n`)
            })
            parser.reset(RESPONSE, sock)
            sock.resume()
            if (sock.start) sock.start()
        })
        const r = sock.connect(IP_ADDRESS, PORT)
        if (r !== 0) fail(new Error(`Connect Error: ${r} ${sock.error(r)}`))
    })
}

async function run(size = '1', parallel = '1') {
    await Promise.all((new Array(parseInt(parallel, 10))).fill(0).map((v, i) => fetch(parseInt(size, 10), i)))
}

run(args[2], args[3]).catch(err => print(err.message))
