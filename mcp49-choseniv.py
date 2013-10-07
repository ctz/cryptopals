import aes
import os

shared_key = '0123456789abcdef'

def aes_cbcmac(key, msg, iv = '\0' * 16):
    return aes.encryptData(key, msg, iv = iv)[-16:]

def parse_qs(v):
    parts = v.split('&')
    d = dict()
    for p in parts:
        if len(p) == 0:
            continue
        assert '=' in p
        k, v = p.split('=', 1)
        d[k] = v
    return d

def server_verify(msg):
    msg, iv, mac = msg[:-32], msg[-32:-16], msg[-16:]
    if aes_cbcmac(shared_key, msg, iv) != mac:
        print 'verify failed'
        return None
    else:
        data = parse_qs(msg)
        data = dict((k, int(v)) for k, v in data.items())
        return data

def print_txn(txn):
    print 'transferred %(amount)d spacebucks from account %(from)d to account %(to)d' % txn

def client_sign(amount, fromid, toid):
    msg = 'from=%d&to=%d&amount=%d' % (fromid, toid, amount)
    iv = os.urandom(16)
    return msg + iv + aes_cbcmac(shared_key, msg, iv)

if __name__ == '__main__':
    MY_ACCOUNT = 1
    THEIR_ACCOUNT = 2
    OTHER_ACCOUNT = 3

    # this is our target message
    msg = client_sign(1000000, THEIR_ACCOUNT, OTHER_ACCOUNT)
    
    # check it verifies
    assert server_verify(msg)

    offset = len('from=d&to=')
    bb = map(ord, msg)
    difference = ord(str(OTHER_ACCOUNT)) ^ ord(str(MY_ACCOUNT))

    # twiddle first block, and iv to compensate
    bb[offset] ^= difference
    bb[-32+offset] ^= difference

    forged_msg = ''.join(chr(b) for b in bb)
    txn = server_verify(forged_msg)
    assert txn
    print_txn(txn)
    
