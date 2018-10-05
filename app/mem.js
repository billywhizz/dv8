require('./base.js')
print(JSON.stringify(memoryUsage()))
const timer = setInterval(() => print(JSON.stringify(memoryUsage())), 1000)