#!/usr/bin/python
import re
import sys

if len(sys.argv) != 2:
  print "Usage: %s FACT-FILE" % sys.argv[0]
  sys.exit(1)

players = ["p1"]
tracks = ["p1t1"]
player_preds = ["DoorOpen", "Driving", "ElevatorEnter", "ElevatorExit", "GateEnter", "GateExit", "InCar", "TrunkOpen", "Walking"]

forward_obs_weight = 1
backward_obs_weight = 1
hard_weight = 20
inf_weight = "inf"
init_track_weight = 5

events = {}

def read_file():
  facts = open(sys.argv[1], 'r')
  for line in facts:
    if re.match('#',line):
      continue
    if len(line.strip()) == 0:
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
  if (i1start > i2end) or (i2start > i1end):
    return False
  else:
    return True
  #if (i2start <= i1end) and (i2start >= i1start):
  #  return True
  #elif (i1start <= i2end) and (i1start >= i2start):
  #  return True
  #else:
  #  return False


def print_obs_preds():
  for track in tracks:
    for pred in player_preds:
      for player in players:
        print "%(w)d: [!D-Track(%(t)s) v !HasTrack(%(p)s,%(t)s) v !D-%(pr)s(%(t)s) v %(pr)s(%(p)s)]" % \
          {"w":forward_obs_weight, "t":track, "pr":pred, "p":player}
  for player in players:
    for pred in player_preds:
      fragments = []
      for track in tracks:
	if events_overlap("D-Track(%s)" % track, "D-%s(%s)" % (pred, track)):
          fragments.append("(HasTrack(%(p)s, %(t)s) ^ D-%(pr)s(%(t)s))" % {"p":player, "t":track, "pr":pred})
      if len(fragments) == 0:
        print "%(w)d: [!%(pr)s(%(p)s)]" % {"w":backward_obs_weight, "pr":pred, "p":player}
      else:
        print "%(w)d: [!%(pr)s(%(p)s) v %(form)s]" % \
          {"w":backward_obs_weight, "pr":pred, "p":player, "form":" v ".join(fragments)}
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

read_file()
print "# generated from forms.py"
print "# read %d predicates from fact file" % len(events)
print_obs_preds()

# not needed, we use a single formula to enforce hasTrack
#print
#print "# if we got a label for initTrack, apply it"
#for player in players:
#  for track in tracks:
#    print "%(w)d: initTrack(%(p)s, %(t)s) -> hasTrack(%(p)s, %(t)s)" % \
#      {"w":def_weight, "p":player, "t":track}

print "# HasTrack, if true, is true everywhere"
for p in players:
  for t in tracks:
    print "%(w)s: !<>{*} HasTrack(%(p)s, %(t)s) v HasTrack(%(p)s, %(t)s)" % \
      {"w":inf_weight, "p":p, "t":t}

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
      print "%(w)d: [ !HasTrack(%(p)s, %(t1)s) v !HasTrack(%(p)s, %(t2)s) v !D-Track(%(t1)s) v !D-Track(%(t2)s) ]" % \
        {"w":def_weight, "p":player, "t1":t1, "t2":t2}


print
print "# if two tracks are similar, then they belong to the same player"
for p in players:
  for t1 in tracks:
    for t2 in tracks:
      if t1 == t2:
        continue
      print "%(w)d: [ !D-Similar(%(t1)s, %(t2)s) v !HasTrack(%(p)s, %(t1)s) v HasTrack(%(p)s, %(t2)s) ]" % \
       {"w":simple_weight, "p":p, "t1":t1, "t2":t2}

print
print "# initialize tracks"
for p in players:
  print "%d: [HasTrack(%s, %st1)]" % (init_track_weight, p, p)

print
print "# a person can only be walking, in the car, or nothing"
for p in players:
  print "%s: %s" % (inf_weight, am1(["Walking", "InCar"], p))

print
print "# if a person is driving, they must be in the car"
for p in players:
  print "%s: [!Driving(%s) v InCar(%s)]" % (inf_weight, p, p)

print
print "# a person who is in the car cannot use the elevator"
for p in players:
  print "%s: [!InCar(%s) v (!ElevatorEnter(%s) ^ !ElevatorExit(%s))]" % (inf_weight, p, p, p)

print "# a person who is driving cannot use the gate"
for p in players:
  print "%s: [Driving(%s) v (!GateEnter(%s) ^ !GateExit(%s))]" % (inf_weight, p, p, p)

print "# if the trunk is open or the door is open, they are not driving"
for p in players:
  print "%s: [(!TrunkOpen(%s) ^ !DoorOpen(%s)) v !Driving(%s)]" % (inf_weight, p, p, p)

print "# entrances and exits go together"
for p in players:
  print "%s: !ElevatorEnter(%s) v <>{>} (GateExit(%s))" % (inf_weight, p, p)
  print "%s: !ElevatorExit(%s) v <>{<} (GateEnter(%s))" % (inf_weight, p, p)
  print "%s: !GateEnter(%s) v <>{>} (ElevatorExit(%s))" % (inf_weight, p, p)
  print "%s: !GateExit(%s) v <>{<} (ElevatorEnter(%s))" % (inf_weight, p, p)

print
print "# an entrance/exit specifies the beginning/end of all events"
for p in players:
  without_elevator_enter = ["%s(%s)" % (pred, p) for pred in player_preds if (pred != "ElevatorEnter")]
  without_elevator_exit = ["%s(%s)" % (pred, p) for pred in player_preds if (pred != "ElevatorExit")]
  without_gate_enter = ["%s(%s)" % (pred, p) for pred in player_preds if (pred != "GateEnter")]
  without_gate_exit = ["%s(%s)" % (pred, p) for pred in player_preds if (pred != "GateExit")]
  print "%s: !ElevatorEnter(%s) v <>{m} ElevatorEnter(%s) v !<>{m,o,fi,<} (%s)" % \
    (inf_weight, p, p, " v ".join(without_elevator_enter))
  print "%s: !ElevatorExit(%s) v <>{m} ElevatorExit(%s) v !<>{m,o,fi,<} (%s)" % \
    (inf_weight, p, p, " v ".join(without_elevator_exit))
  print "%s: !GateEnter(%s) v <>{m} GateEnter(%s) v !<>{m,o,fi,<} (%s)" % \
    (inf_weight, p, p, " v ".join(without_gate_enter))
  print "%s: !GateExit(%s) v <>{m} GateExit(%s) v !<>{m,o,fi,<} (%s)" % \
    (inf_weight, p, p, " v ".join(without_gate_exit))

print
print "# every person has an elevator use and a gate use"
for p in players:
  print "#%s: <>{*} ElevatorEnter(%s) v ElevatorExit(%s)" % (inf_weight, p, p)
  print "#%s: <>{*} GateEnter(%s) v GateExit(%s)" % (inf_weight, p, p)

print
print "# a person must walk between an elevator and car event"
for p in players:
  print "#%s: !(ElevatorEnter(%s) v ElevatorExit(%s) ^{<,>} TrunkOpen(%s) v DoorOpen(%s) v InCar(%s)) v <>{s,d} Walking(%s)" % \
    (inf_weight, p, p, p, p, p, p)
# 
# vim:set ts=2 sw=2 expandtab:
