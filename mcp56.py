from rc4 import rc4
import os

# nb. this is unused, python is a little too slow to do 2 ** 24 * 30 of anything
# before my hair turns grey. see mcp56.c instead

# assume we know this (we can find it out with oracle('') if not...)
secret_length = 30

def oracle(req):
    key = os.urandom(16)
    cookie = 'QkUgU1VSRSBUTyBEUklOSyBZT1VSIE9WQUxUSU5F'.decode('base64')

    return rc4(key).encrypt(req + cookie)

def winner(byte):
    highest_index = 0
    highest_score = 0
    for i, score in enumerate(byte):
        if score > highest_score:
            highest_score = score
            highest_index = i
    return chr(highest_index)

def recover(counts):
    msg = ''
    for byte in counts:
        msg += winner(byte)
    return msg

def recover_cookie(f):
    zero = [0] * 256
    counts = [list(zero) for i in range(secret_length)]

    FULL_WEIGHT = 4
    HALF_WEIGHT = 1

    prefix = 'AA'
    while len(prefix) < 16:
        lp = len(prefix)
        for r in xrange(2 ** 16):
            ct = f(prefix)

            # bias at 16 towards 240 (full) 0 (half) 16 (half)
            b16 = ord(ct[15])
            byte = counts[15 - lp]
            byte[b16 ^ 240] += FULL_WEIGHT
            byte[b16 ^ 0] += HALF_WEIGHT
            byte[b16 ^ 16] += HALF_WEIGHT

            # bias at 32 towards 224 (full) 0 (half) 32 (half)
            b32 = ord(ct[31])
            byte = counts[31 - lp]
            byte[b32 ^ 224] += FULL_WEIGHT
            byte[b32 ^ 0] += HALF_WEIGHT
            byte[b32 ^ 32] + HALF_WEIGHT
        print prefix
        print recover(counts)
        prefix += 'A'

if __name__ == '__main__':
    recover_cookie(oracle)
