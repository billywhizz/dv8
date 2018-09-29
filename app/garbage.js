require('./base.js')

const objects = []
let gid = 0

class Foo {
    constructor() {
        this.id = gid++
        this.buffer = new Buffer()
        this.buffer.alloc(1000)
    }
}

const t = setInterval(() => {
    objects.push(new Foo())
}, 100)

setInterval(() => {
    print(`clearing: ${objects.length}`)
    objects.length = 0
    print(JSON.stringify(memoryUsage(), null, '  '))
}, 3000)
