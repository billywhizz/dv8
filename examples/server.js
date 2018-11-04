const server = require('http').createServer((req, res) => {
    res.end()
})
server.listen(3000)
server.keepAliveTimeout = 20000
setTimeout(() => {
    console.log('closing')
    server.close()
}, 10000)