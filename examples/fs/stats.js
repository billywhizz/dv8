function swap (data, i, j) {
  if (i === j) return
  const tmp = data[j]
  data[j] = data[i]
  data[i] = tmp
}

function partition (data, start, end) {
  let i
  let j
  for (i = start + 1, j = start; i < end; i++) {
    if (data[i] < data[start]) {
      swap(data, i, ++j)
    }
  }
  swap(data, start, j)
  return j
}

function findK (data, s, e, k) {
  let start = s
  let end = e
  while (start < end) {
    const pos = partition(data, start, end)
    if (pos === k) {
      return data[k]
    }
    if (pos > k) {
      end = pos
    } else {
      start = pos + 1
    }
  }
  return null
}

const percentile = (data, n) => {
  return findK(data.concat(), 0, data.length, Math.ceil((data.length * n) / 100) - 1)
}

function getPercentiles (values, step = 2) {
  const percentiles = []
  for (let i = step; i <= 100; i += step) {
    percentiles.unshift({ p: i, value: percentile(values, i) })
  }
  return percentiles
}

module.exports = { getPercentiles }
