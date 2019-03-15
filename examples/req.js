const one = require('./lib.js')
const two = require('./lib.js')

print(`one: ${one.number}, two: ${two.number}`)
print(one == two)

one.number++
print(`one: ${one.number}, two: ${two.number}`)
