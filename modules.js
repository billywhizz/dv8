const { print, require, args } = dv8
const { readFile, writeFile, stat, mkdir, unlink, rmdir } = require('./fs.js')

const replacer = (k, v) => (typeof v === 'bigint') ? Number(v) : v
const stringify = o => JSON.stringify(o, replacer, '  ')

const fileName = args[2] || './modules.js'
//const buf = Buffer.fromString(readFile(fileName).read())
print(stringify(stat(fileName)))
//writeFile('./arse.js', buf)
//print(unlink('./arse.js'))
//print(mkdir('foo'))
//print(mkdir('foo/bar'))
//print(rmdir('foo/bar'))
//print(rmdir('foo'))
