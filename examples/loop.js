function runLoop() {
    const { EventLoop, UV_RUN_DEFAULT } = module('loop', {})
    const loop = new EventLoop()
    setTimeout(() => {
        loop.onIdle()
        loop.stop()
    }, 10000)
    loop.stop()
    let alive = loop.isAlive()
    print(`loop alive ${threadId}: ${alive}`)
    loop.onIdle(() => {
        
    })
    do {
        loop.run(UV_RUN_DEFAULT)
        alive = loop.isAlive()
        print(`loop done ${threadId}: ${alive}`)
    } while (alive != 0);
    loop.close()
}

const thread = createThread(runLoop, ({ err, thread, status }) => {
    print(`thread: ${thread.id}\ntime: ${thread.time}\nboot: ${thread.boot}, status: ${status}`)
})

runLoop()