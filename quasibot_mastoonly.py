#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
import sys
import random
import hashlib
import os
import atexit
import json
import traceback

import subprocess
from subprocess import call

from multiprocessing import Process, Queue, Value

from mastodon import Mastodon

mastodon_api = Mastodon(
    api_base_url = "https://botsin.space/",
    access_token = "mastodon_user.secret"
)

wordfile = open("wordlist.txt", "r")
wordlist = wordfile.readlines()
wordfile.close()

# Thread handler that adds users that @ the bot to a queue
replyQueue = Queue()
userCount = Value('i', 0)
def getReplies():
    lastIdMastodon = None
    initialRun = True
    notedUsers = []
    if os.path.exists('served.json'):
        with open('served.json', 'r') as f:
            notedUsers = json.load(f)
    print("initial noted: " + str(notedUsers))
    while(True):
        try:
            replies = None
            if lastIdMastodon == None:
                replies = mastodon_api.notifications()
            else:
                print("Mastodon: Fetching replies since " + str(lastIdMastodon))
                replies = mastodon_api.notifications(since_id = lastIdMastodon)
                
            if len(replies) > 0:
                lastIdMastodon = replies[0]["id"]
                replies.reverse()
            
            if(len(notedUsers) > 300):
                notedUsers = notedUsers[len(notedUsers) - 300:]
            
            for reply in replies:
                if reply["type"] == "mention":
                    replyQueue.put((reply["status"]["account"]["acct"], "mastodon", reply["id"]))
                    if initialRun == False and not reply["status"]["account"]["acct"] in notedUsers:
                        allowFor = "some time"
                        if userCount.value > 7:
                            allowFor = "several hours"
                        if userCount.value == 0:
                            mastodon_api.status_reply(reply["status"], "I'll get right on it! Please allow for a few minutes for quasicrystal generation.", visibility="direct", untag=True)
                        else:
                            mastodon_api.status_reply(reply["status"], "Request received! The number of quasicrystals ahead of yours in the queue is around " + str(userCount.value) + ". Please allow for " + allowFor + " until your quasicrystal is ready.", visibility="direct", untag=True)
                    notedUsers.append(reply["status"]["account"]["acct"])
                    userCount.value += 1
                    print("Mastodon: New entry to reply queue: " + str(reply["status"]["account"]["acct"]))
        except Exception as e:
            print("Mastodon: Error in fetch replies: " + str(e))
            time.sleep(60 * 5)
        initialRun = False
        time.sleep(60 * 5)

Process(target = getReplies, daemon = True).start()

servedUsers = []
nextSeeds = []

if os.path.exists("served.json"):
    with open('served.json', 'r') as f:
        servedUsers = json.load(f)
else:
    # Clear user queue once on startup
    time.sleep(10)
    debugExclude = []
    while(not replyQueue.empty()):
        servedUser = replyQueue.get()
        if not servedUser in debugExclude:
            servedUsers.append(servedUser[0])
        else:
            print("Excluding " + str(servedUser) + " from initial queue wipe due to debug exclude")
            nextSeeds.append(servedUser)

if os.path.exists("seeds.json"):
    with open('seeds.json', 'r') as f:
        nextSeeds = json.load(f)

def dump_state():
    global servedUsers
    global nextSeeds
    print("Exiting, dumping state")
    continueQueueGet = True
    while continueQueueGet:
        try:
            print("Polling user queues.")
            user = replyQueue.get(False)
        except:
            continueQueueGet = False
            break
        if user != None:
            if not user[0] in servedUsers and not user in nextSeeds:
                print("Appending " + str(user) + " to work queue.")
                nextSeeds.append(user)
                servedUsers.append(user[0])
            if(len(servedUsers) > 300):
                servedUsers = servedUsers[len(servedUsers) - 300:]
        else:
            continueQueueGet = False
    with open('served.json', 'w') as f:
        json.dump(servedUsers, f)
    with open('seeds.json', 'w') as f:
        json.dump(nextSeeds, f)
atexit.register(dump_state)

startTime = time.time()
pauseTime = 60 * 60 * 3
while(True):
    checkWorkOnce = True
    while(checkWorkOnce or len(nextSeeds) == 0 or (time.time() - startTime > pauseTime)):
        checkWorkOnce = False
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
                if not user[0] in servedUsers and not user in nextSeeds:
                    print("Appending " + str(user) + " to work queue.")
                    nextSeeds.append(user)
                    servedUsers.append(user[0])
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

    userCount.value = len(nextSeeds)
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
        if not os.path.exists("done/" + seedhash + ".gif"):
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
        if os.path.getsize(mediaFile) > 1024 * 1024 * 3.9:
            mediaFile = "done/" + seedhash + ".mp4"
    except:
        print("Encountered error during encode. Link-only tweet.")
        mediaFile = None
    
    if not os.path.exists(mediaFile):
        mediaFile = None
    

    hqLink = "http://aka-san.halcy.de/quasi/" + seedhash + ".gif"
    print("HQ link: " + hqLink)

    # Post to Mastodon
    try:
        if seeduser[1] == "mastodon" or seeduser[1] == "both":
            if mediaFile != None:
                print("Mastodon media upload... mediafile is " + mediaFile)
                mediaIdsMastodon = [mastodon_api.media_post(mediaFile)["id"]]
            else:
                mediaIdsMastodon = []
            
            print("Tooting...")
            if userSpec == False:
                mastodon_api.status_post("seed phrase: " + seedphrase + "(HQ: " + hqLink + " )", media_ids = mediaIdsMastodon)
            else:
                reply_status = mastodon_api.status(seeduser[2])
                mastodon_api.status_reply(reply_status, "here is your personal quasicrystal: (HQ: " + hqLink + " )", media_ids = mediaIdsMastodon, visibility="public", untag=True)

    except:
        print("Encountered error in post toot. Trying HQ only.")
        e = sys.exc_info()[0]
        print("Exception was: " + str(e))
        traceback.print_exc()
        
        try:
            if userSpec == False:
                mastodon_api.status_post("seed phrase: " + seedphrase + "(HQ: " + hqLink + " )")
            else:
                mastodon_api.status_post("@" + seedphrase + " here is your personal quasicrystal: (HQ: " + hqLink + " )")
        except:
             print("HQ only toot failed. Skipping.")
