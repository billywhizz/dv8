const { Thread } = module('thread', {})

const BUFFER_SIZE = 4 * 1024
let next = 1
const thousand = BigInt(1000)
const million = BigInt(1000000)

onExit(() => print('exiting main'))

function spawn(fname) {
    const start = hrtime()
    const thread = new Thread()
    thread.buffer = createBuffer(BUFFER_SIZE)
    const dv = new DataView(thread.buffer.bytes)
    thread.id = next++
    dv.setUint8(0, thread.id)
    const envJSON = JSON.stringify(env)
    const argsJSON = JSON.stringify(args)
    dv.setUint32(1, envJSON.length)
    thread.buffer.write(envJSON, 5)
    dv.setUint32(envJSON.length + 5, argsJSON.length)
    thread.buffer.write(argsJSON, envJSON.length + 9)
    thread.start(fname, () => {
        const finish = hrtime()
        print(`${thread.id.toString().padEnd(5, ' ')} : ${(finish - start) / thousand} usec`)
        const ready = dv.getBigUint64(0)
        print(`boot: ${(ready - start) / thousand} usec`)

    }, thread.buffer)
}

const WORKERS = parseInt(args[3] || '4', 10)
let i = WORKERS
while(i--) {
    spawn(args[2])
}
