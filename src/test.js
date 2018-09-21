function hello() {
    const t = new Timer()
    t.start(() => {
        const t2 = new Timer()
        t2.start(() => {
            throw new Error("Foo")
        }, 1000)
    }, 1000)
}

function init() {
    hello()
}

init()