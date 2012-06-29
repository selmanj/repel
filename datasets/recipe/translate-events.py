#!/usr/bin/python
import re

def printEvents(obj, name):
    # first do create/takeaway
    if len(obj['create']) != 1:
        raise Exception("only one create event allowed")
    if len(obj['takeaway']) != 1:
        raise Exception("only one takeaway event allowed")
    # on table when it is created until its taken away
    print "OnTable({}) @ [{}:{}]".format(name, obj['create'][0], obj['takeaway'][0])

    # now handle touch
    touchOn = sorted(obj['touch'])
    touchOff = sorted(obj['untouch'])

    if len(touchOn) > 0 or len(touchOff) > 0:
        # loop over every untouch, picking the touch preceedingi t
        while len(touchOff) > 0:
            endPoint = touchOff.pop()
            startPoint = None
            for i in touchOn:
                if i < endPoint:
                    startPoint = i
                else:
                    break
            touchOn.remove(startPoint)
            print "Touching({}) @ [{}:{}]".format(name, startPoint, endPoint)
        # if we have anything left in touchOn, its at the moment
        for i in touchOn:
            print "Touching({}) @ [{}:{}]".format(name, i, i)




f = open('events.txt', 'r')
p = re.compile('(\d+) (\w+) (\w+)')
eventmap = {}
for line in f:
    m = p.match(line)
    frame = m.group(1)
    event = m.group(2)
    obj = m.group(3)
    if obj not in eventmap:
        eventmap[obj] = {}
        eventmap[obj]['create'] = []
        eventmap[obj]['touch'] = []
        eventmap[obj]['untouch'] = []
        eventmap[obj]['occlude'] = []
        eventmap[obj]['unocclude'] = []
        eventmap[obj]['takeaway'] = []
        eventmap[obj]['reappear'] = []
    if event not in eventmap[obj]:
        print 'unknown event: {}'.format(event)
    eventmap[obj][event].append(frame)

printEvents(eventmap['object0001'], 'object0001')
