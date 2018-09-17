const { Socket } = module('socket', {})
const { HTTPParser } = require('./http-parser.js')

const READ_BUFFER_SIZE = 4096
const WRITE_BUFFER_SIZE = 4096
const r200 = 'HTTP/1.1 200 OK\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
const size200 = r200.length
const contexts = {}

const sock = new Socket(0)
let conn = 0

sock.onConnect(fd => {
	conn++
	let context = contexts[fd]
	if (!context) {
		context = {
			fd,
			in: new Buffer(),
			out: new Buffer()
        }
        context.in.alloc(READ_BUFFER_SIZE)
        context.out.alloc(READ_BUFFER_SIZE)
		contexts[fd] = context
        context.parser = new HTTPParser(HTTPParser.REQUEST)
        context.parser[HTTPParser.kOnMessageComplete] = () => {
            sock.write(fd, 0, size200)
        }
		sock.setup(fd, context.in, context.out)
    } else {
        context.parser.reinitialize(HTTPParser.REQUEST)
        context.parser[HTTPParser.kOnMessageComplete] = () => {
            sock.write(fd, 0, size200)
        }
    }
	sock.setNoDelay(fd, false)
	sock.push(fd, r200, 0)
})

sock.onData((fd, len) => {
    const context = contexts[fd]
    const { parser } = context
    parser.execute(context.in, 0, len)
})

sock.onClose(fd => {
	conn--
})

/*
setInterval(() => {
	console.log(conn)
}, 1000)
*/

print(`listen: ${sock.listen('0.0.0.0', 3000)}`)
