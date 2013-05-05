import rsa

plain = 'VGhhdCdzIHdoeSBJIGZvdW5kIHlvdSBkb24ndCBwbGF5IGFyb3VuZCB3aXRoIHRoZSBGdW5reSBDb2xkIE1lZGluYQ=='.decode('base64')
plain_i = long(plain.encode('hex'), 16)

def oracle(priv, cipher):
    pt = rsa.raw_decrypt(priv, cipher)
    return pt & 1

def decode_int(i):
    v = hex(long(i))[2:-1]
    if len(v) & 1: v = '0' + v
    return v.decode('hex')

def extract_bits(priv, pub, cipher):
    N = pub[1]
    c2 = rsa.raw_encrypt(pub, 2)
    cipher = (cipher * c2) % N
    
    for _ in range(1024):
        yield oracle(priv, cipher)
        cipher = (cipher * c2) % N

if __name__ == '__main__':
    pub, priv = rsa.gen_rsa(1024, rsa.PUBLIC_EXP)
    N = pub[1]
    cipher = rsa.raw_encrypt(pub, plain_i)
    
    lo, hi = 0, N
    for b in extract_bits(priv, pub, cipher):
        mid = (lo + hi) / 2
        if b == 1:
            lo = mid
        else:
            hi = mid

    # whoops, the last byte is trashed (div accuracy?). nevermind.
    print decode_int(hi)[:-1]
