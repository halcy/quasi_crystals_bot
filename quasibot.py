#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
import sys
import random
import hashlib
import os

import subprocess
from subprocess import call

from multiprocessing import Process, Queue

import twitter
from mastodon import Mastodon

with open("twitter_credentials.secret", 'r') as secret_file:
    TWITTER_CONSUMER_KEY = secret_file.readline().rstrip()
    TWITTER_CONSUMER_SECRET = secret_file.readline().rstrip()
    TWITTER_ACCESS_KEY = secret_file.readline().rstrip()
    TWITTER_ACCESS_SECRET = secret_file.readline().rstrip()

twitter_api = twitter.Api(consumer_key = TWITTER_CONSUMER_KEY,
                consumer_secret = TWITTER_CONSUMER_SECRET,
                access_token_key = TWITTER_ACCESS_KEY,
                access_token_secret = TWITTER_ACCESS_SECRET)

mastodon_api = Mastodon(
    client_id = "mastodon_client.secret", 
    access_token = "mastodon_user.secret"
)

wordfile = open("wordlist.txt", "r")
wordlist = wordfile.readlines()
wordfile.close()

# Thread handler that adds users that @ the bot to a queue
replyQueue = Queue()
def getReplies():
    lastId = None
    lastIdMastodon = None
    while(True):
        try:
            replies = None 
            if lastId == None:
                replies = twitter_api.GetMentions()
            else:
                print("Fetching replies since " + str(lastId))
                replies = twitter_api.GetMentions(since_id = lastId)
            
            if len(replies) > 0:
                lastId = replies[0].id
                replies.reverse()
                
            for reply in replies:
                replyQueue.put((reply.user.screen_name, "twitter"))
                print("New entry to reply queue: " + str(reply.user.screen_name))
        except:
            print("Hit rate limit in get-replies")
            time.sleep(60 * 5)
    
        try:
            replies = None
            if lastIdMastodon == None:
                replies = mastodon_api.timeline("mentions")
            else:
                print("Mastodon: Fetching replies since " + str(lastIdMastodon))
                replies = mastodon_api.timeline("mentions", since_id = lastIdMastodon)

            if len(replies) > 0:
                lastIdMastodon= replies[0]["id"]
                replies.reverse()

            for reply in replies:
                replyQueue.put((reply["account"]["acct"], "mastodon"))
                print("Mastodon: New entry to reply queue: " + str(reply["account"]["acct"]))
        except:
            print("Mastodon: Error in fetch replies")
            time.sleep(60 * 5)

        time.sleep(60 * 5)

Process(target = getReplies, daemon = True).start()

servedUsers = []
nextSeeds = []

# Clear user queue once on startup
time.sleep(5)
debugExclude = [("chimericalgirls", "mastodon"), ("halcy", "mastodon")]
while(not replyQueue.empty()):
   servedUser = replyQueue.get()
   if not servedUser in debugExclude:
       servedUsers.append(servedUser)

startTime = 0
pauseTime = 60 * 60 * 3
while(True):
    while(len(nextSeeds) == 0 or (time.time() - startTime > pauseTime)):
        print("Checking new work...")
        continueQueueGet = True
        while continueQueueGet:
            try:
                print("Polling user queues.")
                user = replyQueue.get(False)
            except:
                continueQueueGet = False
                break
            if user != None:
                if not user in servedUsers and not user in nextSeeds:
                    print("Appending " + str(user) + " to work queue.")
                    nextSeeds.append(user)
                    servedUsers.append(user)
                if(len(servedUsers) > 300):
                    servedUsers = servedUsers[len(servedUsers) - 300:]
            else:
                continueQueueGet = False
        if(time.time() - startTime > pauseTime):
            print("Time for seed post, prepending to work queue...")
            startTime = time.time()
            nextSeeds = [("___GEN___", "both")] + nextSeeds
            
        print("Sleeping until next check.")
        time.sleep(60)
    print("Current queue: " + str(nextSeeds))

    seeduser = nextSeeds.pop(0)
    seedphrase = seeduser[0]
    userSpec = True
    if(seedphrase == "___GEN___"):
        seedphrase = ""
        userSpec = False
        for i in range(0, 5):
                seedphrase += random.choice(wordlist).rstrip() + " "

    try:
        seedhash = str(abs(int(hashlib.sha1(seedphrase.encode("utf-8")).hexdigest(), 16)) % 4294967294)
        print("seedphrase (no hash): " + seedphrase)
        subprocess.call(["python3", "genquasi.py", seedhash])
        subprocess.call(["ffmpeg", "-f", "lavfi", "-i", "anullsrc=channel_layout=stereo:sample_rate=44100", "-f", "gif", "-i", seedhash + ".gif", "-shortest", "-c:v", "libx264", "-b:v", "1M", "-pix_fmt", "yuv420p", "-c:a", "aac", seedhash + "_nl.mp4"])
        
        listfile = open(seedhash + ".txt", "w")
        for i in range(0, 10):
            listfile.write("file '" + seedhash + "_nl.mp4'\n")
        listfile.close()
        
        subprocess.call(["ffmpeg", "-f", "concat", "-i", seedhash + ".txt", "-c:a", "copy", "-c:v", "copy", seedhash + ".mp4"])
        subprocess.call(["mv", seedhash + ".gif", "done"])
        subprocess.call(["mv", seedhash + ".mp4", "done"])
        subprocess.call(["rm", seedhash + "_nl.mp4"])
        subprocess.call(["rm", seedhash + ".txt"])
        
        mediaFile = "done/" + seedhash + ".gif"
        if os.path.getsize(mediaFile) > 1024 * 1024 * 4.9:
            mediaFile = "done/" + seedhash + ".mp4"
    except:
        print("Encountered error during encode. Link-only tweet.")
        mediaFile = None
    
    if not os.path.exists(mediaFile):
        mediaFile = None
    

    hqLink = "http://halcy.de/quasi/" + seedhash + ".gif"
    print("HQ link: " + hqLink)

    # Post to twitter
    try:
        if seeduser[1] == "twitter":
            if mediaFile != None:
                print("Twitter media upload... mediafile is " + mediaFile)
                mediaIdTwitter = [twitter_api.UploadMediaChunked(media = mediaFile)]
            else:
                mediaIdTwitter = None
        
            print("Tweeting...")
            if userSpec == False:
                twitter_api.PostUpdate("seed phrase: " + seedphrase + "(HQ: " + hqLink + " )", media = mediaIdTwitter)
            else:
                twitter_api.PostUpdate("@" + seedphrase + " here is your personal quasicrystal: (HQ: " + hqLink + " )" , media = mediaIdTwitter)
    except:
        print("Encountered error in post tweet. Whatever.")
        e = sys.exc_info()[0]
        print("Exception was: " + str(e))

    # Post to Mastodon
    try:
        if seeduser[1] == "mastodon":
            if mediaFile != None:
                print("Mastodon media upload... mediafile is " + mediaFile)
                mediaIdsMastodon = [mastodon_api.media_post(mediaFile)["id"]]
            else:
                mediaIdsMastodon = []
            
            print("Tooting...")
            if userSpec == False:
                mastodon_api.status_post("seed phrase: " + seedphrase + "(HQ: " + hqLink + " )", media_ids = mediaIdsMastodon)
            else:
                mastodon_api.status_post("@" + seedphrase + " here is your personal quasicrystal: (HQ: " + hqLink + " )", media_ids = mediaIdsMastodon)

    except:
        print("Encountered error in post toot. Whatever.")
        e = sys.exc_info()[0]
        print("Exception was: " + str(e))

