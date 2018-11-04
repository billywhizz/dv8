function spawn() {
    return new Promise((ok, fail) => {
        createThread(() => {}, ({ err }) => {
            if (err) return fail(err)
            ok()
        })
    })
}

async function run() {
    let runs = 10000
    const start = Date.now()
    await Promise.all((new Array(runs)).fill(spawn).map(f => f()))
    print(Date.now() - start)
}

run().catch(err => print(err.stack))
