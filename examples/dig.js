const { require, print, args } = dv8
const { lookup, qtypes, qclasses } = require('./lib/dns.js')

function getFlags (message) {
  const flags = []
  if (message.QR) flags.push('qr')
  if (message.AA) flags.push('aa')
  if (message.TC) flags.push('tc')
  if (message.RD) flags.push('rd')
  if (message.RA) flags.push('ra')
  if (message.Z) flags.push('z')
  return flags.join(' ')
}

function getAnswer (answer) {
  const { name, ttl, qclass, qtype, ip, cname } = answer
  if (qtype === 1) {
    return `${name.join('.').slice(0, 29).padEnd(30, ' ')}${ttl.toString().padEnd(8, ' ')}${qclasses[qclass].padEnd(4, ' ')}${qtypes[qtype].padEnd(8, ' ')}${ip['0']}.${ip['1']}.${ip['2']}.${ip['3']}`
  } else if (qtype === 5) {
    return `${name.join('.').slice(0, 29).padEnd(30, ' ')}${ttl.toString().padEnd(8, ' ')}${qclasses[qclass].padEnd(4, ' ')}${qtypes[qtype].padEnd(8, ' ')}${cname.join('.')}`
  }
}

async function run () {
  const start = Date.now()
  const params = {
    query: args[2] || 'www.google.com',
    address: args[3] || '8.8.8.8',
    port: parseInt(args[4] || '53', 10)
  }
  const result = await lookup(params)
  const elapsed = Date.now() - start
  const { address, port, message, length } = result
  const { id, ancount, qcount, nscount, arcount, RCODE, opCode, question, answer } = message
  print(`; <<>> DiG 9.11.3-1ubuntu1.11-Ubuntu <<>> ${args.query}
;; Got answer:
;; ->>HEADER<<- opcode: ${opCode}, status: ${RCODE}, id: ${id}
;; flags: ${getFlags(message)}; QUESTION: ${qcount}, ANSWER: ${ancount}, AUTHORITY: ${nscount}, ADDITIONAL: ${arcount}

;; QUESTION SECTION:
;${question[0].name.join('.').slice(0, 36).padEnd(37, ' ')}${qclasses[question[0].qclass].padEnd(4, ' ')}${qtypes[question[0].qtype].padEnd(8, ' ')}

;; ANSWER SECTION:
${answer.map(getAnswer).join('\n')}

;; Query time: ${elapsed} msec
;; SERVER: ${address}#${port}(${address})
;; WHEN: ${(new Date()).toUTCString()}
;; MSG SIZE  rcvd: ${length}`)
}

run()
