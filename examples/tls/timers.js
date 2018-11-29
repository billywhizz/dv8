const tt = setInterval(() => {
  const timer = setTimeout(() => {
    clearTimeout(timer)
  }, 10)
}, 1)