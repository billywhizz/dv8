require('./debug.js')

let counter = 0

const timer = setInterval(() => {
  counter++
  print('goodbye')
}, 1000)
