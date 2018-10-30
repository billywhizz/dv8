const { UnauthorizedError } = require('./faas.js')

function authenticate(request) {
    const { authorization } = request.headers
    const token = authorization.match(/Bearer (.+)/)[1]
    if (!token) throw new UnauthorizedError('Unauthorized')
}

async function hello(context) {
    const { request, response } = context
    const { user } = await authenticate(request)
    context.done({})
}

module.exports = { hello }
