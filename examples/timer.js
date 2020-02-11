const timer = setInterval(() => {
  dv8.print(dv8.memoryUsage().rss)
}, 1000)
