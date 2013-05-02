import rsa
import decimal

def invmod(x, y):
    _, i, _ = rsa.extended_euclidian(x, y)
    return i
    
def cuberoot(y):
    # this is pretty much bollocks. nevermind.
    dy = decimal.Decimal(y)
    with decimal.localcontext() as ctx:
        ctx.prec = rsa.bit_len(y) * 3
        p = ctx.divide(decimal.Decimal(1), decimal.Decimal(3))
        i = ctx.power(dy, p)
    return long(i)
    
if __name__ == '__main__':
    pubkeys = []
    for _ in range(3):
        pub, priv = rsa.gen_rsa(1024, 3)
        pubkeys.append(pub)
    m = 0x1235123
    
    ciphertexts = []
    for pub in pubkeys:
        e = rsa.raw_encrypt(pub, m)
        ciphertexts.append(e)
        
    # pubkeys are e, n tuples
    moduli = [pubkeys[x][1] for x in range(3)]
    
    m_s = [
        moduli[1] * moduli[2],
        moduli[0] * moduli[2],
        moduli[0] * moduli[1]
    ]
    n_012 = moduli[0] * moduli[1] * moduli[2]
    
    magic = sum(ciphertexts[i] * m_s[i] * invmod(m_s[i], moduli[i]) for i in range(3))
    assert m - 1 == cuberoot(magic)
    print 'ok'