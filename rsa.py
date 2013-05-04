import random
import math

small_primes = (
    3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
    233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
    283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
    353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
    419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
    467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
    547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
    607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
    661, 673, 677, 683, 691, 701, 709, 719, 727, 733,
    739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
    811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
    877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
    947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013,
    1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069,
    1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151,
    1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
    1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291
)

fpt_trials = 50

def bit_len(x):
    return int(math.ceil(math.log(x, 2)))

def byte_len(x):
    return (bit_len(x) + 7) // 8

def modexp(n, e, p):
    return pow(n, e, p)

def perhaps_prime(x):
    # trial division
    for p in small_primes:
        if (x % p) == 0:
            return False
    return True

def probably_prime(x):
    # fermat's little theorem
    for i in range(fpt_trials):
        witness = random.randrange(1, x - 1)
        w = modexp(witness, x - 1, x)
        if w == 1:
            continue
        else:
            return False
    return True

def extended_euclidian(a, b):
    if b == 0:
        return (a, 1, 0)
    assert a > 0 and b > 0

    k = abs(a % b)
    n = a // b
    d, k, l = extended_euclidian(b, k)

    return (d, l, k - l * n)

def invmod(x, y):
    _, i, _ = extended_euclidian(x, y)
    return i
    
def cuberoot(y):
    root = 3
    guess = 1
    
    while True:
        if guess ** root == y:
            return guess
        
        xk = guess ** (root - 1)
        guess = ((root - 1) * guess + (y / xk)) / root
  
def gen_prime(x):
    while 1:
        candidate = random.getrandbits(x)
        candidate |= 1
        candidate |= 1 << (x - 1)
        assert bit_len(candidate) == x
        if perhaps_prime(candidate) and probably_prime(candidate):
            return candidate

def gen_rsa(x, e = 65537):
    done = False
    while not done:
        n = 1
        while bit_len(n) != x:
            p = gen_prime(x // 2)
            q = gen_prime(x - bit_len(p))
            n = p * q
        
        phi = (p - 1) * (q - 1)
        d = invmod(e, phi)
        d = d % n
        done = (e * d) % phi == 1
        
    return (e, n), (d, n)

def raw_encrypt(pub, m):
    e, n = pub
    return modexp(m, e, n)

def raw_decrypt(priv, m):
    d, n = priv
    return modexp(m, d, n)
