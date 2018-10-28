const MAX_SIZE = (1 * 1024 * 1024 * 1024) - 1
let blocks = [createBuffer(MAX_SIZE), createBuffer(MAX_SIZE), createBuffer(MAX_SIZE), createBuffer(MAX_SIZE), createBuffer(MAX_SIZE)]
const t1 = setInterval(() => {
    gc()
    print(JSON.stringify(memoryUsage(), null, '  '))
}, 1000)
const t2 = setInterval(() => {
    blocks = []
    blocks = [createBuffer(MAX_SIZE), createBuffer(MAX_SIZE), createBuffer(MAX_SIZE), createBuffer(MAX_SIZE), createBuffer(MAX_SIZE)]
}, 5000)
