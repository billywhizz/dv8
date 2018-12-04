Error.stackTraceLimit = Infinity
/*
function stackHandler(err, stack) {
  const keys = Object.getOwnPropertyNames(stack)
  for (const key of keys) {
    print(`${key.padEnd(20, ' ').slice(0, 20)} : ${stack[key]}`)
    const keys2 = Object.getOwnPropertyNames(stack[key])
    for (const key2 of keys2) {
      print(`${key2.padEnd(20, ' ').slice(0, 20)} : ${stack[key][key2]}`)
    }
  }
  print(JSON.stringify(stack))
}
*/

global.onUncaughtException = () => {}

function stackHandler2(err, stack) {
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

Error.prepareStackTrace = stackHandler2

function eventuallyThrow (counter) {
  if (!counter--) throw new Error('Eventually')
  //return process.nextTick(() => eventuallyThrow(counter))
  return eventuallyThrow(counter)
}

eventuallyThrow(3)
