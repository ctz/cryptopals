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
# 49. CBC-MAC Message Forgery
# 
# Let's talk about CBC-MAC.
# 
# CBC-MAC is like this:
# 
#   1. Take the plaintext P.
#   2. Encrypt P under CBC with key K, yielding ciphertext C.
#   3. Chuck all of C but the last block C[n].
#   4. C[n] is the MAC.
# 
# Suppose there's an online banking application, and it carries out user
# requests by talking to an API server over the network. Each request
# looks like this:
# 
#   message || IV || MAC
# 
# The message looks like this:
# 
#   from=#{from_id}&to=#{to_id}&amount=#{amount}
# 
# Now, write an API server and a web frontend for it. (NOTE: No need to
# get ambitious and write actual servers and web apps. Totally fine to
# go lo-fi on this one.) The client and server should share a secret key
# K to sign and verify messages.
# 
# The API server should accept messages, verify signatures, and carry
# out each transaction if the MAC is valid. It's also publicly exposed -
# the attacker can submit messages freely assuming he can forge the
# right MAC.
# 
# The web client should allow the attacker to generate valid messages
# for accounts he controls. (Feel free to sanitize params if you're
# feeling anal-retentive.) Assume the attacker is in a position to
# capture and inspect messages from the client to the API server.
# 
# One thing we haven't discussed is the IV. Assume the client generates
# a per-message IV and sends it along with the MAC. That's how CBC
# works, right?
# 
# Wrong.
# 
# For messages signed under CBC-MAC, an attacker-controlled IV is a
# liability. Why? Because it yields full control over the first block of
# the message.
# 
# Use this fact to generate a message transferring 1M spacebucks from a
# target victim's account into your account.
ensure "`python mcp49-choseniv.py`" "transferred 1000000 spacebucks from account 2 to account 1"

# I'll wait. Just let me know when you're done.
# 
# ~ waiting ~
# 
# ~ waiting ~
# 
# ~ waiting ~
# 
# All done? Great - I knew you could do it!
# 
# Now let's tune up that protocol a little bit.
# 
# As we now know, you're supposed to use a fixed IV with CBC-MAC, so
# let's do that. We'll set ours at 0 for simplicity. This means the IV
# comes out of the protocol:
# 
#   message || MAC
# 
# Pretty simple, but we'll also adjust the message. For the purposes of
# efficiency, the bank wants to be able to process multiple transactions
# in a single request. So the message now looks like this:
# 
#   from=#{from_id}&tx_list=#{transactions}
# 
# With the transaction list formatted like:
# 
#   to:amount(;to:amount)*
# 
# There's still a weakness here: the MAC is vulnerable to length
# extension attacks. How?
# 
# Well, the output of CBC-MAC is a valid IV for a new message.
# 
# "But we don't control the IV anymore!"
# 
# With sufficient mastery of CBC, we can fake it.
# 
# Your mission: capture a valid message from your target user. Use
# length extension to add a transaction paying the attacker's account 1M
# spacebucks.
ensure "`python mcp49-zeroiv.py`" "transfers: 10 spacebucks from account 2 to account 3, 1000000 spacebucks from account 2 to account 1"

# HINT:
# 
#   This would be a lot easier if you had full control over the first
#   block of your message, huh? Maybe you can simulate that.
# 
# Food for thought:
# 
#   How would you modify the protocol to prevent this?
# 
# answer: prefix message with the length, or use cmac
#
# // ------------------------------------------------------------
# 
# 50. Hashing with CBC-MAC
# 
# Sometimes people try to use CBC-MAC as a hash function.
# 
# This is a bad idea. Matt Green explains:
# 
#   To make a long story short: cryptographic hash functions are public
#   functions (i.e., no secret key) that have the property of
#   collision-resistance (it's hard to find two messages with the same
#   hash). MACs are keyed functions that (typically) provide message
#   unforgeability -- a very different property. Moreover, they
#   guarantee this only when the key is secret.
# 
# Let's try a simple exercise.
# 
# Hash functions are often used for code verification. This snippet of
# JavaScript (with newline):
# 
#   alert('MZA who was that?');
# 
# Hashes to 296b8d7cb78a243dda4d0a61d33bbdd1 under CBC-MAC with a key of
# "YELLOW SUBMARINE" and a 0 IV.
# 
# Forge a valid snippet of JavaScript that alerts "Ayo, the Wu is back!"
# and hashes to the same value. Ensure that it runs in a browser.
ensure "`python mcp50.py`" "616c657274282741796f2c20746865205775206973206261636b2127293b202f2f204120202021090909090909090909849b3b0b6932b6fbf8501b6d33ac9bba"
ensure "`grep -o 'Ayo, the Wu is back' test50.html`" "Ayo, the Wu is back"

# BONUS:
# 
#   Write JavaScript code that downloads your file, checks its CBC-MAC,
#   and inserts it into the DOM iff it matches the expected hash.
# 
# // ------------------------------------------------------------
# 
# 51. Compression Ratio Side-Channel Attacks
# 
# Internet traffic is often compressed to save bandwidth. Until
# recently, this included HTTPS headers, and it still includes the
# contents of responses.
# 
# Why does that matter?
# 
# Well, if you're an attacker with:
# 
#   1. Partial plaintext knowledge AND
#   2. Partial plaintext control AND
#   3. Access to a compression oracle
# 
# You've got a pretty good chance to recover any additional unknown
# plaintext.
# 
# What's a compression oracle? You give it some input and it tells you
# how well the full message compresses, i.e. the length of the resultant
# output.
# 
# This is somewhat similar to the timing attacks we did way back in set
# 4 in that we're taking advantage of incidental side channels rather
# than attacking the cryptographic mechanisms themselves.
# 
# Scenario: you are running a MITM attack with an eye towards stealing
# secure session cookies. You've injected malicious content allowing you
# to spawn arbitrary requests and observe them in flight. (The
# particulars aren't terribly important, just roll with it.)
# 
# So! Write this oracle:
# 
#   oracle(P) -> length(encrypt(compress(format_request(P))))
# 
# Format the request like this:
# 
#   POST / HTTP/1.1
#   Host: hapless.com
#   Cookie: sessionid=TmV2ZXIgcmV2ZWFsIHRoZSBXdS1UYW5nIFNlY3JldCE=
#   Content-Length: #{len(P)}
# 
#   #{P}
# 
# (Pretend you can't see that session id. You're the attacker.)
# 
# Compress using zlib or whatever.
# 
# Encryption... is actually kind of irrelevant for our purposes, but be
# a sport. Just use some stream cipher. Dealer's choice. Random key/IV
# on every call to the oracle.
# 
# And then just return the length in bytes.
# 
# Now, the idea here is to leak information using the compression
# library. A payload of "sessionid=T" should compress just a little bit
# better than, say, "sessionid=S".
# 
# There is one complicating factor. The DEFLATE algorithm operates in
# terms of individual bits, but the final message length will be in
# bytes. Even if you do find a better compression, the difference may
# not cross a byte boundary. So that's a problem.
# 
# You may also get some incidental false positives.
# 
# But don't worry! I have full confidence in you.
# 
# Use the compression oracle to recover the session id.
ensure "`python mcp51-rc4.py`" "TmV2ZXIgcmV2ZWFsIHRoZSBXdS1UYW5nIFNlY3JldCE="

# I'll wait.
# 
# Got it? Great.
# 
# Now swap out your stream cipher for CBC and do it again.


# // ------------------------------------------------------------
# 
# 52. Iterated Hash Function Multicollisions
# 
# While we're on the topic of hash functions . . .
# 
# The major feature you want in your hash function is
# collision-resistance. That is, it should be hard to generate
# collisions, and it should be REALLY hard to generate a collision for a
# given hash (AKA preimage).
# 
# Iterated hash functions have a problem: the effort to generate LOTS of
# collisions scales sublinearly.
# 
# What's an iterated hash function? For all intents and purposes, we're
# talking about the Merkle-Damgard construction. It looks like this:
# 
#   function MD(M, H, C):
#     for M[i] in pad(M):
#       H := C(M[i], H)
#     return H
# 
# For message M, initial state H, and compression function C.
# 
# This should look really familiar, because SHA-1 and MD4 are both in
# this category. What's cool is you can use this formula to build a
# makeshift hash function out of some spare crypto primitives you have
# lying around (e.g. C = AES-128).
# 
# Back on task: the cost of collisions scales sublinearly. What does
# that mean? If it's feasible to find one collision, it's probably
# feasible to find a lot.
# 
# How? For a given state H, find two blocks that collide. Now take the
# resulting hash from this collision as your new H and repeat. Recognize
# that with each iteration you can actually double your collisions by
# subbing in either of the two blocks for that slot.
# 
# This means that if finding two colliding messages takes 2^(b/2) work
# (where b is the bit-size of the hash function), then finding 2^n
# colliding messages only takes n*2^(b/2) work.
# 
# Let's test it. First, build your own MD hash function. We're going to
# be generating a LOT of collisions, so don't knock yourself out. In
# fact, go out of your way to make it bad. Here's one way:
# 
#   1. Take a fast block cipher and use it as C.
#   2. Make H pretty small. I won't look down on you if it's only 16
#      bits. Pick some initial H.
#   3. H is going to be the input key and the output block from C. That
#      means you'll need to pad it on the way in and drop bits on the
#      way out.
# 
# Now write the function f(n) that will generate 2^n collisions in this
# hash function.
# 
# Why does this matter? Well, one reason is that people have tried to
# strengthen hash functions by cascading them together. Here's what I
# mean:
# 
#   1. Take hash functions f and g.
#   2. Build h such that h(x) = f(x) || g(x).
# 
# The idea is that if collisions in f cost 2^(b1/2) and collisions in g
# cost 2^(b2/2), collisions in h should come to the princely sum of
# 2^((b1+b2)/2).
# 
# But now we know that's not true!
# 
# Here's the idea:
# 
#   1. Pick the "cheaper" hash function. Suppose it's f.
#   2. Generate 2^(b2/2) colliding messages in f.
#   3. There's a good chance your message pool has a collision in g.
#   4. Find it.
# 
# And if it doesn't, keep generating cheap collisions until you find it.
# 
# Prove this out by building a more expensive (but not TOO expensive)
# hash function to pair with the one you just used. Find a pair of
# messages that collide under both functions. Measure the total number
# of calls to the collision function.
# 
# // ------------------------------------------------------------
# 
# 53. Kelsey and Schneier's Expandable Messages
# 
# One of the basic yardsticks we use to judge a cryptographic hash
# function is its resistance to second preimage attacks. That means that
# if I give you x and y such that H(x) = y, you should have a tough time
# finding x' such that H(x') = H(x) = y.
# 
# How tough? Brute-force tough. For a 2^b hash function, we want second
# preimage attacks to cost 2^b operations.
# 
# This turns out not to be the case for very long messages.
# 
# Consider the problem we're trying to solve: we want to find a message
# that will collide with H(x) in the very last block. But there are a
# ton of intermediate blocks, each with its own intermediate hash
# state.
# 
# What if we could collide into one of those? We could then append all
# the following blocks from the original message to produce the original
# H(x). Almost.
# 
# We can't do this exactly because the padding will mess things up.
# 
# What we need are expandable messages.
# 
# In the last problem we used multicollisions to produce 2^n colliding
# messages for n*2^(b/2) effort. We can use the same principles to
# produce a set of messages of length (k, k + 2^k - 1) for a given k.
# 
# Here's how:
# 
# Starting from the hash function's initial state, find a collision
# between a single-block message and a message of 2^(k-1)+1 blocks. DO
# NOT hash the entire long message each time. Choose 2^(k-1) dummy
# blocks, hash those, then focus on the last block.
# 
# Take the output state from the first step. Use this as your new
# initial state and find another collision between a single-block
# message and a message of 2^(k-2)+1 blocks.
# 
# Repeat this process k total times. Your last collision should be
# between a single-block message and a message of 2^0+1 = 2 blocks.
# 
# Now you can make a message of any length in (k, k + 2^k - 1) blocks by
# choosing the appropriate message (short or long) from each pair.
# 
# Now we're ready to attack a long message M of 2^k blocks.
# 
#   1. Generate an expandable message of length (k, k + 2^k - 1) using
#      the strategy outlined above.
#   2. Hash M and generate a map of intermediate hash states to the
#      block indices that they correspond to.
#   3. From your expandable message's final state, find a single-block
#      "bridge" to intermediate state in your map. Note the index i it
#      maps to.
#   4. Use your expandable message to generate a prefix of the right
#      length such that len(prefix || bridge || M[i..]) = len(M).
# 
# The padding in the final block should now be correct, and your forgery
# should hash to the same value as M.
# 
# // ------------------------------------------------------------
# 
# 54. Kelsey and Kohno's Nostradamus Attack
# 
# Hash functions are sometimes used as proof of a secret prediction.
# 
# For example, suppose you wanted to predict the score of every Major
# League Baseball game in a season. (2,430 in all.) You might be
# concerned that publishing your predictions would affect the
# outcomes.
# 
# So instead you write down all the scores, hash the document, and
# publish the hash. Once the season is over, you publish the
# document. Everyone can then hash the document to verify your
# soothsaying prowess.
# 
# But what if you can't accurately predict the scores of 2.4k baseball
# games? Have no fear - forging a prediction under this scheme reduces
# to another second preimage attack.
# 
# We could apply the long message attack from the previous problem, but
# it would look pretty shady. Would you trust someone whose predicted
# message turned out to be 2^50 bytes long?
# 
# It turns out we can run a successful attack with a much shorter
# suffix. Check the method:
# 
#   1. Generate a large number of initial hash states. Say, 2^k.
#   2. Pair them up and generate single-block collisions. Now you have
#      2^k hash states that collide into 2^(k-1) states.
#   3. Repeat the process. Pair up the 2^(k-1) states and generate
#      collisions. Now you have 2^(k-2) states.
#   4. Keep doing this until you have one state. This is your
#      prediction.
#   5. Well, sort of. You need to commit to some length to encode in the
#      padding. Make sure it's long enough to accommodate your actual
#      message, this suffix, and a little bit of glue to join them
#      up. Hash this padding block using the state from step 4 - THIS is
#      your prediction.
# 
# What did you just build? It's basically a funnel mapping many initial
# states into a common final state. What's critical is we now have a big
# field of 2^k states we can try to collide into, but the actual suffix
# will only be k+1 blocks long.
# 
# The rest is trivial:
# 
#   1. Wait for the end of the baseball season. (This may take some
#      time.)
#   2. Write down the game results. Or, you know, anything else. I'm not
#      too particular.
#   3. Generate enough glue blocks to get your message length right. The
#      last block should collide into one of the leaves in your
#      tree.
#   4. Follow the path from the leaf all the way up to the root node and
#      build your suffix using the message blocks along the way.
# 
# The difficulty here will be around 2^(b-k). By increasing or
# decreasing k in the tree generation phase, you can tune the difficulty
# of this step. It probably makes sense to do more work up-front, since
# people will be waiting on you to supply your message once the event
# passes. Happy prognosticating!
# 
# // ------------------------------------------------------------
# 
# 55. MD4 Collisions
# 
# MD4 is a 128-bit cryptographic hash function, meaning it should take a
# work factor of roughly 2^64 to find collisions.
# 
# It turns out we can do much better.
# 
# The paper "Cryptanalysis of the Hash Functions MD4 and RIPEMD" by Wang
# et al details a cryptanalytic attack that lets us find collisions in
# 2^8 or less.
# 
# Given a message block M, Wang outlines a strategy for finding a sister
# message block M', differing only in a few bits, that will collide with
# it. Just so long as a short set of conditions* holds true for M.
# 
# What sort of conditions? Simple bitwise equalities within the
# intermediate hash function state, e.g. a[1][6] = b[0][6]. This should
# be read as: "the sixth bit (zero-indexed) of a[1] (i.e. the first
# update to 'a') should equal the sixth bit of b[0] (i.e. the initial
# value of 'b')".
# 
# It turns out that a lot of these conditions are trivial to enforce. To
# see why, take a look at the first (of three) rounds in the MD4
# compression function. In this round, we iterate over each word in the
# message block sequentially and mix it into the state. So we can make
# sure all our first-round conditions hold by doing this:
# 
#   # calculate the new value for a[1] in the normal fashion
#   a[1] = (a[0] + f(b[0], c[0], d[0]) + m[0]).lrot(3)
# 
#   # correct the erroneous bit
#   a[1] ^= ((a[1][6] ^ b[0][6]) << 6)
# 
#   # use algebra to correct the first message block
#   m[0] = a[1].rrot(3) - a[0] - f(b[0], c[0], d[0])
# 
# Simply ensuring all the first round conditions puts us well within the
# range to generate collisions, but we can do better by correcting some
# additional conditions in the second round. This is a bit trickier, as
# we need to take care not to stomp on any of the first-round
# conditions.
# 
# Once you've adequately massaged M, you can simply generate M' by
# flipping a few bits and test for a collision. A collision is not
# guaranteed as we didn't ensure every condition. But hopefully we got
# enough that we can find a suitable (M, M') pair without too much
# effort.
# 
# Implement Wang's attack.
# 
# * How did they work out the conditions? I'm going to be honest with
#   you: I have no f'ing clue.
# 
# // ------------------------------------------------------------
# 
# 56. RC4 Single-Byte Biases
# 
# RC4 is popular stream cipher notable for its usage in protocols like
# TLS, WPA, RDP, &c.
# 
# It's also susceptible to significant single-byte biases, especially
# early in the keystream. What does this mean?
# 
# Simply: for a given position in the keystream, certain bytes are more
# (or less) likely to pop up than others. Given enough encryptions of a
# given plaintext, an attacker can use these biases to recover the
# entire plaintext.
# 
# Now, search online for "On the Security of RC4 in TLS and WPA". This
# site is your one-stop shop for RC4 information.
# 
# Click through to "RC4 biases" on the right.
# 
# These are graphs of each single-byte bias (one per page). Notice in
# particular the monster spikes on z16, z32, z48, etc. (Note: these are
# one-indexed, so z16 = keystream[15].)
# 
# How useful are these biases?
# 
# Click through to the research paper and scroll down to the simulation
# results. (Incidentally, the whole paper is a good read if you have
# some spare time.) We start out with clear spikes at 2^26 iterations,
# but our chances for recovering each of the first 256 bytes approaches
# 1 as we get up towards 2^32.
# 
# There are two ways to take advantage of these biases. The first method
# is really simple:
# 
#   1. Gain exhaustive knowledge of the keystream biases.
#   2. Encrypt the unknown plaintext 2^30+ times under different keys.
#   3. Compare the ciphertext biases against the keystream biases.
# 
# Doing this requires deep knowledge of the biases for each byte of the
# keystream. But it turns out we can do pretty well with just a few
# useful biases - if we have some control over the plaintext.
# 
# How? By using knowledge of a single bias as a peephole into the
# plaintext.
# 
# Decode this secret:
# 
#   QkUgU1VSRSBUTyBEUklOSyBZT1VSIE9WQUxUSU5F
# 
# And call it a cookie. No peeking!
# 
# Now use it to build this encryption oracle:
# 
#   RC4(your-request || cookie, random-key)
# 
# Use a fresh 128-bit key on every invocation.
# 
# Picture this scenario: you want to steal a user's secure cookie. You
# can spawn arbitrary requests (from a malicious plugin or somesuch) and
# monitor network traffic. (Ok, this is unrealistic - the cookie
# wouldn't be right at the beginning of the request like that - this is
# just an example!)
# 
# You can control the position of the cookie by requesting "/", "/A",
# "/AA", and so on.
# 
# Build bias maps for a couple chosen indices (z16 and z32 are good) and
# decrypt the cookie.