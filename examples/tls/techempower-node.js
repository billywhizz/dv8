const http = require('http')
const parseurl = require('parseurl')

const routing = require('./routing')
const basicHandler = routing.BasicHandler
const queryHandler = routing.QueryHandler
const routeNotImplemented = require('./helper').responses.routeNotImplemented

module.exports = http.createServer(function (req, res) {
  const url = parseurl(req)
  const route = url.pathname
  if (basicHandler.has(route)) {
    return basicHandler.handle(route, req, res)
  } else {
    let queries = url.query && url.query.split('=')[1]
    queries = ~~(queries) || 1
    queries = Math.min(Math.max(queries, 1), 500)
    if (queryHandler.has(route)) {
      return queryHandler.handle(route, queries, req, res)
    } else {
      return routeNotImplemented(req, res)
    }
  }
}).listen(8080, () => console.log('NodeJS worker listening on port 8080'))
