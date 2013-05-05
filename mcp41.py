import rsa
import random

ciphers = []

def decrypt_once(priv, ct):
    global ciphers
    
    if ct in ciphers:
        return None
    ciphers.append(ct)
    
    return rsa.raw_decrypt(priv, ct)

if __name__ == '__main__':
    pub, priv = rsa.gen_rsa(1024, rsa.PUBLIC_EXP)
    m = 0x12351234
    
    ct = rsa.raw_encrypt(pub, m)
    assert decrypt_once(priv, ct) == m
    assert decrypt_once(priv, ct) is None
    
    N = pub[1]
    S = random.randrange(1, N)
    S_ct = rsa.raw_encrypt(pub, S)
    S_pt = decrypt_once(priv, (S_ct * ct) % N)
    S_inv = rsa.invmod(S, N)
    assert (S_inv * S_pt) % N == m
    print 'ok'