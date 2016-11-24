# coding: utf-8

import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import random
import sys

from subprocess import call

seedphrase = sys.argv[1]
print("SEED: " + seedphrase)

random.seed(abs(int(seedphrase)))
np.random.seed(abs(int(seedphrase)))

cmaps = [
        'viridis', 'inferno', 'plasma', 'magma', 'Blues', 'BuGn', 'BuPu',
        'GnBu', 'Greens', 'Greys', 'Oranges', 'OrRd', 'PuBu', 'PuBuGn', 'PuRd', 
        'Purples', 'RdPu', 'Reds', 'YlGn', 'YlGnBu', 'YlOrBr', 'YlOrRd', 
        'afmhot', 'autumn', 'bone', 'cool', 'copper', 'gist_heat', 'gray', 'hot',
        'pink', 'spring', 'summer', 'winter', 'BrBG', 'bwr', 'coolwarm', 'PiYG', 
        'PRGn', 'PuOr', 'RdBu', 'RdGy', 'RdYlBu', 'RdYlGn', 'Spectral', 'seismic',
        'Accent', 'Dark2', 'Paired', 'Pastel1', 'Pastel2', 'Set1', 'Set2', 'Set3',
        'gist_earth', 'terrain', 'ocean', 'gist_stern', 'brg', 'CMRmap', 'cubehelix',
        'gnuplot', 'gnuplot2', 'gist_ncar', 'nipy_spectral', 'jet', 'rainbow',
        'gist_rainbow', 'hsv', 'flag', 'prism'
]

palOk = False
while palOk == False:
    # Palette generation.
    # Very ugly, but the results are fairly pretty.
    cmapAName = random.choice(cmaps)
    cmapBName = random.choice(cmaps)
    cmapA = mpl.cm.get_cmap(cmapAName)
    cmapB = mpl.cm.get_cmap(cmapBName)

    print("CMAP A: " + str(cmapAName))
    print("CMAP B: " + str(cmapBName))

    palette = []
    intendedSize = int(min(np.random.exponential(0.15), 1.0) * 254.0) + 1

    splitCmap = np.random.random() < 0.2

    print("Intended size: " + str(intendedSize))
    print("Split cmap: " + str(splitCmap))

    if splitCmap:
        firstSplitSize = np.floor(max(min(np.random.random(), 1.0), 0.0) * 256)
        intendedSizeSplit = intendedSize / 2
        palettePositionsA = np.round(np.linspace(0, 1, firstSplitSize) * intendedSizeSplit) / intendedSizeSplit
        palettePositionsB = np.round(np.linspace(0, 1, 256 - firstSplitSize) * intendedSizeSplit) / intendedSizeSplit
    
        for pos in palettePositionsA:
            palette.extend(np.floor(np.array(cmapA(pos)[0:3]) * 255))   
        for pos in palettePositionsB:
            palette.extend(np.floor(np.array(cmapB(pos)[0:3]) * 255))
    else:
        stripeyMap = False
        stripeyMix = False

        if(intendedSize > 20):
            stripeyMap = np.random.random() < 0.3
            stripeyMix = np.random.random() < 0.3

        print("Stripey map: " + str(stripeyMap))
        print("Stripey mix: " + str(stripeyMix))        

        if stripeyMap == False:
            palettePositions = np.round(np.linspace(0, 1, 256) * intendedSize) / intendedSize
            for pos in palettePositions:
                palette.extend(np.floor(np.array(cmapA(pos)[0:3]) * 255))
        else:
            stripeyDist = np.int(np.random.random() * 50.0) + 20
            stripeyStripes = np.int(256 / (intendedSize + stripeyDist))
            stripeyColA = np.random.random()
            stripeyColB = np.random.random()
            print("Stripey params: dist " + str(stripeyDist) + ", stripes " + str(stripeyStripes) + ", A " + str(stripeyColA) + ", B " + str(stripeyColB))
            for stripe in range(0, stripeyStripes):
                if stripeyMix == True:
                    stripeyColA = np.random.random()
                for pos in range(0, intendedSize):
                    palette.extend(np.floor(np.array(cmapA(stripeyColA)[0:3]) * 255))
                for pos in range(0, stripeyDist):
                    palette.extend(np.floor(np.array(cmapB(stripeyColB)[0:3]) * 255))
            for pos in range(0, 256 - stripeyStripes * (stripeyDist + intendedSize)):
                palette.extend(np.floor(np.array(cmapB(stripeyColB)[0:3]) * 255))
    
    rotPal = np.random.random() < 0.85
    print("Rot pal: " + str(rotPal))
    if rotPal:
        rotatedPalette = []
        vAdjust = min(1.0, np.random.random() + 0.6)
        print("vAdjust: " + str(vAdjust))
        hAdjust = np.random.random()
        print("hAdjust: " + str(hAdjust))
        for i in range(int(len(palette) / 3)):
            colorTriplet = palette[i * 3:i * 3 + 3]
            #print("---------------------------")
            #print("BEFORE RGB: " + str(colorTriplet))
            hsvTriplet = mpl.colors.rgb_to_hsv(np.array(colorTriplet) / 255.0)
            #print("BEFORE: " + str(hsvTriplet))
            hsvTriplet[2] *= vAdjust
            hsvTriplet[0] += hAdjust
            while hsvTriplet[0] > 1.0:
                hsvTriplet[0] -= 1.0
            #print("AFTER " + str(hsvTriplet))
            #print("AFTER RGB " + str(mpl.colors.hsv_to_rgb(hsvTriplet) * 255.0))
            rotatedPalette.extend(mpl.colors.hsv_to_rgb(hsvTriplet) * 255.0)
        palette = rotatedPalette
    palEnts = []

    # Attempt to estimate "Boringness" of a palette
    for i in range(int(len(palette) / 3)):
        colorTriplet = palette[i * 3:i * 3 + 3]
        palEnts.append(np.array([colorTriplet[0] + colorTriplet[1] + colorTriplet[2]]))
    palPicks = np.array([palEnts[0], palEnts[40], palEnts[80], palEnts[120], palEnts[160], palEnts[200], palEnts[255]])
    palDiffs = []
    smallDiffsCounter = -len(palPicks)
    for ent in palPicks:
        for ent2 in palPicks:
            palDiff = np.linalg.norm(ent - ent2)
            palDiffs.append(palDiff)
            if palDiff < 50:
                smallDiffsCounter += 1
    print("Small-differences count: " + str(smallDiffsCounter))
    if smallDiffsCounter < 9:
        palOk = True
    else:
        palOk = False
        print("Detected low-variety palette, retrying")

paletteString = " ".join(map(lambda x: str(max(0, min(255, int(x)))), list(palette)))

# Parameter set
allParams = []
setTwo = False
for i in range(0, 4):
    quasiParamA = int(min(np.random.exponential(0.4), 1.0) * 13.0) + 2
    quasiParamB = int(min(np.random.exponential(0.35), 1.0) * 55.0) + 1
    phaseMultiplier = 1
    if setTwo == False:
        if np.random.random() < 0.35:
            setTwo = True
            phaseMultiplier = 2
    if np.random.random() < 0.5:
        phaseMultiplier = -phaseMultiplier
    print("Param set " + str(i) + ": " + str(quasiParamA) + ", " + str(quasiParamB) + ", " + str(phaseMultiplier))
    allParams.extend([quasiParamA, quasiParamB, phaseMultiplier])

# Mixing arguments
argString = ""
pure = np.random.random() < 0.5
flow = np.random.random() < 0.3
mixType = np.random.random() < 0.7
polar = np.random.random() < 0.6
print("Mixing: Pure/Flow/Type/Polar " + str(pure) + "/" + str(flow) + "/" + str(mixType) + "/" + str(polar))

if pure:
    if polar:
        if flow:
            argString = "0.0 0.0 0.0 1.0 "
        else:
            argString = "0.0 1.0 0.0 0.0 "
    else:
        if flow:
            argString = "0.0 0.0 1.0 0.0 "
        else:
            argString = "1.0 0.0 0.0 0.0 "
else:
    if mixType:
        if flow:
            argString = "0.0 0.0 0.5 0.5 "
        else:
            argString = "0.5 0.5 0.0 0.0 "
    else:
        if flow:
            argString = "0.5 0.0 0.0 0.5 "
        else:
            argString = "0.0 0.5 0.5 0.0 "
            
# Generate call
argString += " ".join(map(lambda x: str(x), allParams))
argString += " " + paletteString + " " + seedphrase + ".gif -s 100 -o 1"

# Make quasicrystal
argString = "./Quasi " + argString
print("Call: " + str(argString))
call(argString.split(" "))
