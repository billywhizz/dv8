
global.onUncaughtException = err => {
    print('uncaughtException')
    print(err.message)
    print(err.stack)
}

let i = 0

function foo() {
    print(`foo: ${global.ticks}`)
}

function tickHandler() {
    print(`${i}: ${new Date()}, ticks: ${global.ticks}`)
    i++
    nextTick(foo)
    nextTick(foo)
    nextTick(foo)
    nextTick(foo)
}

const timer = setInterval(() => {
    nextTick(tickHandler)
}, 1000)
/*
const { EventLoop, UV_RUN_DEFAULT } = module('loop', {})

const loop = new EventLoop()
function runloop(id) {
    const stats = {
        idle: 0,
        check: 0,
        prepare: 0
    }
    const loop = new EventLoop()
    loop.onIdle(() => {
        stats.idle++
        //runMicroTasks()
    })
    loop.onCheck(() => {
        stats.check++
        //runMicroTasks()
    })
    loop.onPrepare(() => {
        stats.prepare++
        //runMicroTasks()
    })
    return setInterval(() => {
        print(`loop ${id}:\n${JSON.stringify(stats, null, '  ')}`)
        stats.idle = stats.check = stats.prepare = 0
    }, 1000)
}

const timer1 = runloop(1)
const timer2 = runloop(2)

global.onUnhandledRejection((promise, value) => {
    print('unhandledRejection')
})

global.onUncaughtException = err => {
    print('uncaughtException')
    print(err.message)
    print(err.stack)
}

*/
/*
async function run() {
    await sleep(1000)
    throw new Error('Foo')
}

const t = setTimeout(() => {
    throw new Error("Bar")
}, 3000)

run()

throw new Error("Bar")
*/
