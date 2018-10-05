const internal = 'i am internal to the module'

module.exports = {
    name: 'foo',
    foo: v => print(`foo says ${v}`),
    bar: () => print(internal),
    accessGlobal: (prop) => {
        print(`From Module: ${global.version()}`)
        print(`From Module: ${global[prop]}`)
        global.modulevar = 'ooh'
    }
}
