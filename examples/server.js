/*
server.js

An example http server

This app can be run in a thread (see httpd.js) or as a startup script. no code changes required
The socket module uses SO_REUSE_PORT option so multiple threads/processes can listen 
on the same port and the kernel will load balance connections across them

*/

// import global helpers - might move this to a pre-bake boostrap js script
require('./base.js')

// OS module for signal handling
const { OS } = module('os', {})
// Socket module for sockets
const { Socket } = module('socket', {})
// slightly modified version of: https://www.npmjs.com/package/http-parser-js
// this is slow. we can do it much faster with c++ http_parser, but need to figure out
// module interop
const { HTTPParser } = require('./http-parser.js')

const os = new OS()
const SIGTERM = 15
let timer // stats timer
let sock // server socket
let id = 0 // thread id - 0 for main
let closing = false // whether the server is closing or not
let conn = 0 // number of active connections
let bytesRead = 0 // number of bytes received
let bytesWritten = 0 // number of bytes received

// handle SIGTERM
function terminateHandler(signum) {
    // print is just a wrapper around fprintf(stderr, "%s"). we should replace with libuv as this can block
    print(`worker ${id} got SIGNAL: ${signum}`)
    // close the server socket - any active client sockets remain open until explicitly closed 
    sock.close()
    // set the closing flag so server can do graceful shutdown
    closing = true
    // clear the timer from the event loop
    clearTimeout(timer)
    // return 1 to close the signal watcher, anything else and it will be kept alive.
    // event loop for this thread should now be empty except for any existing tcp sockets
    return 1
}

function printMetrics() {
    print(JSON.stringify({
        worker: id,
        connections: conn,
        bytesRead,
        bytesWritten
    }, null, '  '))
}

// if we are running in a thread we will have shared workerData as an ArrayBuffer on global
if (global.workerData) {
    // get a Uint8Array so we can read/write each byte of the buffer
    const data = new Uint8Array(workerData)
    // get the threadId from the buffer
    id = data[0]
}
// setup the signal handler for SIGTERM
// can use the same callback for multiple signals, but need to register them
// individually
os.onSignal(terminateHandler, SIGTERM)

// each socket has a pre-allocated read and write buffer
const READ_BUFFER_SIZE = 4096
const WRITE_BUFFER_SIZE = 4096
// normal keepalive response
const r200 = 'HTTP/1.1 200 OK\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
// connection close response to send before closing socket
const r200Close = 'HTTP/1.1 200 OK\r\nConnection: close\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
const size200 = r200.length
const size200Close = r200Close.length

// cache the contexts
const contexts = {}
let r = 0

// create a TCP socket, 0 = TCP, 1 = Unix Domain Socket/Pipe
sock = new Socket(0)

// when we get a new client connection, this event is fired with fd of the new socket
sock.onConnect(fd => {
    conn++
    let context = contexts[fd]
    if (!context) {
        // create a new context for the socket
        context = {
            fd,
            in: new Buffer(),
            out: new Buffer()
        }
        // create byte views into the input and output buffers
        context.in.ab = new Uint8Array(context.in.alloc(READ_BUFFER_SIZE))
        context.out.ab = new Uint8Array(context.out.alloc(WRITE_BUFFER_SIZE))
        contexts[fd] = context
        // initialise a http parser
        context.parser = new HTTPParser(HTTPParser.REQUEST)
        // fires when a http request has been completed fully (body fully parsed)
        context.parser[HTTPParser.kOnMessageComplete] = () => {
            // if we are shutting down, then tell client we are closing
            // and close our end of the socket
            if (closing) {
                // tell the client to close
                sock.write(fd, size200, size200Close)
                // close our end
                sock.close(fd, 1)
                return
            }
            // write out the response (which is already in the output buffer)
            sock.write(fd, 0, size200)
        }
        // configure the socket with the buffers
        sock.setup(fd, context.in, context.out)
    } else {
        // re-initialise the parser on the existing context we are re-using
        context.parser.reinitialize(HTTPParser.REQUEST)
        context.parser[HTTPParser.kOnMessageComplete] = () => {
            if (closing) {
                sock.write(fd, size200, size200Close)
                sock.close(fd, 1)
                return
            }
            sock.write(fd, 0, size200)
        }
    }
    // enable nagle buffering
    sock.setNoDelay(fd, false)
    // enable tcp keepalive on the socket and send the packet every 5 seconds
    sock.setKeepAlive(fd, 1, 5)
    // write the standard response into the output buffer
    sock.push(fd, r200, 0)
    // write the final response into the output buffer
    sock.push(fd, r200Close, size200)
})

// this is fired when we receive data on a socket
// it tells us the number of bytes to read from the start of the 
// input buffer
sock.onData((fd, len) => {
    bytesRead += len
    const context = contexts[fd]
    const { parser } = context
    // execute the parser on the input buffer slice
    parser.execute(context.in, 0, len)
})

// this is fired when the socket is closed
sock.onClose(fd => {
    conn--
    // if we have no connections left, print the metrics
    if (conn === 0) printMetrics()
    // if we had an onexit event (when all handles in event loop are closed)
    // we could know when process is ready to exit
})    

// setup the timer to display metrics
timer = setInterval(printMetrics, 1000)

// listen to port 3000 on all available interfaces
r = sock.listen('0.0.0.0', 3000)
if (r !== 0) {
    throw new Error("Could not Listen")
}
