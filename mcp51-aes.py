import aes
import zlib

whatever_key = 'a' * 16

def format_request(msg):
    return 'POST / HTTP/1.1\r\n' + \
           'Host: hapless.com\r\n' + \
           'Cookie: sessionid=TmV2ZXIgcmV2ZWFsIHRoZSBXdS1UYW5nIFNlY3JldCE=\r\n' + \
           'Content-Length: %d\r\n' % len(msg) + \
           '\r\n' + \
           msg

def compress(msg):
    return zlib.compress(msg)

def encrypt(msg):
    # sorry, the pure python aes i found under a rock is grotesquely slow.
    # (it doesn't do key scheduling outside of individual encryptions!)
    # return aes.encryptData(whatever_key, msg)

    padlen = 16 - (len(msg) % 16)
    return msg + chr(padlen) * padlen
    
def oracle(msg):
    return len(encrypt(compress(format_request(msg))))

def block_cipher_oracle(root):
    # block cipher padding introduces a new problem: we don't have good length resolution
    # try to fix this by extending the guess by some characters which don't appear the the framing
    # and seeing when the result grows by a block size

    length_adjust = '\xf0' * 32
    original_length = oracle(root)
    
    for i in range(1,17):
        length_now = oracle(root + length_adjust[:i])
        if length_now != original_length:
            return original_length - i

valid_b64 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'

if __name__ == '__main__':
    # we'll guess pairs of base64 characters (excluding the padding character, which we can fixup later)
    
    guessed_sessionid = ''
    sessionid_length = 43 # assume we can learn this length (if not we can guess another pair: \r\n)

    while len(guessed_sessionid) < sessionid_length:
        best_score = 0xffff
        best_guess = None
        
        for x in range(64):
            for y in range(64):
                guess = guessed_sessionid + valid_b64[x] + valid_b64[y]
                score = block_cipher_oracle('Cookie: sessionid=' + guess)
                
                if score < best_score:
                    best_guess = guess
                    best_score = score

        assert best_guess is not None
        guessed_sessionid = best_guess
    
    print guessed_sessionid[:sessionid_length] + '='
