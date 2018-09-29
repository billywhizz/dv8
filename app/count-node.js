const { TTY } = process.binding('tty_wrap')
const stdin = new TTY(0, true, {})
let bytes = 0
stdin.onread = (err, b) => {
    if (b) return bytes += b.length
    console.log(bytes)
}
stdin.readStart()
