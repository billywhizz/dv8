const { createServer } = require('../lib/http.js')
const { setSecure, addContext } = require('../lib/tls.js')

const server = createServer((req, res) => {
  req.body = []
  const { body } = req

  req.onBody = (buf, len) => {
    body.push(buf.read(0, len))
  }

  req.onEnd = () => {
    console.log(JSON.stringify(req, null, '  '))
    res.statusCode = 200
    res.setHeader('Content-Type', 'application/json')
    res.end(JSON.stringify({}))
  }
})

server.onClose(() => {
  console.log('server closed')
})

server.onConnect(sock => {
  setSecure(sock, () => {
    print('onSecure')
  })
})

addContext('dv8.billywhizz.io', './certs')
const r = server.listen()
if (r !== 0) throw new Error('Listen Error')
