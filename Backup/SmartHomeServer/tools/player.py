#!/usr/bin/env python
import os, random, sys

print 'Number of arguments:', len(sys.argv), 'arguments.'
print 'Argument List:', str(sys.argv)

player = '/usr/bin/omxplayer '
music_path = sys.argv[1] #'./music/romance/'

list = os.listdir(music_path)
def rndmp3 ():
   randomfile = random.choice(list)
   list.remove(randomfile)
   file = '"' + music_path + randomfile + '"'
   os.system (player + file)

while list:
	rndmp3 ()