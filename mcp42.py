import rsa
from hashlib import sha1
from re import match

ciphers = []

asn1_sha1_prefix = '3021300906052b0e03021a05000414'

def pkcs1_sign(priv, msg):
    d, n = priv
    
    modlen = rsa.byte_len(n)
    h = sha1(msg).hexdigest()
    
    npad = modlen - 2 - 1 - len(asn1_sha1_prefix + h) / 2
    
    mr = '0001' + ('ff' * npad) + '00' + asn1_sha1_prefix + h
    mr = long(mr, 16)
    return rsa.raw_decrypt(priv, mr)

def bad_pkcs1_verify(pub, sig, msg):
    e, n = pub
    modlen = rsa.byte_len(n)
    mr = rsa.raw_encrypt(pub, sig)
    h = sha1(msg).hexdigest().lower()
    
    mrh = ('%0' + str(modlen * 2) + 'x') % mr
    if match('^0001ff+00' + asn1_sha1_prefix + h, mrh):
        return 'ok'
    else:
        return 'bad signature'

if __name__ == '__main__':
    pub, priv = rsa.gen_rsa(1024, 3)
    msg = 'hi mom'
    
    # check
    good_sig = pkcs1_sign(priv, msg)
    assert 'ok' == bad_pkcs1_verify(pub, good_sig, msg)
    
    # forge
    mr = '0001ff00' + asn1_sha1_prefix + sha1(msg).hexdigest()
    # right-pad to approx half the interval
    mr += '7f' * (128 - 4 - len(asn1_sha1_prefix) / 2 - 20)
    mr = long(mr, 16)
    
    near_root = rsa.cuberoot_approx(mr)
    mr = near_root ** 3
    bad_sig = rsa.cuberoot(mr)
    print bad_pkcs1_verify(pub, bad_sig, msg)