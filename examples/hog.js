const thread = createThread(() => {
    const buffers = []
    const t1 = setInterval(() => {
        const ab = new ArrayBuffer(1024 * 1024)
        const bytes = new Uint8Array(ab)
        bytes.fill(0)
        buffers.push(ab)
    }, 1)
    const t2 = setInterval(() => {
        print(JSON.stringify(memoryUsage()))
    }, 1000)
}, ({ err, thread, status }) => {
    print(`thread: ${thread.id}\ntime: ${thread.time}\nboot: ${thread.boot}, status: ${status}`)
    if (err) {
        print(err.exception)
        print(err.stack)
    }
})
