import random
import aes
from hashlib import sha1

p = 0xffffffffffffffffc90fdaa22168c234c4c6628b80dc1cd129024e088a67cc74020bbea63b139b22514a08798e3404ddef9519b3cd3a431b302b0a6df25f14374fe1356d6d51c245e485b576625e7ec6f44c42e9a637ed6b0bff5cb6f406b7edee386bfb5a899fa5ae9f24117c4b1fe649286651ece45b3dc2007cb8a163bf0598da48361c55d39a69163fa8fd24cf5f83655d23dca3ad961c62f356208552bb9ed529077096966d670c354e4abc9804f1746c08ca237327ffffffffffffffff
g = 2

def keygen(p, g):
    a = random.randrange(1, p-1)
    A = pow(g, a, p)
    return a, A

def derivekey(s):
    return sha1(hex(long(s))).digest()[0:16]

def basic_protocol(p, g):
    # A->B            Send "p", "g", "A"
    a, A = keygen(p, g)

    # B->A            Send "B"
    b, B = keygen(p, g)

    # A->B            Send AES-CBC(SHA1(s)[0:16], iv=random(16), msg) + iv
    s_A = pow(B, a, p)
    aeskey_A = derivekey(s_A)    
    msg_A = 'hello world'
    cipher_A = aes.encryptData(aeskey_A, msg_A)
    
    # B->A            Send AES-CBC(SHA1(s)[0:16], iv=random(16), A's msg) + iv
    s_B = pow(A, b, p)
    aeskey_B = derivekey(s_B)
    msg_B = aes.decryptData(aeskey_B, cipher_A)
    cipher_B = aes.encryptData(aeskey_B, msg_B)

    # (A checks B's msg?)
    check_A = aes.decryptData(aeskey_A, cipher_B)
    assert msg_A == check_A and msg_B == msg_A

if __name__ == '__main__':
    basic_protocol(p, g) # check
    basic_protocol(p, 1)
    basic_protocol(p, p)
    basic_protocol(p, p - 1)
    
    print('ok')
