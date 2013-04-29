import random
import aes
from hashlib import sha1

p = 0xffffffffffffffffc90fdaa22168c234c4c6628b80dc1cd129024e088a67cc74020bbea63b139b22514a08798e3404ddef9519b3cd3a431b302b0a6df25f14374fe1356d6d51c245e485b576625e7ec6f44c42e9a637ed6b0bff5cb6f406b7edee386bfb5a899fa5ae9f24117c4b1fe649286651ece45b3dc2007cb8a163bf0598da48361c55d39a69163fa8fd24cf5f83655d23dca3ad961c62f356208552bb9ed529077096966d670c354e4abc9804f1746c08ca237327ffffffffffffffff
g = 2

def keygen():
    a = random.randrange(1, p-1)
    A = pow(g, a, p)
    return a, A

def derivekey(s):
    return sha1(hex(long(s))).digest()[0:16]

def basic_protocol():
    # A->B            Send "p", "g", "A"
    a, A = keygen()

    # B->A            Send "B"
    b, B = keygen()

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

def mitm_protocol():
    # A->M            Send "p", "g", "A"
    a, A = keygen()
    
    # M->B            Send "p", "g", "p"
    B_idea_of_A = p
    
    # B->M            Send "B"
    b, B = keygen()
    
    # M->A            Send "p"
    A_idea_of_B = p
    
    # A->M            Send AES-CBC(SHA1(s)[0:16], iv=random(16), msg) + iv
    s_A = pow(A_idea_of_B, a, p)
    aeskey_A = derivekey(s_A)
    msg_A = 'hello world'
    cipher_A = aes.encryptData(aeskey_A, msg_A)
    
    # M->B            Relay that to B
    # (check we can decrypt)
    aeskey_M = derivekey(0)
    msg_AM = aes.decryptData(aeskey_M, cipher_A)
    assert msg_AM == msg_A
    
    # B->M            Send AES-CBC(SHA1(s)[0:16], iv=random(16), A's msg) + iv
    s_B = pow(B_idea_of_A, b, p)
    aeskey_B = derivekey(s_B)
    msg_B = aes.decryptData(aeskey_B, cipher_A)
    cipher_B = aes.encryptData(aeskey_B, msg_B)
    
    # M->A            Relay that to A
    msg_BM = aes.decryptData(aeskey_M, cipher_B)
    assert msg_BM == msg_A
    assert aeskey_M == aeskey_A

if __name__ == '__main__':
    basic_protocol()
    mitm_protocol()
    
    
    print('ok')
