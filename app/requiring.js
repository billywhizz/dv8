const foo = require('./module.js')
print(`foo:\n${JSON.stringify(Object.getOwnPropertyNames(foo))}`)
print(foo.name)
foo.foo('hello')

const os = module('os', {})
print(`os:\n${JSON.stringify(Object.getOwnPropertyNames(os))}`)
print(`  OS:\n${JSON.stringify(Object.getOwnPropertyNames(os.OS))}`)
const x = new os.OS()
print(`  OS::Instance:\n${JSON.stringify(Object.getOwnPropertySymbols(x))}`)
print(`global:\n${JSON.stringify(Object.getOwnPropertyNames(global))}`)
shutdown()
