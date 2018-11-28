const { createServer, TCP } = require('./lib/http.js')

const opts = {
  address: '0.0.0.0',
  type: TCP,
  port: 3000,
  secure: true,
  domains: ['dv8.billywhizz.io']
}

const server = createServer((req, res) => {
  //console.log(JSON.stringify(req, null, '  '))
  res.statusCode = 404
  res.end()
}, opts)

const r = server.listen()
if (r !== 0) throw new Error('Listen Error')
