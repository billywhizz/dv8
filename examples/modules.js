const { Javascript } = module('js', {})
const js = new Javascript()
print(`Javascript.hello: ${js.hello()}`)
const { Filesystem } = module('fs', {})
const fs = new Filesystem()
print(`Filesystem.hello: ${fs.hello()}`)
const { Event } = module('event', {})
const event = new Event()
print(`Event.hello: ${event.hello()}`)
const { UDP } = module('udp', {})
const udp = new UDP()
print(`UDP.hello: ${udp.hello()}`)
const { DNS } = module('dns', {})
const dns = new DNS()
print(`DNS.hello: ${dns.hello()}`)
const { Thread } = module('thread', {})
const thread = new Thread()
print(`Thread.hello: ${thread.hello()}`)