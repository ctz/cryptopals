import dsa
import rsa

p = 0x800000000000000089e1855218a0e7dac38136ffafa72eda7859f2171e25e65eac698c1702578b07dc2a1076da241c76c62d374d8389ea5aeffd3226a0530cc565f3bf6b50929139ebeac04f48c3c84afb796d61e5a4f9a8fda812ab59494232c7d2b4deb50aa18ee9e132bfa85ac4374d7f9091abc3d015efc871a584471bb1
q = 0xf4f47f05794b256174bba6e9b396a7707e563c5b

if __name__ == '__main__':
    # er, g = 0 means all signature r values will be 0 -- signature generation won't halt
    # because that case is detected and retried with another k value
    #
    # for signature validation, the signature is rejected early because r < 1
    
    group = dsa.group(p, q, p + 1)
    pub, priv = dsa.gen_pair(group)
    
    z = 1
    y = pub[1]
    
    r = pow(y, z, group.p) % group.q
    zinv = rsa.invmod(z, group.q)
    s = (r * zinv) % group.q
    magic_sig = (r, s)
    
    dsa.verify_sha1(pub, magic_sig, 'hello world')
    dsa.verify_sha1(pub, magic_sig, 'goodbye world')
    print 'ok'