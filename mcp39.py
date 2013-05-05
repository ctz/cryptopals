import rsa
    
if __name__ == '__main__':
    pub, priv = rsa.gen_rsa(1024, rsa.PUBLIC_EXP)
    m = 0x1235123
    e = rsa.raw_encrypt(pub, m)
    assert rsa.raw_decrypt(priv, e) == m
    print 'ok'
    