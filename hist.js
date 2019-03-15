/*
> At first glance this looks like a linear histogram, but take a look at the 1
million point on the X axis. You’ll notice a change in bin size by a factor of
10. Where there was a bin from 990k to 1M, the next bin spans 1M to 1.1M. Each
power of 10 contains 90 bins evenly spaced. Why not 100 bins you might think?
Because the lower bound isn’t zero. In the case of a lower bound of 1 and an
upper bound of 10, 10-1 = 9, and spacing those in 0.1 increments yields 90 bins.

Results:
so
bin 0 = [ 0 - 9 ]
bin 1 = [ 10 - 999 ]
bin 2 = [ 100 - 9999 ]
bin 3 = [ 1000 - 99999 ]
bin 4 = [ 10000 - 999999 ]
*/

function getBin (value, base = 10, bins = 10) {
  let bin = 0
  let i = bins
  while (i--) {
    if (value < Math.pow(base, bin)) {
      return bin
    }
    bin++
  }
}

function store (value) {
  console.log(`${value.toString().padEnd(10)} ${getBin(value)}`)
}

console.log(`${'value'.padEnd(10)} bin`)
console.log('-'.repeat(20))

const values = [100, 100, 50, 50, 200, 100, 50, 50, 50, 20, 20, 20, 10, 10, 10, 5, 5, 5, 5, 5, 1000, 2000, 3000, 4000, 4000, 10000, 2000, 1000, 500, 200, 500, 500, 100, 100, 100, 50, 100, 100, 50, 50]

values.forEach(v => store(v))
