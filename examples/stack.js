function stackHandler (err, stack) {
  try {
    const keys = Object.getOwnPropertyNames(err)
    for (const key of keys) {
      print(`${key.padEnd(20, ' ').slice(0, 20)} : ${err[key]}`)
    }
    for (const callsite of stack) {
      const typeName = callsite.getTypeName()
      const functionName = callsite.getFunctionName()
      const methodName = callsite.getMethodName()
      const fileName = callsite.getFileName()
      const lineNumber = callsite.getLineNumber()
      const columnNumber = callsite.getColumnNumber()
      const isToplevel = callsite.isToplevel()
      const isEval = callsite.isEval()
      const isNative = callsite.isEval()
      const isConstructor = callsite.isConstructor()
      print(JSON.stringify({ typeName, functionName, methodName, fileName, lineNumber, columnNumber, isToplevel, isEval, isNative, isConstructor }))
    }
  } catch (err) {
    print(err.message)
    print(err.stack)
  }
}

function eventuallyThrow (counter) {
  if (!counter--) throw new Error('Eventually')
  return process.nextTick(() => eventuallyThrow(counter))
}

Error.prepareStackTrace = stackHandler
Error.stackTraceLimit = Infinity

process.onExit = () => {
  print(JSON.stringify(process.activeHandles(), null, '  '))
}

global.onUncaughtException = () => {}

eventuallyThrow(parseInt(process.args[2] || 3, 10))
