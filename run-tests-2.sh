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
# 9. Implement PKCS#7 padding
# 
# Pad any block to a specific block length, by appending the number of
# bytes of padding to the end of the block. For instance,
# 
#   "YELLOW SUBMARINE"
# 
# padded to 20 bytes would be:
# 
#   "YELLOW SUBMARINE\x04\x04\x04\x04"
# 
# The particulars of this algorithm are easy to find online.
ensure "`bin/mcp9 "59454c4c4f57205355424d4152494e45" 20`" "59454c4c4f57205355424d4152494e4504040404"

# // ------------------------------------------------------------
# 
# 10. Implement CBC Mode
# 
# In CBC mode, each ciphertext block is added to the next plaintext
# block before the next call to the cipher core.
# 
# The first plaintext block, which has no associated previous ciphertext
# block, is added to a "fake 0th ciphertext block" called the IV.
# 
# Implement CBC mode by hand by taking the ECB function you just wrote,
# making it encrypt instead of decrypt (verify this by decrypting
# whatever you encrypt to test), and using your XOR function from
# previous exercise.
# 
# DO NOT CHEAT AND USE OPENSSL TO DO CBC MODE, EVEN TO VERIFY YOUR
# RESULTS. What's the point of even doing this stuff if you aren't going
# to learn from it?
# 
# The buffer at:
# 
#     https://gist.github.com/3132976
# 
# is intelligible (somewhat) when CBC decrypted against "YELLOW
# SUBMARINE" with an IV of all ASCII 0 (\x00\x00\x00 &c)
inp=`cat gistfile5.txt`
ans="I'm back and I'm ringin' the bell .A rockin' on the mike while the fly girls yell .In ecstasy in the back of me .Well that's my DJ Deshay cuttin' all them Z's .Hittin' hard and the girlies goin' crazy .Vanilla's on the mike, man I'm not lazy. ..I'm lettin' my drug kick in .It controls my mouth and I begin .To just let it flow, let my concepts go .My posse's to the side yellin', Go Vanilla Go! ..Smooth 'cause that's the way I will be .And if you don't give a damn, then .Why you starin' at me .So get off 'cause I control the stage .There's no dissin' allowed .I'm in my own phase .The girlies sa y they love me and that is ok .And I can dance better than any kid n' play ..Stage 2 -- Yea the one ya' wanna listen to .It's off my head so let the beat play through .So I can funk it up and make it sound good .1-2-3 Yo -- Knock on some wood .For good luck, I like my rhymes atrocious .Supercalafragilisticexpialidocious .I'm an effect and that you can bet .I can take a fly girl and make her wet. ..I'm like Samson -- Samson to Delilah .There's no denyin', You can try to hang .But you'll keep tryin' to get my style .Over and over, practice makes perfect .But not if you're a loafer. ..You'll get nowhere, no place, no time, no girls .Soon -- Oh my God, homebody, you probably eat .Spaghetti with a spoon! Come on and say it! ..VIP. Vanilla Ice yep, yep, I'm comin' hard like a rhino .Intoxicating so you stagger like a wino .So punks stop trying and girl stop cryin' .Vanilla Ice is sellin' and you people are buyin' .'Cause why the freaks are jockin' like Crazy Glue .Movin' and groovin' trying to sing along .All through the ghetto groovin' this here song .Now you're amazed by the VIP posse. ..Steppin' so hard like a German Nazi .Startled by the bases hittin' ground .There's no trippin' on mine, I'm just gettin' down .Sparkamatic, I'm hangin' tight like a fanatic .You trapped me once and I thought that .You might have it .So step down and lend me your ear .'89 in my time! You, '90 is my year. ..You're weakenin' fast, YO! and I can tell it .Your body's gettin' hot, so, so I can smell it .So don't be mad and don't be sad .'Cause the lyrics belong to ICE, You can call me Dad .You're pitchin' a fit, so step back and endure .Let the witch doctor, Ice, do the dance to cure .So come up close and don't be square .You wanna battle me -- Anytime, anywhere ..You thought that I was weak, Boy, you're dead wrong .So come on, everybody and sing this song ..Say -- Play that funky music Say, go white boy, go white boy go .play that funky music Go white boy, go white boy, go .Lay down and boogie and play that funky music till you die. ..Play that funky music Come on, Come on, let me hear .Play that funky music white boy you say it, say it .Play that funky music A little louder now .Play that funky music, white boy Come on, Come on, Come on .Play that funky music ....."
ensure "`bin/mcp10 "$inp" "59454c4c4f57205355424d4152494e45" "00000000000000000000000000000000"`" "$ans"

# // ------------------------------------------------------------
# 
# 11. Write an oracle function and use it to detect ECB.
# 
# Now that you have ECB and CBC working:
# 
# Write a function to generate a random AES key; that's just 16 random
# bytes.
# 
# Write a function that encrypts data under an unknown key --- that is,
# a function that generates a random key and encrypts under it.
# 
# The function should look like:
# 
# encryption_oracle(your-input)
#  => [MEANINGLESS JIBBER JABBER]
# 
# Under the hood, have the function APPEND 5-10 bytes (count chosen
# randomly) BEFORE the plaintext and 5-10 bytes AFTER the plaintext.
# 
# Now, have the function choose to encrypt under ECB 1/2 the time, and
# under CBC the other half (just use random IVs each time for CBC). Use
# rand(2) to decide which to use.
# 
# Now detect the block cipher mode the function is using each time.
# 
# // ------------------------------------------------------------
# 
# 12. Byte-at-a-time ECB decryption, Full control version
# 
# Copy your oracle function to a new function that encrypts buffers
# under ECB mode using a consistent but unknown key (for instance,
# assign a single random key, once, to a global variable).
# 
# Now take that same function and have it append to the plaintext,
# BEFORE ENCRYPTING, the following string:
# 
#   Um9sbGluJyBpbiBteSA1LjAKV2l0aCBteSByYWctdG9wIGRvd24gc28gbXkg
#   aGFpciBjYW4gYmxvdwpUaGUgZ2lybGllcyBvbiBzdGFuZGJ5IHdhdmluZyBq
#   dXN0IHRvIHNheSBoaQpEaWQgeW91IHN0b3A/IE5vLCBJIGp1c3QgZHJvdmUg
#   YnkK
# 
# SPOILER ALERT: DO NOT DECODE THIS STRING NOW. DON'T DO IT.
# 
# Base64 decode the string before appending it. DO NOT BASE64 DECODE THE
# STRING BY HAND; MAKE YOUR CODE DO IT. The point is that you don't know
# its contents.
# 
# What you have now is a function that produces:
# 
#   AES-128-ECB(your-string || unknown-string, random-key)
# 
# You can decrypt "unknown-string" with repeated calls to the oracle
# function!
# 
# Here's roughly how:
# 
# a. Feed identical bytes of your-string to the function 1 at a time ---
# start with 1 byte ("A"), then "AA", then "AAA" and so on. Discover the
# block size of the cipher. You know it, but do this step anyway.
# 
# b. Detect that the function is using ECB. You already know, but do
# this step anyways.
# 
# c. Knowing the block size, craft an input block that is exactly 1 byte
# short (for instance, if the block size is 8 bytes, make
# "AAAAAAA"). Think about what the oracle function is going to put in
# that last byte position.
# 
# d. Make a dictionary of every possible last byte by feeding different
# strings to the oracle; for instance, "AAAAAAAA", "AAAAAAAB",
# "AAAAAAAC", remembering the first block of each invocation.
# 
# e. Match the output of the one-byte-short input to one of the entries
# in your dictionary. You've now discovered the first byte of
# unknown-string.
# 
# f. Repeat for the next byte.
# 
# // ------------------------------------------------------------
# 
# 13. ECB cut-and-paste
# 
# Write a k=v parsing routine, as if for a structured cookie. The
# routine should take:
# 
#    foo=bar&baz=qux&zap=zazzle
# 
# and produce:
# 
#   {
#     foo: 'bar',
#     baz: 'qux',
#     zap: 'zazzle'
#   }
# 
# (you know, the object; I don't care if you convert it to JSON).
# 
# Now write a function that encodes a user profile in that format, given
# an email address. You should have something like:
# 
#   profile_for("foo@bar.com")
# 
# and it should produce:
# 
#   {
#     email: 'foo@bar.com',
#     uid: 10,
#     role: 'user'
#   }
# 
# encoded as:
# 
#   email=foo@bar.com&uid=10&role=user
# 
# Your "profile_for" function should NOT allow encoding metacharacters
# (& and =). Eat them, quote them, whatever you want to do, but don't
# let people set their email address to "foo@bar.com&role=admin".
# 
# Now, two more easy functions. Generate a random AES key, then:
# 
#  (a) Encrypt the encoded user profile under the key; "provide" that
#  to the "attacker".
# 
#  (b) Decrypt the encoded user profile and parse it.
# 
# Using only the user input to profile_for() (as an oracle to generate
# "valid" ciphertexts) and the ciphertexts themselves, make a role=admin
# profile.
# 
# // ------------------------------------------------------------
# 
# 14. Byte-at-a-time ECB decryption, Partial control version
# 
# Take your oracle function from #12. Now generate a random count of
# random bytes and prepend this string to every plaintext. You are now
# doing:
# 
#   AES-128-ECB(random-prefix || attacker-controlled || target-bytes, random-key)
# 
# Same goal: decrypt the target-bytes.
# 
# What's harder about doing this?
# 
# How would you overcome that obstacle? The hint is: you're using
# all the tools you already have; no crazy math is required.
# 
# Think about the words "STIMULUS" and "RESPONSE".
# 
# // ------------------------------------------------------------
# 
# 15. PKCS#7 padding validation
# 
# Write a function that takes a plaintext, determines if it has valid
# PKCS#7 padding, and strips the padding off.
# 
# The string:
# 
#     "ICE ICE BABY\x04\x04\x04\x04"
# 
# has valid padding, and produces the result "ICE ICE BABY".
# 
# The string:
# 
#     "ICE ICE BABY\x05\x05\x05\x05"
# 
# does not have valid padding, nor does:
# 
#      "ICE ICE BABY\x01\x02\x03\x04"
# 
# If you are writing in a language with exceptions, like Python or Ruby,
# make your function throw an exception on bad padding.
# 
# // ------------------------------------------------------------
# 
# 16. CBC bit flipping
# 
# Generate a random AES key.
# 
# Combine your padding code and CBC code to write two functions.
# 
# The first function should take an arbitrary input string, prepend the
# string:
#         "comment1=cooking%20MCs;userdata="
# and append the string:
#     ";comment2=%20like%20a%20pound%20of%20bacon"
# 
# The function should quote out the ";" and "=" characters.
# 
# The function should then pad out the input to the 16-byte AES block
# length and encrypt it under the random AES key.
# 
# The second function should decrypt the string and look for the
# characters ";admin=true;" (or, equivalently, decrypt, split the string
# on ;, convert each resulting string into 2-tuples, and look for the
# "admin" tuple. Return true or false based on whether the string exists.
# 
# If you've written the first function properly, it should not be
# possible to provide user input to it that will generate the string the
# second function is looking for.
# 
# Instead, modify the ciphertext (without knowledge of the AES key) to
# accomplish this.
# 
# You're relying on the fact that in CBC mode, a 1-bit error in a
# ciphertext block:
# 
# * Completely scrambles the block the error occurs in
# 
# * Produces the identical 1-bit error (/edit) in the next ciphertext
#  block.
# 
# Before you implement this attack, answer this question: why does CBC
# mode have this property?