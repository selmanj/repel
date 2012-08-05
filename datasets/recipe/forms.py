#!/usr/bin/python
import re
import sys

forwardObsWeight = 1
backwardObsWeight = 1
smallWeight = 1
defWeight = 3

obsPredsWithObj= ["OnTable", "Above", "Touched"]
obsPredsNoObj = ["HandInFrame"]

def readObjects(filename):
    f = open(filename, 'r')
    objs = {}
    for line in f:
        m = re.search('OnTable\((\w+)\)', line)
        if m != None:
            objName = m.group(1)
            objs[objName] = None
    return objs

def printObsForms(objs):
    for pred in obsPredsWithObj:
        for obj in objs:
            print "{0}: [D-{1}({2}) -> {1}({2})]".format(forwardObsWeight, pred, obj)
            print "{0}: [{1}({2}) -> D-{1}({2})]".format(backwardObsWeight, pred, obj)
    for pred in obsPredsNoObj:
        print "{0}: [D-{1}() -> {1}()]".format(forwardObsWeight, pred)
        print "{0}: [{1}() -> D-{1}()]".format(forwardObsWeight, pred)

def am1(list, obj):
  preds = []
  for pred in list:
    preds.append("%s(%s)" % (pred, obj))
  formlist = []
  for predTrue in preds:
    form = []
    for pred in preds:
      if pred == predTrue:
        form.append(pred)
      else:
        form.append("!%s" % pred)
    formlist.append("(%s)" % (" ^ ".join(form)))
  formlist.append("(%s)" % (" ^ ".join(["!%s(%s)" % (pred, obj) for pred in list])))

  return "[ %s ]" % (" v ".join(formlist)) 

print "# List of forward/backward obs preds"
objs = readObjects(sys.argv[1])
printObsForms(objs)

print 
print "# InHand rule"
for obj1 in objs:
    for obj2 in objs:
        if obj1 == obj2:
            continue
        l  = ""
        l += "{0}: "
        l += "!<>{{m}} [OnTable({1})] v "
        l += "OnTable({1}) v "
        l += "!<>{{mi}} [OnTable({2})] v "
        l += "OnTable({2}) v "
        l += "<>{{d}} [!HandInFrame()]"
        r = ""
        r += "[InHand({1}, {2})]"
        print (l + " v " + r).format(defWeight, obj1, obj2)
        print (l + " v !<>{{m}} [!Above({1})] v " + r).format(defWeight, obj1, obj2) 
        print (l + " v !<>{{m}} [!Touched({1})] v " + r).format(defWeight, obj1, obj2) 
        
        print "{0}: [InHand({1}, {2}) -> !OnTable({1})]".format(defWeight, obj1, obj2)
        print "{0}: [InHand({1}, {2}) -> !OnTable({2})]".format(defWeight, obj1, obj2)

print
print "# An object in hand only maps to a single blob"

for obj1 in objs:
    for obj2 in objs:
        if obj1 == obj2:
            continue
        for obj3 in objs:
            if obj3 == obj1 or obj3 == obj2:
                continue
            print "{0}: [!InHand({1}, {2})] v !<>{{*}} [InHand({1}, {3})]".format(defWeight, obj1, obj2, obj3)
            print "{0}: [!InHand({1}, {2})] v !<>{{*}} [InHand({3}, {2})]".format(defWeight, obj1, obj2, obj3)

# TODO: bias an inhand so that other inhand events don't occur during

print
print "# Prefer inhand events where nothing changes on/off the table"
for obj1 in objs:
    for obj2 in objs:
        if obj1 == obj2:
            continue
        for obj3 in objs:
            if obj3 == obj1 or obj3 == obj2:
                continue
            print "{0}: !InHand({1}, {2}) v (!OnTable({3}) ; OnTable({3}))".format(smallWeight, obj1, obj2, obj3)
# TODO: consider possibly adding rule preferring shorter inhands
# Add Place/Deplace
