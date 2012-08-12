#!/usr/bin/python
#
# Generates synthetic recipe domains.
# RUNS ON PYTHON 3.x, not 2.7

# GENERAL DIRECTIONS
# 1. Plate items down
#   * Cup, bowl, cakemix
# 2. Pour cakemix into bowl
# 3. Mix bowl
# 4. Place oil bottle
# 5. pour oil bottle into mixing bowl
# 6. MIX
# 7. Place 1-2 more bowls, pour and mix
#
# So we mainly have variation in how we bring items out, and a bit in how we 
# pour/mix.  But essentially we will always be pouring and mixing.

# number of frames on average between actions
delayBetweenActsMean = 130
delayBetweenActsDev = 100

shortestLength = 20

putDownActsMean = 100
putDownActsDev = 50

pourActsMean = 200
pourActsDev = 75

mixActsMean = 400
mixActsDev = 200

pickupMean = 100
pickupDev = 30

import argparse
import random

def normalvariateMin(mean, stddev, min):
    sample = random.normalvariate(mean, stddev)
    if sample < min:
        sample = min
    return round(sample)

def interval(mean, stddev, offset):
    '''Get an interval for a generic event (including delay) from offset'''
    start = offset+normalvariateMin(mean, stddev, shortestLength)
    end = start+normalvariateMin(mean, stddev, shortestLength)
    return (start, end)

def placeInterval(offset):
    '''Get an interval for a place event (including delay) from offset'''
    start = offset+normalvariateMin(delayBetweenActsMean, delayBetweenActsDev, shortestLength)
    end = start+normalvariateMin(putDownActsMean, putDownActsDev, shortestLength)
    return (start, end)

def outputAsREPEL(basicEvents):
    # determine max interval
    maxStart = 1
    maxEnd = None
    for pred, start, end in basicEvents:
        if maxStart == None or maxStart > start:
            maxStart = start
        if maxEnd == None or maxEnd < end:
            maxEnd = end
    # add a small delay to the end
    maxEnd = maxEnd+normalvariateMin(delayBetweenActsMean, delayBetweenActsDev, shortestLength)

    output = ''
    
    for pred, start, end in basicEvents:
        if pred[0] == 'place':
            output += 'D-OnTable({0}) @ [{1} : {2}]\n'.format(pred[1], end+1, maxEnd)
            output += 'D-HandInFrame() @ [{0} : {1}]\n'.format(start, end+10)
        if pred[0] == 'pour':
            output += 'D-Pour() @ [{0} : {1}]\n'.format(start, end)
    print(output)
    

parser = argparse.ArgumentParser(description='Generate synthetic data.')
parser.add_argument('-s', '--seed', type=int, default=1)

args = parser.parse_args()
seed = args.seed

random.seed(seed)

# choose our order for putting down cup, bowl, then cakemix
bowlOrder = ['cup', 'bowl', 'cakemix']
random.shuffle(bowlOrder)

basicEvents = []
end = 0
for i in range(len(bowlOrder)):
    start, end = placeInterval(end)
    basicEvents.append((['place', bowlOrder.pop()], start, end))

# pour cakemix into bowl
start, end = interval(pourActsMean, pourActsDev, end)
basicEvents.append((['pour', 'cakemix', 'bowl'], start, end))
# perform a mix
start,end = interval(mixActsMean, mixActsDev, end)
basicEvents.append((['mix', 'bowl'], start, end))

outputAsREPEL(basicEvents)

