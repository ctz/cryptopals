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
# 25. Break "random access read/write" AES CTR
# 
# Back to CTR. Encrypt the recovered plaintext from
# 
#      https://gist.github.com/3132853
# 
# (the ECB exercise) under CTR with a random key (for this exercise the
# key should be unknown to you, but hold on to it).
# 
# Now, write the code that allows you to "seek" into the ciphertext,
# decrypt, and re-encrypt with different plaintext. Expose this as a
# function, like, "edit(ciphertext, key, offet, newtext)".
# 
# Imagine the "edit" function was exposed to attackers by means of an
# API call that didn't reveal the key or the original plaintext; the
# attacker has the ciphertext and controls the offset and "new text".
# 
# Recover the original plaintext.
inp=`cat gistfile3.plain.txt`
plain="I'm back and I'm ringin' the bell .A rockin' on the mike while the fly girls yell .In ecstasy in the back of me .Well that's my DJ Deshay cuttin' all them Z's .Hittin' hard and the girlies goin' crazy .Vanilla's on the mike, man I'm not lazy. ..I'm lettin' my drug kick in .It controls my mouth and I begin .To just let it flow, let my concepts go .My posse's to the side yellin', Go Vanilla Go! ..Smooth 'cause that's the way I will be .And if you don't give a damn, then .Why you starin' at me .So get off 'cause I control the stage .There's no dissin' allowed .I'm in my own phase .The girlies sa y they love me and that is ok .And I can dance better than any kid n' play ..Stage 2 -- Yea the one ya' wanna listen to .It's off my head so let the beat play through .So I can funk it up and make it sound good .1-2-3 Yo -- Knock on some wood .For good luck, I like my rhymes atrocious .Supercalafragilisticexpialidocious .I'm an effect and that you can bet .I can take a fly girl and make her wet. ..I'm like Samson -- Samson to Delilah .There's no denyin', You can try to hang .But you'll keep tryin' to get my style .Over and over, practice makes perfect .But not if you're a loafer. ..You'll get nowhere, no place, no time, no girls .Soon -- Oh my God, homebody, you probably eat .Spaghetti with a spoon! Come on and say it! ..VIP. Vanilla Ice yep, yep, I'm comin' hard like a rhino .Intoxicating so you stagger like a wino .So punks stop trying and girl stop cryin' .Vanilla Ice is sellin' and you people are buyin' .'Cause why the freaks are jockin' like Crazy Glue .Movin' and groovin' trying to sing along .All through the ghetto groovin' this here song .Now you're amazed by the VIP posse. ..Steppin' so hard like a German Nazi .Startled by the bases hittin' ground .There's no trippin' on mine, I'm just gettin' down .Sparkamatic, I'm hangin' tight like a fanatic .You trapped me once and I thought that .You might have it .So step down and lend me your ear .'89 in my time! You, '90 is my year. ..You're weakenin' fast, YO! and I can tell it .Your body's gettin' hot, so, so I can smell it .So don't be mad and don't be sad .'Cause the lyrics belong to ICE, You can call me Dad .You're pitchin' a fit, so step back and endure .Let the witch doctor, Ice, do the dance to cure .So come up close and don't be square .You wanna battle me -- Anytime, anywhere ..You thought that I was weak, Boy, you're dead wrong .So come on, everybody and sing this song ..Say -- Play that funky music Say, go white boy, go white boy go .play that funky music Go white boy, go white boy, go .Lay down and boogie and play that funky music till you die. ..Play that funky music Come on, Come on, let me hear .Play that funky music white boy you say it, say it .Play that funky music A little louder now .Play that funky music, white boy Come on, Come on, Come on .Play that funky music ....."
ensure "`bin/mcp25 "$inp"`" "$plain"

# // ------------------------------------------------------------
# 
# 26. CTR bit flipping
# 
# There are people in the world that believe that CTR resists
# bit flipping attacks of the kind to which CBC mode is susceptible.
# 
# Re-implement the CBC bitflipping exercise (16) from earlier to use CTR mode
# instead of CBC mode. Inject an "admin=true" token.
# 
ensure "`bin/mcp26`" "ok"

# // ------------------------------------------------------------
# 
# 27. Recover the key from CBC with IV=Key
# 
# Take your code from the CBC exercise (16) and modify it so that it
# repurposes the key for CBC encryption as the IV. Applications
# sometimes use the key as an IV on the auspices that both the sender
# and the receiver have to know the key already, and can save some space
# by using it as both a key and an IV.
# 
# Using the key as an IV is insecure; an attacker that can modify
# ciphertext in flight can get the receiver to decrypt a value that will
# reveal the key.
# 
# The CBC code from exercise 16 encrypts a URL string. Verify each byte
# of the plaintext for ASCII compliance (ie, look for high-ASCII
# values). Noncompliant messages should raise an exception or return an
# error that includes the decrypted plaintext (this happens all the time
# in real systems, for what it's worth).
# 
# Use your code to encrypt a message that is at least 3 blocks long:
# 
#   AES-CBC(P_1, P_2, P_3) -> C_1, C_2, C_3
# 
# Modify the message (you are now the attacker):
# 
#   C_1, C_2, C_3 -> C_1, 0, C_1
# 
# Decrypt the message (you are now the receiver) and raise the
# appropriate error if high-ASCII is found.
# 
# As the attacker, recovering the plaintext from the error, extract the
# key:
# 
#   P'_1 XOR P'_3
# 
# // ------------------------------------------------------------
# 
# 28. Implement a SHA-1 keyed MAC
# 
# Find a SHA-1 implementation in the language you code in. Do not use
# the SHA-1 implementation your language already provides (for instance,
# don't use the "Digest" library in Ruby, or call OpenSSL; in Ruby,
# you'd want a pure-Ruby SHA-1).
# 
# Write a function to authenticate a message under a secret key by using
# a secret-prefix MAC, which is simply:
# 
#   SHA1(key || message)
# 
# Verify that you cannot tamper with the message without breaking the
# MAC you've produced, and that you can't produce a new MAC without
# knowing the secret key.
# 
# // ------------------------------------------------------------
# 
# 29. Break a SHA-1 keyed MAC using length extension
# 
# Secret-prefix SHA-1 MACs are trivially breakable.
# 
# The attack on secret-prefix SHA1 relies on the fact that you can take
# the ouput of SHA-1 and use it as a new starting point for SHA-1, thus
# taking an arbitrary SHA-1 hash and "feeding it more data".
# 
# Since the key precedes the data in secret-prefix, any additional data
# you feed the SHA-1 hash in this fashion will appear to have been
# hashed with the secret key.
# 
# To carry out the attack, you'll need to account for the fact that
# SHA-1 is "padded" with the bit-length of the message; your forged
# message will need to include that padding. We call this "glue
# padding". The final message you actually forge will be:
# 
#           SHA1(key || original-message || glue-padding || new-message)
# 
# (where the final padding on the whole constructed message is implied)
# 
# Note that to generate the glue padding, you'll need to know the
# original bit length of the message; the message itself is known to the
# attacker, but the secret key isn't, so you'll need to guess at it.
# 
# This sounds more complicated than it is in practice.
# 
# To implement the attack, first write the function that computes the MD
# padding of an arbitrary message and verify that you're generating the
# same padding that your SHA-1 implementation is using. This should take
# you 5-10 minutes.
# 
# Now, take the SHA-1 secret-prefix MAC of the message you want to forge
# --- this is just a SHA-1 hash --- and break it into 32 bit SHA-1
# registers (SHA-1 calls them "a", "b", "c", &c).
# 
# Modify your SHA-1 implementation so that callers can pass in new
# values for "a", "b", "c" &c (they normally start at magic
# numbers). With the registers "fixated", hash the additional data you
# want to forge.
# 
# Using this attack, generate a secret-prefix MAC under a secret key
# (choose a random word from /usr/share/dict/words or something) of the
# string:
# 
# "comment1=cooking%20MCs;userdata=foo;comment2=%20like%20a%20pound%20of%20bacon"
# 
# Forge a variant of this message that ends with ";admin=true".
# 
# // ------------------------------------------------------------
# 
# 30. Break an MD4 keyed MAC using length extension.
# 
# Second verse, same as the first, but use MD4 instead of SHA-1. Having
# done this attack once against SHA-1, the MD4 variant should take much
# less time; mostly just the time you'll spend Googling for an
# implementation of MD4.
# 
# // ------------------------------------------------------------
# 
# 31. Implement HMAC-SHA1 and break it with an artificial timing leak.
# 
# The psuedocode on Wikipedia should be enough. HMAC is very easy.
# 
# Using the web framework of your choosing (Sinatra, web.py, whatever),
# write a tiny application that has a URL that takes a "file" argument
# and a "signature" argument, like so:
# 
# http://localhost:9000/test?file=foo&signature=46b4ec586117154dacd49d664e5d63fdc88efb51
# 
# Have the server generate an HMAC key, and then verify that the
# "signature" on incoming requests is valid for "file", using the "=="
# operator to compare the valid MAC for a file with the "signature"
# parameter (in other words, verify the HMAC the way any normal
# programmer would verify it).
# 
# Write a function, call it "insecure_compare", that implements the ==
# operation by doing byte-at-a-time comparisons with early exit (ie,
# return false at the first non-matching byte).
# 
# In the loop for "insecure_compare", add a 50ms sleep (sleep 50ms after
# each byte).
# 
# Use your "insecure_compare" function to verify the HMACs on incoming
# requests, and test that the whole contraption works. Return a 500 if
# the MAC is invalid, and a 200 if it's OK.
# 
# Using the timing leak in this application, write a program that
# discovers the valid MAC for any file.
# 
# // ------------------------------------------------------------
# 
# 32. Break HMAC-SHA1 with a slightly less artificial timing leak
# 
# Reduce the sleep in your "insecure_compare" until your previous
# solution breaks. (Try 5ms to start.)
# 
# Now break it again.
# 
# // ------------------------------------------------------------