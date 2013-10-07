import aes
import os

mac_key = 'YELLOW SUBMARINE'

def aes_cbcmac(key, msg):
    cipher = aes.encryptData(key, msg, iv = '\0' * 16)
    return cipher[-16:]

def aes_cbcmac_nopad(key, msg):
    cipher = aes.encryptData(key, msg, iv = '\0' * 16, pad = False)
    return cipher[-16:]

def aes_ecb_decrypt(key, block):
    adjustment = aes.AES().decrypt(map(ord, block), map(ord, key), len(key))
    return ''.join(chr(x) for x in adjustment)

def xor_block(a, b):
    assert len(a) == len(b)
    return ''.join(chr(ord(a[i]) ^ ord(b[i])) for i in range(len(a)))

UNACCEPTABLE = ''.join(chr(x) for x in range(9)) + '\n'

def roughly_printable(msg):
    for m in msg:
        if m in UNACCEPTABLE:
            print 'bad', repr(m)
            return False
    return True

if __name__ == '__main__':
    good_code = "alert('MZA who was that?');\n"
    target_mac = '296b8d7cb78a243dda4d0a61d33bbdd1'.decode('hex')

    assert target_mac == aes_cbcmac(mac_key, good_code)
    
    # we have a 4 byte counter on the end of bad_code, to search for an adjustment block
    # which doesn't contain control characters
    bad_code_prefix = "alert('Ayo, the Wu is back!'); // A"
    assert roughly_printable(bad_code_prefix)
    counter_format = '%08x'
    counter_sz = 4
    
    bad_pad = '\t' * 9
    assert 9 == 16 - (len(bad_code_prefix) + counter_sz) % 16
    
    # now we need to add an adjustment block, such that adjustment = D(target_mac) ^ bad_mac
    adjustment = aes_ecb_decrypt(mac_key, target_mac)

    counter = 0x20202020

    while True:
        counter += 1
        counter_block = counter_format % counter

        bad_code = bad_code_prefix + counter_block.decode('hex')
        bad_mac = aes_cbcmac(mac_key, bad_code)

        # now we add a that adjustment block to the bad code
        attack = bad_code + bad_pad + xor_block(bad_mac, adjustment)
        assert target_mac == aes_cbcmac_nopad(mac_key, attack)

        if roughly_printable(attack):
            f = open('test50.html', 'w')
            print >>f, '<html><body><script>'
            print >>f, attack
            print >>f, '</script></body></html>'
            f.close()
            print attack.encode('hex')
            break
