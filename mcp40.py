import rsa

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
    
    magic = 0
    for i in range(3):
        inv = rsa.invmod(m_s[i], moduli[i])
        magic += ciphertexts[i] * m_s[i] * inv
        magic %= n_012
    assert m == rsa.cuberoot(magic)
    print 'ok'