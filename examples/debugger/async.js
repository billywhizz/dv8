async function foo(x) {
  await bar(x);
}

async function bar(x) {
  await x;
  throw new Error("Let's have a look...");
}

foo(1).catch(e => console.log(JSON.stringify(e.stack)));
