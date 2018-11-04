function spawn(fun) {
    return new Promise((ok, fail) => {
        createThread(run, ({ err, thread, status }) => {
            if (err) return fail(err)
            ok(thread, status)
        })
    })
}

function estimatePI() {
    const points = 40000
    var i = points;
    var inside = 0;
    while (i-- > 0) {
        if (i%10000 == 0) print(i);
        var x = Math.random();
        var y = Math.random();
        if ((x * x) + (y * y) <= 1) {
            inside++;
        }
    }
}

function run() {
    const results = await Promise.all([spawn(estimatePI), spawn(estimatePI), spawn(estimatePI), spawn(estimatePI)])
}

run().catch(err => print(err.stack))
