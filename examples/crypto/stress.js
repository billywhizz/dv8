const { Hmac, Hash } = module('openssl', {})

function hmac1 () {
  try {
    const hmac = new Hmac()
    hmac.setup(alg, secret, buf, buf)
    hmac.create(buf.write('hello'))
    hmac.digest()
    counter++
    process.nextTick(hmac1)
  } catch (err) {
    print(err.message)
  }
}

function hmac2 () {
  try {
    hmac.setup(alg, secret, buf, buf)
    hmac.create(buf.write('hello'))
    hmac.digest()
    hmac.free()
    counter++
    process.nextTick(hmac2)
  } catch (err) {
    print(err.message)
  }
}

function hmac3 () {
  try {
    hmac.create(buf.write('hello'))
    hmac.digest()
    counter++
    process.nextTick(hmac3)
  } catch (err) {
    print(err.message)
  }
}

function hash1 () {
  try {
    const hash = new Hash()
    hash.setup(alg, buf, buf)
    hash.create(buf.write('hello'))
    hash.digest()
    counter++
    process.nextTick(hash1)
  } catch (err) {
    print(err.message)
  }
}

function hash2 () {
  try {
    hash.setup(alg, buf, buf)
    hash.create(buf.write('hello'))
    hash.digest()
    hash.free()
    counter++
    process.nextTick(hash2)
  } catch (err) {
    print(err.message)
  }
}

function hash3 () {
  try {
    hash.create(buf.write('hello'))
    hash.digest()
    counter++
    process.nextTick(hash3)
  } catch (err) {
    print(err.message)
  }
}

const onStats = () => {
  print(`counter: ${counter}, rss: ${process.memoryUsage().rss}`)
  counter = 0
  gc()
}

const alg = process.args[3] || 'md5'
const secret = process.args[4] || 'abcdefgh12345678abcdefgh12345678abcdefgh12345678abcdefgh12345678abcdefgh12345678abcdefgh12345678abcdefgh12345678abcdefgh12345678'
const tests = { hash1, hash2, hash3, hmac1, hmac2, hmac3 }
const buf = Buffer.alloc(4096)
let counter = 0
const hmac = new Hmac()
hmac.setup(alg, secret, buf, buf)
const hash = new Hash()
hash.setup(alg, buf, buf)

buf.timer = setInterval(onStats, 1000)
process.nextTick(tests[process.args[2] || 'hash1'])
