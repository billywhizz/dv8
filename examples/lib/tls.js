const { SecureContext, SecureSocket } = module('openssl', {})

const contexts = {}
let defaultContext

const setSecure = (sock, onSecure, currentContext = defaultContext) => {
  const secureClient = new SecureSocket()
  secureClient.onHost(hostname => {
    if (!hostname) return false
    const context = contexts[hostname]
    if (!context) return false
    print(context.hostname)
    print(currentContext.hostname)
    if (context.hostname === currentContext.hostname) return context
    return context
  })
  secureClient.onError((code, message) => {
    print(`SSL Error (${code}): ${message}`)
    sock.close()
  })
  sock.start = () => secureClient.start()
  sock.write = len => secureClient.write(len)
  secureClient.setup(currentContext, sock)
  if (onSecure) secureClient.onSecure(onSecure)
}

const addContext = (hostname, { isServer = true, certStore = './certs' }) => {
  const secureContext = new SecureContext()
  if (isServer) {
    secureContext.setup(0, `${certStore}/${hostname}.cert.pem`, `${certStore}/${hostname}.key.pem`)
  } else {
    secureContext.setup(1)
  }
  if (!defaultContext) defaultContext = secureContext
  secureContext.hostname = hostname
  contexts[hostname] = secureContext
  return secureContext
}

const getContext = hostname => contexts[hostname]
const deleteContext = hostname => (delete contexts[hostname])

module.exports = { setSecure, addContext, getContext, deleteContext }
