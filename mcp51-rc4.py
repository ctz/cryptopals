import rc4
import zlib

whatever_key = 'iaksmdasomqwemaosmdsadakml'

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
    return rc4.rc4(map(ord, whatever_key)).encrypt(map(ord, msg))

def oracle(msg):
    return len(encrypt(compress(format_request(msg))))

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
                score = oracle('Cookie: sessionid=' + guess)

                if score < best_score:
                    best_guess = guess
                    best_score = score

        assert best_guess is not None
        guessed_sessionid = best_guess
    
    print guessed_sessionid[:sessionid_length] + '='
