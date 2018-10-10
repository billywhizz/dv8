/*
httpd.js

An example threaded daemon running multiple http servers in isolated thread contexts

when you start a new "thread" you are doing the following:
- using an existing thread in the libuv worker pool
    - this pool defaults to 4 threads which are initialised on startup by libuv
    - the number of threads can be adjusted using UV_THREADPOOL_SIZE environment variable, up to 128 threads
- creating a new V8 Isolate which is a completely independent instance of the v8 virtual machine
- creating a new isolated context in the new VM
    - globals are available in the new context, but they are not shared with the main process or any other contexts
    - everything in the new context should work the same as in a main module - sockets/timers/tty etc.
    - each context has it's own event loop
    - when there are no active handles left on the context event loop, the thread will be released

each worker thread will be on the main event loop (and will have it's own event loop it is running)
when all workers finish, the main event loop will be empty (assuming no other async work is active) and the app will exit
each worker listent to SIGTEM. when it receives it, it will shut down cleanly, all threads will finish and the main event loop will shut down cleanly

e.g. this is what the output looks like when sending SIGTERM
worker 2 got SIGNAL: 15
singal watcher stopped: 15
uv_thread_loop_close: -16
worker 2 stopped
worker 3 got SIGNAL: 15
singal watcher stopped: 15
uv_thread_loop_close: -16
worker 3 stopped
worker 4 got SIGNAL: 15
singal watcher stopped: 15
uv_thread_loop_close: -16
worker 4 stopped
worker 1 got SIGNAL: 15
singal watcher stopped: 15
uv_thread_loop_close: -16
worker 1 stopped

*/

// import the Thread Prototype from the module
const { Thread } = module('thread', {})

// counter for thread id's allocated
let id = 1

// spawn a new thread and execute the script in fname in the thread context
function spawn(fname) {
    // create a new instance of Thread class - nothing is allocated at this stage
    const thread = new Thread()
    // create a new buffer and ref it on the thread
    thread.buffer = new Buffer()
    // allocate a 10 byte buffer and crrate a bytes view on it
    const bytes = new Uint8Array(thread.buffer.alloc(10))
    // increment the thread id and put the value in the shared buffer
    const threadId = bytes[0] = id++
    // spawn the thread, create the v8 context, execute the script
    // the buffer is shared with the thread and can be manipulated on both sides
    thread.start(fname, () => {
        // called when thread completes/exits
        print(`worker ${threadId} stopped`)
    }, thread.buffer)
}

// spawn 4 worker threads and execute the server.js script in each
const WORKERS = 4
let i = WORKERS
while(i--) {
    spawn('./server.js')
}
