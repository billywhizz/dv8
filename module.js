const { args, print, require, compile } = dv8
const { readFile, writeFile, FileSystem } = require('./src/util.js')
const [name, className] = args.slice(2)
print(`name: ${name}, className: ${className}`)
const header = { text: '' }
compile('file.text = `' + readFile('./module.h').read() + '`', '', ['name', 'className', 'file'], []).call({}, name, className, header)
const source = { text: '' }
compile('file.text = `' + readFile('./module.cc').read() + '`', '', ['name', 'className', 'file'], []).call({}, name, className, source)
FileSystem.mkdir(`./src/modules/${name}`)
writeFile(`./src/modules/${name}/${name}.h`, Buffer.fromString(header.text))
writeFile(`./src/modules/${name}/${name}.cc`, Buffer.fromString(source.text))
