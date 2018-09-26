let bytes = 0
process.stdin.on('data', chunk => {
    bytes += chunk.length
})
setInterval(() => {
    console.log(bytes)
    bytes = 0
}, 1000)