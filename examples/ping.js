const { library, memoryUsage, print, args } = dv8
const { Socket } = library('net')
const BUFFER_SIZE = 4 * 1024
const stdout = new Socket()
const stdin = new Socket()
const buf = Buffer.alloc(BUFFER_SIZE)
const todo = parseInt(args[2] || '3', 10)
stdout.onConnect(() => {
  let counter = 0
  stdout.setup(buf, buf)
  const timer = setInterval(() => {
    const stats = { rss: memoryUsage().rss }
    stdout.write(buf.write(`${JSON.stringify(stats)}\n`))
    counter++
    if (counter === todo) {
      clearTimeout(timer)
      stdin.close()
      stdout.close()
    }
  }, 1000)
})
stdout.pair(1)
stdin.onConnect(() => {
  stdin.setup(buf, buf)
})
stdin.onData(len => {
  print(`onData: ${len}`)
})
stdin.pair(0)
