const { createServer } = require('./lib/http.js')

const middleware = (req, res) => {
  req.onBody((buf, len) => {
    print(buf.read(0, len))
  })
  req.onEnd(() => {
    print(JSON.stringify(req, null, '  '))
    res.statusCode = 200
    res.setHeader('Content-Type', 'application/json')
    res.end(JSON.stringify({}))
  })
}

createServer(middleware).listen()
