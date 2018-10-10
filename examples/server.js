// import global helpers
require('./base.js')

// imports
const { OS } = module('os', {})
const { Socket } = module('socket', {})
const { HTTPParser } = require('./http-parser.js')

// module variables
const os = new OS()
const SIGTERM = 15
let timer // stats timer
let sock // server socket
let id = 0

// handle SIGTERM
function SIGTERM_handler(signum) {
    print(`worker ${id} got SIGNAL: ${signum}`)
    // close the server socket - any active client sockets remain open until explicitly closed 
    sock.close()
    // clear the timer from the event loop
    clearTimeout(timer)
    // return watcher to close the signal watcher. event loop for this thread should now be empty
    return 1
}

async function run() {
    if (global.workerData) {
        // if we are running in a thread we will have shared workerData as an ArrayBuffer on global
        const data = new Uint8Array(workerData)
        // get the threadId from the shared array buffer
        id = data[0]
    }
    // setup the signal handler for SIGTERM
    os.onSignal(SIGTERM_handler, SIGTERM)

    const READ_BUFFER_SIZE = 4096
    const WRITE_BUFFER_SIZE = 4096
    const r200 = 'HTTP/1.1 200 OK\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
    const size200 = r200.length
    const contexts = {}
    let conn = 0
    let r = 0

    sock = new Socket(0)
    
    sock.onConnect(fd => {
        conn++
        let context = contexts[fd]
        if (!context) {
            context = {
                fd,
                in: new Buffer(),
                out: new Buffer()
            }
            context.in.ab = new Uint8Array(context.in.alloc(READ_BUFFER_SIZE))
            context.out.ab = new Uint8Array(context.out.alloc(READ_BUFFER_SIZE))
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

    // setup the timer to display metrics
    timer = setInterval(() => {
        print(JSON.stringify({
            worker: id,
            connections: conn
        }, null, '  '))
    }, 1000)
    
    r = sock.listen('0.0.0.0', 3000)
    if (r !== 0) {
        throw new Error("Could not Listen")
    }
    
}

run().catch(err => print(`error`))
