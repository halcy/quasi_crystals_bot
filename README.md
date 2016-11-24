# quasicrystals-bot

Bot that posts quasicrystal-like gif animations to Twitter and Mastodon.

Quasicrystal renderer is based on WAHa_06x36s NoRegifuge gif output code
with improvements to quantization stability for animation. The code is
included here for ease of use, but if you write any new stuff or change
anything, please contribute to upstream: https://bitbucket.org/WAHa_06x36/noregifuge

Uses the python "twitter" module for twitter integration and 
https://github.com/halcy/Mastodon.py for Mastodon integration. To run,
install these plus other requirements, built the quasicrystal renderer
(just run make), then create twitter_credentials.secret with your consumer
key, consumer secret, access key and access secret (one per line) and
mastodon_client.secret as well as mastodon_user.secret, as output by
Mastodon.py, and run the quasibot.py.

* On twitter: http://twitter.com/quasi_crystals
* On Mastodon: https://mastodon.social/users/quasi_crystals

* Quasicrystals (Wikipedia): https://en.wikipedia.org/wiki/Quasicrystal
