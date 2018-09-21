require('./base.js')
const sleep = ms => new Promise(ok => setTimeout(ok, ms))

async function run() {
    const foo = require('./module.js')
    print(foo.name)
    foo.foo('hello')
    foo.bar()
    // console.log(internal)
    await sleep(1000)
    console.log('i had a sleep')
    shutdown()
}

run().catch(err => print(err.toString())).finally(shutdown)
