#!/usr/bin/python
import re
import os
import sys

if (len(sys.argv) < 2):
  print "usage: forms.py FACT-FILE"
  sys.exit(0)

filename = sys.argv[1]

players = ["p1", "p2", "p3", "p4"]
#tracks = ["t1", "t2", "t3", "t4"]
tracks = []
for i in range(1,121):
  tracks.append("t%d" % i)
player_preds = ["dribble", "pass"]

forward_obs_weight = 1
backward_obs_weight = 1
def_weight = 3 
simple_weight=1
events = {}

def read_file():
  facts = open(filename, 'r')
  for line in facts:
    if re.match('#',line):
      continue
    (name, when) = re.split("@", line.rstrip())
    name = name.strip()
    m = re.match("\[(\d+):(\d+)\]", when.strip())
    start = int(m.group(1))
    end = int(m.group(2))
    if name in events:
      events[name].append((start, end,))
    else:
      events[name] = [(start, end)]

def print_obs_preds():
  #for track in tracks:
    #print "%d: d-track(%s) -> track(%s)" % (forward_obs_weight, track, track) 
  #for track in tracks:
    #print "%d: track(%s) -> d-track(%s)" % (backward_obs_weight, track, track)
  return

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

def list_liq_intervals_intersect(li1, li2):
  for i1 in li1:
    for i2 in li2:
      if liq_intervals_intersect(i1, i2):
        return True
  return False

def events_overlap(e1name, e2name):
  if e1name not in events:
    return False
  if e2name not in events:
    return False
  return list_liq_intervals_intersect(events[e1name], events[e2name])

def liq_intervals_intersect(i1, i2):
  (i1start, i1end) = i1
  (i2start, i2end) = i2
  if (i2start <= i1end) and (i2start >= i1start):
    return True
  elif (i1start <= i2end) and (i1start >= i2start):
    return True
  else:
    return False

read_file()
print "# generated from forms.py"
print_obs_preds()

# not needed, we use a single formula to enforce hasTrack
#print
#print "# if we got a label for initTrack, apply it"
#for player in players:
#  for track in tracks:
#    print "%(w)d: initTrack(%(p)s, %(t)s) -> hasTrack(%(p)s, %(t)s)" % \
#      {"w":def_weight, "p":player, "t":track}

print "# hasTrack, if true, is true everywhere"
for p in players:
  for t in tracks:
    print "%(w)d: !<>{*} hasTrack(%(p)s, %(t)s) v hasTrack(%(p)s, %(t)s)" % \
      {"w":def_weight, "p":p, "t":t}

#print
#print "# at all times, a player has a track associated with them"
#print "# cnf rep of (hasTrack(p1, t1) ^ track(t1)) v (hasTrack(p1, t2) ^ track(t2)) v ..."
#for player in players:
#  pieces = []
#  for track in tracks:
#    piece = []
#    piece.append("hasTrack(%s, %s)" % (player, track))
#    piece.append("d-track(%s)" % track)
#    pieces.append(piece)
#  for i in range(2**len(pieces)):
#    k = i
#    piece = []
#    for j in range(len(pieces)):
#      piece.append(pieces[j][k%2])
#      k = k/2
#    print "%d: [ %s ]" % (def_weight, " v ".join(piece))

print
print "# a player cannot belong to two tracks at the same time"
for player in players:
  for t1 in tracks:
    for t2 in tracks:
      if t1 == t2:
        continue
      if events_overlap("d-track(%s)" % t1, "d-track(%s)" % t2):
        print "%(w)d: [ !hasTrack(%(p)s, %(t1)s) v !hasTrack(%(p)s, %(t2)s) v !d-track(%(t1)s) v !d-track(%(t2)s) ]" % \
          {"w":def_weight, "p":player, "t1":t1, "t2":t2}


print
print "# if two tracks are similar, then they belong to the same player"
for p in players:
  for t1 in tracks:
    for t2 in tracks:
      if t1 == t2:
        continue
      if "d-similar(%s, %s)" % (t1, t2) in events:
        print "%(w)d: [ !d-similar(%(t1)s, %(t2)s) v !hasTrack(%(p)s, %(t1)s) v hasTrack(%(p)s, %(t2)s) ]" % \
         {"w":simple_weight, "p":p, "t1":t1, "t2":t2}

print
print "# for each predicate, put a forward observation on the predicate"
for player in players:
  for track in tracks:
    for pred in player_preds: 
      if "d-%s(%s)" % (pred, track) in events:
        print "%(w)d: [ !d-%(pred)s(%(track)s) v !hasTrack(%(player)s, %(track)s) v %(pred)s(%(player)s) ]" % \
          {"pred":pred, "track":track, "w":simple_weight, "player":player}
      
print 
print "# if an event is true of a player, then we expect it to be true of the base predicate"
for pred in player_preds:
  for player in players:
    pieces = []
    for track in tracks:
      pieces.append("[hasTrack(%(player)s, %(track)s) ^ d-%(pred)s(%(track)s)]" % {"player":player, "pred":pred, "track":track})
    print "%d: [!%s(%s)] v %s" % (1, pred, player, " v ".join(pieces))
    #print "%(w)d: !%(pred)s(%(player)s) v 
#    pieces = []
#    for track in tracks:
#      piece = []
#      piece.append("hasTrack(%s, %s)" % (player, track))
#      piece.append("d-%s(%s)" % (pred, track))
#      pieces.append(piece)
#    for i in range(2**len(pieces)):
#      k = i
#      piece = []
#      for j in range(len(pieces)):
#        piece.append(pieces[j][k%2])
#        k = k/2
#      piece.append("!%s(%s)" % (pred, player))
#      print "%d: [ %s ]" % (1, " v ".join(piece))

print
print "## domain-specific"
print "# if a player is dribbling, they got passed the ball"
for p1 in players:
  for p2 in players:
    if p1 == p2:
      continue
    print "%(w)d: !dribble(%(p1)s) v <>{m} [ dribble(%(p1)s) v pass(%(p2)s) ]" % \
      {"w": simple_weight, "p1":p1, "p2":p2}
# 
# vim:set ts=2 sw=2 expandtab:
