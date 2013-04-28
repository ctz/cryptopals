import web
from hashlib import sha1
import time

HMAC_KEY = 'b9e2eae7ba44e2a40ddbf82402759e009d7e186f041d1a57bb4231d41039a1be'.decode('hex')
WAIT_TIME = 50 * 1e-3

urls = (
  '/test/(.+)/(.+)', 'test'
)

def sha1hmac(key, msg):
    blocksz = 64
    if len(key) < blocksz:
        key = key + ('\x00' * (blocksz - len(key)))
    
    o_key_pad = ''.join([chr(0x5c ^ ord(y)) for y in key])
    i_key_pad = ''.join([chr(0x36 ^ ord(y)) for y in key])
    
    inner = i_key_pad + str(msg)
    return sha1(o_key_pad + sha1(inner).digest()).digest()

def insecure_compare(x, y):
    if len(x) != len(y):
        return False
    
    for xx, yy in zip(x, y):
        if xx != yy:
            return False
        time.sleep(WAIT_TIME)
    return True
    
class test:
    def GET(self, file, sig):
        sig = sig.decode('hex')
        real_sig = sha1hmac(HMAC_KEY, file)
        if insecure_compare(sig, real_sig):
            return 'ok'
        else:
            raise web.internalerror('bad signature')

if __name__ == '__main__':
    app = web.application(urls, globals())
    app.run()