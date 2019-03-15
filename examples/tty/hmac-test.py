import hmac, hashlib, sys
digest = hmac.new(b'mybigsecret', b'', hashlib.sha512)
while True:
    buf = sys.stdin.buffer.read(65536)
    if not buf:
        break
    digest.update(buf)
print(digest.hexdigest())
