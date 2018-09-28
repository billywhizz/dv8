const { TTY } = process.binding('tty_wrap')

const context = {}
const stdin = new TTY(0, true, context)
let bytes = 0

stdin.onread = (err, b) => {
    if (b) {
        bytes += b.length
    } else {
        console.log(bytes)
    }
}
stdin.readStart()
