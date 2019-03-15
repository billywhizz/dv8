const { Socket, TCP } = module('socket', {})
const { setSecure, addContext } = require('../lib/tls.js')
const { HTTPParser, RESPONSE } = module('httpParser', {})

const IP_ADDRESS = process.env.IP_ADDRESS || '52.218.80.108'
const PORT = parseInt(process.env.PORT || 443)

addContext(`${IP_ADDRESS}:${PORT}`, false)

async function fetch (filename, id) {
  return new Promise((resolve, reject) => {
    const [rb, wb] = [Buffer.alloc(16384), Buffer.alloc(16384)]
    const sock = new Socket(TCP)

    function writeText (sock, str) {
      const len = str.length
      wb.write(str, 0)
      const r = sock.write(len)
      if (r < 0) return sock.close()
    }

    sock.onConnect(() => {
      const parser = new HTTPParser()
      const work = Buffer.alloc(16384)
      const view = new DataView(work.bytes)
      const bytes = new Uint8Array(work.bytes)
      let response = {}
      sock.setup(rb, wb)
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
        headers.forEach(h => {
          response.headers[h[0].toLowerCase()] = h[1]
        })
        response.totalLength = parseInt(response.headers['content-length'])
        response.timer = setInterval(() => {
          print(`${filename} (${id}): ${response.bodyLength} of ${response.totalLength}, ${Math.floor(response.bodyLength / response.totalLength * 100)} %`)
        }, 1000)
      })
      parser.onBody(len => {
        response.bodyLength += len
      })
      parser.onResponse(() => {
        clearTimeout(response.timer)
        print(`${filename} (${id}): ${response.bodyLength} of ${response.totalLength}, ${Math.floor(response.bodyLength / response.totalLength * 100)} %`)
        print(JSON.stringify(response, null, '  '))
        if (response.statusCode === 200 && response.totalLength === response.bodyLength) return resolve(response)
        reject(new Error(`Status: ${response.statusCode}, Diff: ${response.totalLength - response.bodyLength}`))
      })
      sock.onEnd(() => sock.close())
      sock.setNoDelay(true)
      sock.onError(reject)
      sock.setKeepAlive(true, 3000)
      setSecure(sock, () => {
        writeText(sock, `GET /oneflow-public/${filename} HTTP/1.1\r\nConnection: close\r\nHost: s3-eu-west-1.amazonaws.com\r\n\r\n`)
      })
      parser.reset(RESPONSE, sock)
      sock.resume()
      sock.start()
    })
    const r = sock.connect(IP_ADDRESS, PORT)
    if (r !== 0) reject(new Error(`Connect Error: ${r} ${sock.error(r)}`))
  })
}

async function run (parallel = 1) {
  const results = await Promise.all((new Array(parseInt(parallel, 10))).fill(0).map((v, i) => fetch('whitespace.pdf', i)))
  print(JSON.stringify(results))
}

run(process.args[2]).catch(err => print(err.message))
