#!/bin/sh

i=0

function ensure()
{
  if [ "$1" != "$2" ] ; then
    echo "test failed:"
    echo "  '$1'"
    echo "is not the same as:"
    echo "  '$2'"
    exit 1
  fi
  
  i=$(($i + 1))
  echo "test $i: OK"
}

# // ------------------------------------------------------------
# 
# 1. Convert hex to base64 and back.
# 
# The string:
# 
#   49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d
# 
# should produce:
# 
#   SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t
# 
# Now use this code everywhere for the rest of the exercises. Here's a
# simple rule of thumb:
# 
#   Always operate on raw bytes, never on encoded strings. Only use hex
#   and base64 for pretty-printing.
# 
ensure `bin/mcp1 49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d` "SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t"

# // ------------------------------------------------------------
# 
# 2. Fixed XOR
# 
# Write a function that takes two equal-length buffers and produces
# their XOR sum.
# 
# The string:
# 
#  1c0111001f010100061a024b53535009181c
# 
# ... after hex decoding, when xor'd against:
# 
#  686974207468652062756c6c277320657965
# 
# ... should produce:
# 
#  746865206b696420646f6e277420706c6179
# 
ensure `bin/mcp2 1c0111001f010100061a024b53535009181c 686974207468652062756c6c277320657965` "746865206b696420646f6e277420706c6179"

# // ------------------------------------------------------------
# 
# 3. Single-character XOR Cipher
# 
# The hex encoded string:
# 
#       1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736
# 
# ... has been XOR'd against a single character. Find the key, decrypt
# the message.
# 
# Write code to do this for you. How? Devise some method for "scoring" a
# piece of English plaintext. (Character frequency is a good metric.)
# Evaluate each output and choose the one with the best score.
# 
# Tune your algorithm until this works.
# 
ensure "`bin/mcp3 1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736`" "key: 58, msg: Cooking MC's like a pound of bacon"

# // ------------------------------------------------------------
# 
# 4. Detect single-character XOR
# 
# One of the 60-character strings at:
# 
#   https://gist.github.com/3132713
# 
# has been encrypted by single-character XOR. Find it. (Your code from
# #3 should help.)
# 
inp=`cat gistfile1.txt`
ensure "`bin/mcp4 $inp`" "cipher: 7b5a4215415d544115415d5015455447414c155c46155f4058455c5b523f, key: 35, msg: Now that the party is jumping?"

# // ------------------------------------------------------------
# 
# 5. Repeating-key XOR Cipher
# 
# Write the code to encrypt the string:
# 
#   Burning 'em, if you ain't quick and nimble
#   I go crazy when I hear a cymbal
# 
# Under the key "ICE", using repeating-key XOR. It should come out to:
# 
#   0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324272765272a282b2f20430a652e2c652a3124333a653e2b2027630c692b20283165286326302e27282f
# 
# Encrypt a bunch of stuff using your repeating-key XOR function. Get a
# feel for it.
# 
PLAIN="4275726e696e672027656d2c20696620796f752061696e277420717569636b20616e64206e696d626c650a4920676f206372617a79207768656e2049206865617220612063796d62616c"
ICE="494345"
ensure `bin/mcp2 $PLAIN $ICE` "0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324272765272a282b2f20430a652e2c652a3124333a653e2b2027630c692b20283165286326302e27282f"

# // ------------------------------------------------------------
# 
# 6. Break repeating-key XOR
# 
# The buffer at the following location:
# 
#  https://gist.github.com/3132752
# 
# is base64-encoded repeating-key XOR. Break it.
# 
# Here's how:
# 
# a. Let KEYSIZE be the guessed length of the key; try values from 2 to
# (say) 40.
# 
# b. Write a function to compute the edit distance/Hamming distance
# between two strings. The Hamming distance is just the number of
# differing bits. The distance between:
# 
#   this is a test
# 
# and:
# 
#   wokka wokka!!!
# 
# is 37.
# 
# c. For each KEYSIZE, take the FIRST KEYSIZE worth of bytes, and the
# SECOND KEYSIZE worth of bytes, and find the edit distance between
# them. Normalize this result by dividing by KEYSIZE.
# 
# d. The KEYSIZE with the smallest normalized edit distance is probably
# the key. You could proceed perhaps with the smallest 2-3 KEYSIZE
# values. Or take 4 KEYSIZE blocks instead of 2 and average the
# distances.
# 
# e. Now that you probably know the KEYSIZE: break the ciphertext into
# blocks of KEYSIZE length.
# 
# f. Now transpose the blocks: make a block that is the first byte of
# every block, and a block that is the second byte of every block, and
# so on.
# 
# g. Solve each block as if it was single-character XOR. You already
# have code to do this.
# 
# e. For each block, the single-byte XOR key that produces the best
# looking histogram is the repeating-key XOR key byte for that
# block. Put them together and you have the key.
# 
# // ------------------------------------------------------------
# 
# 7. AES in ECB Mode
# 
# The Base64-encoded content at the following location:
# 
#     https://gist.github.com/3132853
# 
# Has been encrypted via AES-128 in ECB mode under the key
# 
#     "YELLOW SUBMARINE".
# 
# (I like "YELLOW SUBMARINE" because it's exactly 16 bytes long).
# 
# Decrypt it.
# 
# Easiest way:
# 
# Use OpenSSL::Cipher and give it AES-128-ECB as the cipher.
# 
# // ------------------------------------------------------------
# 
# 8. Detecting ECB
# 
# At the following URL are a bunch of hex-encoded ciphertexts:
# 
#    https://gist.github.com/3132928
# 
# One of them is ECB encrypted. Detect it.
# 
# Remember that the problem with ECB is that it is stateless and
# deterministic; the same 16 byte plaintext block will always produce
# the same 16 byte ciphertext.
# 
# // ------------------------------------------------------------