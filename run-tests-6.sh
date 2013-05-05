#!/bin/sh

i=0

ensure()
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
# 41. Implement Unpadded Message Recovery Oracle
# 
# Nate Lawson says we should stop calling it "RSA padding" and start
# calling it "RSA armoring". Here's why.
# 
# Imagine a web application, again with the Javascript encryption,
# taking RSA-encrypted messages which (again: Javascript) aren't padded
# before encryption at all.
# 
# You can submit an arbitrary RSA blob and the server will return
# plaintext. But you can't submit the same message twice: let's say the
# server keeps hashes of previous messages for some liveness interval,
# and that the message has an embedded timestamp:
# 
#   {
#     time: 1356304276,
#     social: '555-55-5555',
#   }
# 
# You'd like to capture other people's messages and use the server to
# decrypt them. But when you try, the server takes the hash of the
# ciphertext and uses it to reject the request. Any bit you flip in the
# ciphertext irrevocably scrambles the decryption.
# 
# This turns out to be trivially breakable:
# 
# * Capture the ciphertext C
# 
# * Let N and E be the public modulus and exponent respectively
# 
# * Let S be a random number > 1 mod N. Doesn't matter what.
# 
# * C' = ((S**E mod N) * C) mod N
# 
# * Submit C', which appears totally different from C, to the server,
#   recovering P', which appears totally different from P
# 
#          P'
#    P = -----  mod N
#          S
# 
# Oops!
# 
# (Remember: you don't simply divide mod N; you multiply by the
# multiplicative inverse mod N.)
# 
# Implement that attack.
ensure `python mcp41.py` "ok"

# // ------------------------------------------------------------
# 
# 42. Bleichenbacher's e=3 RSA Attack
# 
# RSA with an encrypting exponent of 3 is popular, because it makes the
# RSA math faster.
# 
# With e=3 RSA, encryption is just cubing a number mod the public
# encryption modulus:
# 
#    c = m ** 3 % n
# 
# e=3 is secure as long as we can make assumptions about the message
# blocks we're encrypting. The worry with low-exponent RSA is that the
# message blocks we process won't be large enough to wrap the modulus
# after being cubed. The block 00:02 (imagine sufficient zero-padding)
# can be "encrypted" in e=3 RSA; it is simply 00:08.
# 
# When RSA is used to sign, rather than encrypt, the operations are
# reversed; the verifier "decrypts" the message by cubing it. This
# produces a "plaintext" which the verifier checks for validity.
# 
# When you use RSA to sign a message, you supply it a block input that
# contains a message digest. The PKCS1.5 standard formats that block as:
# 
#   00h 01h ffh ffh ... ffh ffh 00h ASN.1 GOOP HASH
# 
# As intended, the ffh bytes in that block expand to fill the whole
# block, producing a "right-justified" hash (the last byte of the hash
# is the last byte of the message).
# 
# There was, 7 years ago, a common implementation flaw with RSA
# verifiers: they'd verify signatures by "decrypting" them (cubing them
# modulo the public exponent) and then "parsing" them by looking for
# 00h 01h ... ffh 00h ASN.1 HASH.
# 
# This is a bug because it implies the verifier isn't checking all the
# padding. If you don't check the padding, you leave open the
# possibility that instead of hundreds of ffh bytes, you have only a
# few, which if you think about it means there could be squizzilions of
# possible numbers that could produce a valid-looking signature.
# 
# How to find such a block? Find a number that when cubed (a) doesn't
# wrap the modulus (thus bypassing the key entirely) and (b) produces a
# block that starts "00h 01h ffh ... 00h ASN.1 HASH".
# 
# There are two ways to approach this problem:
# 
# * You can work from Hal Finney's writeup, available on Google, of how
#   Bleichenbacher explained the math "so that you can do it by hand
#   with a pencil".
# 
# * You can implement an integer cube root in your language, format the
#   message block you want to forge, leaving sufficient trailing zeros
#   at the end to fill with garbage, then take the cube-root of that
#   block.
# 
# Forge a 1024-bit RSA signature for the string "hi mom". Make sure your
# implementation actually accepts the signature!
ensure `python mcp42.py` "ok"

# // ------------------------------------------------------------
# 
# 43. DSA Key Recovery From Nonce
# 
# Step 1: Relocate so that you are out of easy travel distance of us.
# 
# Step 2: Implement DSA, up to signing and verifying, including
# parameter generation.
# 
# HAH HAH YOU'RE TOO FAR AWAY TO COME PUNCH US.
# 
# JUST KIDDING you can skip the parameter generation part if you
# want; if you do, use these params:
# 
#  p = 800000000000000089e1855218a0e7dac38136ffafa72eda7
#      859f2171e25e65eac698c1702578b07dc2a1076da241c76c6
#      2d374d8389ea5aeffd3226a0530cc565f3bf6b50929139ebe
#      ac04f48c3c84afb796d61e5a4f9a8fda812ab59494232c7d2
#      b4deb50aa18ee9e132bfa85ac4374d7f9091abc3d015efc87
#      1a584471bb1
# 
#  q = f4f47f05794b256174bba6e9b396a7707e563c5b
# 
#  g = 5958c9d3898b224b12672c0b98e06c60df923cb8bc999d119
#      458fef538b8fa4046c8db53039db620c094c9fa077ef389b5
#      322a559946a71903f990f1f7e0e025e2d7f7cf494aff1a047
#      0f5b64c36b625a097f1651fe775323556fe00b3608c887892
#      878480e99041be601a62166ca6894bdd41a7054ec89f756ba
#      9fc95302291
# 
# ("But I want smaller params!" Then generate them yourself.)
# 
# The DSA signing operation generates a random subkey "k". You know this
# because you implemented the DSA sign operation.
# 
# This is the first and easier of two challenges regarding the DSA "k"
# subkey.
# 
# Given a known "k", it's trivial to recover the DSA private key "x":
# 
#        (s * k) - H(msg)
#    x = ----------------  mod q
#                r
# 
# Do this a couple times to prove to yourself that you grok it. Capture
# it in a function of some sort.
# 
# Now then. I used the parameters above. I generated a keypair. My
# pubkey is:
# 
#     y = 84ad4719d044495496a3201c8ff484feb45b962e7302e56a392aee4
#         abab3e4bdebf2955b4736012f21a08084056b19bcd7fee56048e004
#         e44984e2f411788efdc837a0d2e5abb7b555039fd243ac01f0fb2ed
#         1dec568280ce678e931868d23eb095fde9d3779191b8c0299d6e07b
#         bb283e6633451e535c45513b2d33c99ea17
# 
# I signed
# 
#   For those that envy a MC it can be hazardous to your health
#   So be friendly, a matter of life and death, just like a etch-a-sketch
# 
# (My SHA1 for this string was d2d0714f014a9784047eaeccf956520045c45265;
# I don't know what NIST wants you to do, but when I convert that hash
# to an integer I get 0xd2d0714f014a9784047eaeccf956520045c45265).
# 
# I get:
# 
#     r = 548099063082341131477253921760299949438196259240
#     s = 857042759984254168557880549501802188789837994940
# 
# I signed this string with a broken implemention of DSA that generated
# "k" values between 0 and 2^16. What's my private key?
# 
# Its SHA-1 fingerprint (after being converted to hex) is:
# 
#   0954edd5e0afe5542a4adf012611a91912a3ec16
# 
# Obviously, it also generates the same signature for that string.
ensure "`python mcp43.py`" "k: 0x40bf, x: 0x15fb2873d16b3e129ff76d0918fd7ada54659e49"

# // ------------------------------------------------------------
# 
# 44. DSA Nonce Recovery From Repeated Nonce
# 
# At the following URL, find a collection of DSA-signed messages:
# 
#   https://gist.github.com/anonymous/f83e6b6e6889f2e8b7ff
# 
# (NB: each msg has a trailing space.)
# 
# These were signed under the following pubkey:
# 
#   y = 2d026f4bf30195ede3a088da85e398ef869611d0f68f07
#       13d51c9c1a3a26c95105d915e2d8cdf26d056b86b8a7b8
#       5519b1c23cc3ecdc6062650462e3063bd179c2a6581519
#       f674a61f1d89a1fff27171ebc1b93d4dc57bceb7ae2430
#       f98a6a4d83d8279ee65d71c1203d2c96d65ebbf7cce9d3
#       2971c3de5084cce04a2e147821
# 
# (using the same domain parameters as the previous exercise)
# 
# It should not be hard to find the messages for which we have
# accidentally used a repeated "k". Given a pair of such messages, you
# can discover the "k" we used with the following formula:
# 
#            (m1 - m2)
#        k = --------- mod q
#            (s1 - s2)
# 
# Remember all this math is mod q; s2 may be larger than s1, for
# instance, which isn't a problem if you're doing the subtraction mod
# q. If you're like me, you'll definitely lose an hour to forgetting a
# paren or a mod q. (And don't forget that modular inverse function!)
# 
# What's my private key? Its SHA-1 (from hex) is:
# 
#      ca8f6f7c66fa362d40760d135b763eb8527d3d52
ensure "`python mcp44.py`" "i1: 0, i2: 8, k: 0x51ffac4835ccfda57356a86ebd57fbf9, x: 0xf1b733db159c66bce071d21e044a48b0e4c1665a"

# // ------------------------------------------------------------
# 
# 45. DSA Parameter Tampering
# 
# Take your DSA code from the previous exercise. Imagine it as part of
# an algorithm in which the client was allowed to propose domain
# parameters (the p and q moduli, and the g generator).
# 
# This would be bad, because attackers could trick victims into accepting
# bad parameters. Vaudenay gave two examples of bad generator
# parameters: generators that were 0 mod p, and generators that were 1
# mod p.
# 
# Use the parameters from the previous exercise, but substitute 0 for
# "g". Generate a signature. You will notice something bad. Verify the
# signature. Now verify any other signature, for any other string.
# 
# Now, try (p+1) as "g". With this "g", you can generate a magic
# signature s, r for any DSA public key that will validate against any
# string. For arbitrary z:
# 
#     r = ((y**z) % p) % q
# 
#           r
#     s =  --- % q
#           z
# 
# Sign "Hello, world". And "Goodbye, world".
# 
# // ------------------------------------------------------------
# 
# 46. Decrypt RSA From One-Bit Oracle
# 
# This is a bit of a toy problem, but it's very helpful for
# understanding what RSA is doing (and also for why pure
# number-theoretic encryption is terrifying).
# 
# Generate a 1024 bit RSA key pair.
# 
# Write an oracle function that uses the private key to answer the
# question "is the plaintext of this message even or odd" (is the last
# bit of the message 0 or 1). Imagine for instance a server that
# accepted RSA-encrypted messages and checked the parity of their
# decryption to validate them, and spat out an error if they were of the
# wrong parity.
# 
# Anyways: function returning true or false based on whether the
# decrypted plaintext was even or odd, and nothing else.
# 
# Take the following string and un-Base64 it in your code (without
# looking at it!) and encrypt it to the public key, creating a
# ciphertext:
# 
# VGhhdCdzIHdoeSBJIGZvdW5kIHlvdSBkb24ndCBwbGF5IGFyb3VuZCB3aXRoIHRoZSBGdW5reSBDb2xkIE1lZGluYQ==
# 
# With your oracle function, you can trivially decrypt the message.
# 
# Here's why:
# 
# * RSA ciphertexts are just numbers. You can do trivial math on
#   them. You can for instance multiply a ciphertext by the
#   RSA-encryption of another number; the corresponding plaintext will
#   be the product of those two numbers.
# 
# * If you double a ciphertext (multiply it by (2**e)%n), the resulting
#   plaintext will (obviously) be either even or odd.
# 
# * If the plaintext after doubling is even, doubling the plaintext
#   DIDN'T WRAP THE MODULUS --- the modulus is a prime number. That
#   means the plaintext is less than half the modulus.
# 
# You can repeatedly apply this heuristic, once per bit of the message,
# checking your oracle function each time.
# 
# Your decryption function starts with bounds for the plaintext of [0,n].
# 
# Each iteration of the decryption cuts the bounds in half; either the
# upper bound is reduced by half, or the lower bound is.
# 
# After log2(n) iterations, you have the decryption of the message.
# 
# Print the upper bound of the message as a string at each iteration;
# you'll see the message decrypt "hollywood style".
# 
# Decrypt the string (after encrypting it to a hidden private key, duh) above.
# 
# // ------------------------------------------------------------
# 
# 47. Bleichenbacher's PKCS 1.5 Padding Oracle (Simple Case)
# 
# Google for:
# 
# "Chosen ciphertext attacks against protocols based on the RSA encryption standard"
# 
# This is Bleichenbacher from CRYPTO '98; I get a bunch of .ps versions
# on the first search page.
# 
# Read the paper. It describes a padding oracle attack on
# PKCS#1v1.5. The attack is similar in spirit to the CBC padding oracle
# you built earlier; it's an "adaptive chosen ciphertext attack", which
# means you start with a valid ciphertext and repeatedly corrupt it,
# bouncing the adulterated ciphertexts off the target to learn things
# about the original.
# 
# This is a common flaw even in modern cryptosystems that use RSA.
# 
# It's also the most fun you can have building a crypto attack. It
# involves 9th grade math, but also has you implementing an algorithm
# that is complex on par with finding a minimum cost spanning tree.
# 
# The setup:
# 
# *	Build an oracle function, just like you did in the last exercise, but
# 	have it check for plaintext[0] == 0 and plaintext[1] == 2.
# 
# *	Generate a 256 bit keypair (that is, p and q will each be 128 bit
# 	primes), [n, e, d].
# 
# *	Plug d and n into your oracle function.
# 
# *	PKCS1.5-pad a short message, like "kick it, CC", and call it
#   "m". Encrypt to to get "c".
# 
# Decrypt "c" using your padding oracle.
# 
# For this challenge, we've used an untenably small RSA modulus (you
# could factor this keypair instantly). That's because this exercise
# targets a specific step in the Bleichenbacher paper --- Step 2c, which
# implements a fast, nearly O(log n) search for the plaintext.
# 
# Things you want to keep in mind as you read the paper:
# 
# *	RSA ciphertexts are just numbers.
# 
# *	RSA is "homomorphic" with respect to multiplication, which
#   means you can multiply c * RSA(2) to get a c' that will
# 	decrypt to plaintext * 2. This is mindbending but easy to
# 	see if you play with it in code --- try multiplying
#   ciphertexts with the RSA encryptions of numbers so you know
#   you grok it.
# 
# 	What you need to grok for this challenge is that Bleichenbacher
# 	uses multiplication on ciphertexts the way the CBC oracle uses
# 	XORs of random blocks.
# 
# *	A PKCS#1v1.5 conformant plaintext, one that starts with 00:02,
# 	must be a number between 02:00:00...00 and 02:FF:FF..FF --- in
# 	other words, 2B and 3B-1, where B is the bit size of the
# 	modulus minus the first 16 bits. When you see 2B and 3B,
# 	that's the idea the paper is playing with.
# 
# To decrypt "c", you'll need Step 2a from the paper (the search for the
# first "s" that, when encrypted and multiplied with the ciphertext,
# produces a conformant plaintext), Step 2c, the fast O(log n) search,
# and Step 3.
# 
# Your Step 3 code is probably not going to need to handle multiple
# ranges.
# 
# We recommend you just use the raw math from paper (check, check,
# double check your translation to code) and not spend too much time
# trying to grok how the math works.
# 
# // ------------------------------------------------------------
# 
# 48. Bleichenbacher's PKCS 1.5 Padding Oracle (Complete)
# 
# This is a continuation of challenge #47; it implements the complete
# BB'98 attack.
# 
# Set yourself up the way you did in #47, but this time generate a 768
# bit modulus.
# 
# To make the attack work with a realistic RSA keypair, you need to
# reproduce step 2b from the paper, and your implementation of Step 3
# needs to handle multiple ranges.
# 
# The full Bleichenbacher attack works basically like this:
# 
# *	Starting from the smallest 's' that could possibly produce
# 	a plaintext bigger than 2B, iteratively search for an 's' that
# 	produces a conformant plaintext.
# 
# *	For our known 's1' and 'n', solve m1=m0s1-rn (again: just a
# 	definition of modular multiplication) for 'r', the number of
# 	times we've wrapped the modulus.
# 
# 	'm0' and 'm1' are unknowns, but we know both are conformant
# 	PKCS#1v1.5 plaintexts, and so are between [2B,3B].
# 
# 	We substitute the known bounds for both, leaving only 'r'
# 	free, and solve for a range of possible 'r'  values. This
# 	range should be small!
# 
# *	Solve m1=m0s1-rn again but this time for 'm0', plugging in
# 	each value of 'r' we generated in the last step. This gives
# 	us new intervals to work with. Rule out any interval that
# 	is outside 2B,3B.
# 
# *	Repeat the process for successively higher values of 's'.
# 	Eventually, this process will get us down to just one
# 	interval, whereupon we're back to exercise #47.
# 
# What happens when we get down to one interval is, we stop blindly
# incrementing 's'; instead, we start rapidly growing 'r' and backing it
# out to 's' values by solving m1=m0s1-rn for 's' instead of 'r' or
# 'm0'. So much algebra! Make your teenage son do it for you! *Note:
# does not work well in practice*