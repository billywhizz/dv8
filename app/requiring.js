const foo = require('./module.js')

print(`foo:\n${JSON.stringify(Object.getOwnPropertyNames(foo))}`)
print(foo.name)
foo.foo('hello')
shutdown()