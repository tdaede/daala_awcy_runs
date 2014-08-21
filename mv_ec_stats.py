#!/usr/bin/env python

import simplejson as json
from math import log

def estimate_bytes_with_static_model(records, model):
    bits = 0.0
    for r in records:
        bits += -log(model[r[0]], 2)
    return bits / 8.0

def estimate_bytes_with_2d_probs(count0s, count1s, probs):
    bits = 0.0
    for i in range(len(count0s)):
        for j in range(len(count0s[0])):
            bits += -log(probs[i][j], 2) * count1s[i][j] - log(1.0 - probs[i][j], 2) * count0s[i][j]
    return bits / 8.0

acct_data = json.loads(open("ec-acct.json").read())

total = [0, 0, 0, 0, 0]
valid = [0, 0, 0, 0, 0]
total_bytes = [0, 0, 0, 0, 0]
frames = 0
for frame in acct_data:
    frame_total = [0, 0, 0, 0, 0]
    frame_valid = [0, 0, 0, 0, 0]

    count0s = [[0, 0, 0],
               [0, 0, 0],
               [0, 0, 0]]
    count1s = [[0, 0, 0],
               [0, 0, 0],
               [0, 0, 0]]
    probs = [[0.0, 0.0, 0.0],
             [0.0, 0.0, 0.0],
             [0.0, 0.0, 0.0]]

    print "frame %d:" % frames

    for level in range(1, 5):
        label = "mvf-l%d" % level
        if frame.has_key("mvf-l%d" % level):
            for r in frame["mvf-l%d" % level]:
                frame_total[level] += 1
                if r[0] == 1:
                    frame_valid[level] += 1
                if level == 2:
                    count0s[r[2] + r[3]][r[4] + r[5]] += int(r[0] == 0)
                    count1s[r[2] + r[3]][r[4] + r[5]] += int(r[0] == 1)

        total[level] += frame_total[level]
        valid[level] += frame_valid[level]

        print "  level %d:  %0.2f%% valid of %d flags" % \
            (level,
             frame_valid[level] * 100.0 / frame_total[level] if frame_total[level] > 0 else 0,
             frame_total[level])

        if level == 2:
            for i in range(3):
                for j in range(3):
                    probs[i][j] = (2 * float(count1s[i][j]) + 0.5) / (2 * (count0s[i][j] + count1s[i][j])  + 1)
                    print " %0.1f " % (probs[i][j] * 100.0)
            print

        if frame.has_key(label):
            if level in [1, 3]:
                est = estimate_bytes_with_static_model(frame[label], [0.25, 0.75])
                total_bytes[level] += est
                print "  estimated bytes %0.1f" % est
            elif level == 4:
                est = estimate_bytes_with_static_model(frame[label], [0.1, 0.99])
                total_bytes[level] += est
                print "  estimated bytes %0.1f" % est
            elif level == 2:
                est = estimate_bytes_with_2d_probs(count0s, count1s, probs)
                est_static = estimate_bytes_with_static_model(frame[label], [0.25, 0.75])
                total_bytes[level] += est
                print "  estimate bytes %0.1f (static %0.1f)" % (est, est_static)

    frames += 1

print "TOTAL:"

for level in range(1, 5):
    print "  level %d: %0.2f%% valid of %d flags, %0.1f Bpf" % \
        (level,
         valid[level] * 100.0 / total[level] if total[level] > 0 else 0,
         total[level],
         total_bytes[level] / frames)
