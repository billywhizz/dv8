if (process.env.MODE === 'tls') {
  const { SecureContext, SecureSocket } = module('openssl', {})
  const certStore = env.CERT_STORE || './certs'
  const contexts = {}
  let defaultContext

  const setSecure = (sock, onSecure, currentContext = defaultContext) => {
    const secureClient = new SecureSocket()
    secureClient.onHost(hostname => {
      if (!hostname) return
      const context = contexts[hostname]
      if (!context) return false
      if (context.hostname === currentContext.hostname) return
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

  const addContext = (hostname, isServer = true) => {
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
} else {
  module.exports = { setSecure: (sock, onSecure) => { if (onSecure) onSecure() }, addContext: () => { }, getContext: () => { }, deleteContext: () => { } }
}
