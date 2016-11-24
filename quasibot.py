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

CONSUMER_KEY = 'CHANGEME'
CONSUMER_SECRET = 'CHANGEME'
ACCESS_KEY = 'CHANGEME'
ACCESS_SECRET = 'CHANGEME'

api = twitter.Api(consumer_key=CONSUMER_KEY,
                consumer_secret=CONSUMER_SECRET,
                access_token_key=ACCESS_KEY,
                access_token_secret=ACCESS_SECRET)

wordfile = open("wordlist.txt", "r")
wordlist = wordfile.readlines()
wordfile.close()

replyQueue = Queue()
def getReplies():
    lastId = None
    while(True):
        try:
            replies = None 
            if lastId == None:
                replies = api.GetMentions()
            else:
                print("Fetching replies since " + str(lastId))
                replies = api.GetMentions(since_id = lastId)
            
            if len(replies) > 0:
                lastId = replies[0].id
                replies.reverse()
                
            for reply in replies:
                replyQueue.put(reply.user.screen_name)
                print("New entry to reply queue: " + str(reply.user.screen_name))
            time.sleep(60 * 5)
        except:
            print("Hit rate limit in get-replies")
            time.sleep(60 * 5)
    
Process(target = getReplies, daemon = True).start()

servedUsers = []
nextSeeds = []

# Clear user queue once on startup
time.sleep(60)
while(not replyQueue.empty()):
   servedUsers.append(replyQueue.get())
    
startTime = 0
pauseTime = 60 * 60 * 3
while(True):
    while(len(nextSeeds) == 0 or (time.time() - startTime > pauseTime)):
        print("Checking new work...")
        continueQueueGet = True
        while continueQueueGet:
            try:
                print("Polling user queue.")
                user = replyQueue.get(False)
            except:
                continueQueueGet = False
                break
            if user != None:
                if not user in servedUsers and not user in nextSeeds:
                    print("Appending " + user + " to work queue.")
                    nextSeeds.append(user)
                    servedUsers.append(user)
                if(len(servedUsers) > 300):
                    servedUsers = servedUsers[len(servedUsers) - 300:]
            else:
                continueQueueGet = False
        if(time.time() - startTime > pauseTime):
            print("Time for seed post, prepending to work queue...")
            startTime = time.time()
            nextSeeds = ["___GEN___"] + nextSeeds
            
        print("Sleeping until next check.")
        time.sleep(60)
    print("Current queue: " + str(nextSeeds))

    seedphrase = nextSeeds.pop(0)
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
    
    try:
        if mediaFile != None:
            print("Tweeting... mediafile is " + mediaFile)
            mediaId = [api.UploadMediaChunked(media = mediaFile)]
        else:
            mediaId = None
        hqLink = "http://halcy.de/quasi/" + seedhash + ".gif"
        print("HQ link: " + hqLink)
        
        if userSpec == False:
            api.PostUpdate("seed phrase: " + seedphrase + "(HQ: " + hqLink + " )", media = mediaId)
        else:
            api.PostUpdate("@" + seedphrase + " here is your personal quasicrystal: (HQ: " + hqLink + " )" , media = mediaId)
    except:
        print("Encountered error in post tweet. Whatever.")

