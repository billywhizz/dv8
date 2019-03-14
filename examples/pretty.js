const ANSI_RED = '\u001b[31m'
const ANSI_MAGENTA = '\u001b[35m'
const ANSI_DEFAULT = '\u001b[0m'
const ANSI_CYAN = '\u001b[36m'
const ANSI_GREEN = '\u001b[32m'

String.prototype.magenta = function (pad) { return `${ANSI_MAGENTA}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
String.prototype.red = function (pad) { return `${ANSI_RED}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
String.prototype.green = function (pad) { return `${ANSI_GREEN}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
String.prototype.cyan = function (pad) { return `${ANSI_CYAN}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }

function prettyPrint(obj, indent = 0) {
  for (const key of Object.keys(obj)) {
    print(`${key.cyan(indent)} : `)
    prettyPrint(obj[key], indent + 2)
  }
}

prettyPrint({
  name: 'foo'
})
