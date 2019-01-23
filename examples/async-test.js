run().then(() => console.log('success'), err => console.log(JSON.stringify({ message: err.message, stack: err.stack }, null, '  ')));

async function run () {
  await new Promise(resolve => setTimeout(resolve, 10))
  await bar()
}

async function bar () {
  await Promise.resolve()
  // Stack trace will just include `bar()`, no reference to `foo()`
  throw new Error('Oops')
}