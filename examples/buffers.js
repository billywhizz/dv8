let b = new Buffer()
let bytes = b.alloc(1024)

setInterval(() => {
    b = new Buffer()
    bytes = b.alloc(1024)
    gc()
}, 1000)