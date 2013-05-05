import random
import rsa
from hashlib import sha1

class group(object):
    def __init__(self, p, q, g):
        self.p = p
        self.q = q
        self.g = g

def gen_pair(group):
    x = random.randrange(1, group.q - 1)
    y = pow(group.g, x, group.p)
    return (group, y), (group, x)

def hash(msg):
    return long(sha1(msg).hexdigest(), 16)
    
def sign_sha1(priv, msg):
    group, x = priv
    h = hash(msg)
    
    while True:
        k = random.randrange(1, group.q - 1)
        r = pow(group.g, k, group.p) % group.q
        if r == 0:
            continue
        
        kinv = rsa.invmod(k, group.q)
        s = (kinv * (h + x * r)) % group.q
        if s == 0:
            continue
        return (r, s)

def verify_sha1(pub, sig, msg):
    group, y = pub
    r, s = sig
    h = hash(msg)
    
    if not (r > 0 and r < group.q and s > 0 and s < group.q):
        raise ValueError, 'invalid dsa signature'
    
    w = rsa.invmod(s, group.q)
    u1 = (h * w) % group.q
    u2 = (r * w) % group.q
    v = (pow(group.g, u1, group.p) * pow(y, u2, group.p)) % group.p
    v %= group.q
    
    if v != r:
        raise ValueError, 'invalid dsa signature'

def recover_x_given_sig_k(group, k, sig, msg):
    r, s = sig
    rinv = rsa.invmod(r, group.q)
    h = hash(msg)
    x = ((s * k - h) * rinv) % group.q
    return x