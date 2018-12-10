process.onExit = () => {
  print(JSON.stringify(process.activeHandles(), null, '  '))
}

const timer = setInterval(() => {
  print(JSON.stringify(process.activeHandles(), null, '  '))
  print(process.memoryUsage().rss)
}, 1000)

setTimeout(() => clearTimeout(timer), 3000)
