const internal = 'i am internal to the module'

module.exports = {
    name: 'foo',
    foo: v => print(`foo says ${v}`),
    bar: () => print(internal)
}
