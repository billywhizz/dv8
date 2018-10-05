const foo = require('./module.js')
print(`foo:\n${JSON.stringify(Object.getOwnPropertyNames(foo))}`)
print(foo.name)
foo.foo('hello')
try {
    foo.foo(internal)
} catch (err) {
    print(`
message  : ${err.message}
number   : ${err.number}
stack    : ${err.stack}
code     : ${err.code}
    `)
}
foo.bar()
global.testing = 'i am a test global variable'
foo.accessGlobal('testing')
print(`i can see you: ${global.modulevar}`)
delete global.testing
delete global.modulevar
const os = module('os', {})
print(`os:\n${JSON.stringify(Object.getOwnPropertyNames(os))}`)
print(`  OS:\n${JSON.stringify(Object.getOwnPropertyNames(os.OS))}`)
const x = new os.OS()
print(`  OS::Instance:\n${JSON.stringify(Object.getOwnPropertySymbols(x))}`)
print(`global:\n${JSON.stringify(Object.getOwnPropertyNames(global))}`)
shutdown()
