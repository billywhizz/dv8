async function run() {
    const b = new Buffer()
    print(`b:\n${JSON.stringify(Object.getOwnPropertyNames(b))}`)
    const ab = b.alloc(20)
    print(ab)
    print(`ArrayBuffer:\n  Byte Length: ${ab.byteLength}`)
    const view = new Uint8Array(ab)
    print(`Uint8Array:\n  length: ${view.length}`)
    let i = 0
    view.map(b => print(`${i++}: ${b}`))
    i = 0
    'andrew 012'.split('').forEach(c => view[i++] = c.charCodeAt(0))
    i = 0
    view.map(b => print(`${i++}: ${b}`))
    print(b.pull(0, 10))
    b.push("hello", 0)
    print(b.pull(0, 5))
}

run().catch(err => print(`error:\n${err.toString()}`))
