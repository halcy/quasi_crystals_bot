import time
import sys
import random
import hashlib
import os
import atexit
import json

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
lastIdMastodon = None
try:
    replies = []
    replies_page = mastodon_api.notifications()
    try:
        for i in range(0, 10):
            print("Loaded page " + str(i))
            replies.extend(replies_page)
            replies_page = mastodon_api.fetch_next(replies_page)
    except:
        print("Reached end.")
    
    replies.reverse()
    for reply in replies:
        if reply["type"] == "mention":
            replyQueue.put((reply["status"]["account"]["acct"], "mastodon", reply["status"]["id"]))
            print("Mastodon: New entry to reply queue: " + str(reply["status"]["account"]["acct"]))
except Exception as e:
    print("Mastodon: Error in fetch replies: " + str(e))

time.sleep(10)
servedUsers = sys.argv[1:]
nextSeeds = []
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
