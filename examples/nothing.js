const now = Date.now()

const env = global.env().map(entry => entry.split('=')).reduce((e, pair) => { e[pair[0]] = pair[1]; return e }, {})

const launched = parseInt(env.TIME, 10)
const diff = now - launched
print(diff)
