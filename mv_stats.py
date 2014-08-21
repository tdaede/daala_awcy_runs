#!/usr/bin/env python

import simplejson as json

acct_data = json.loads(open("acct.json").read())

frames = 0
mvf_frac_bits = [0, 0, 0, 0, 0] # first field unused
mv_frac_bits = [0, 0, 0, 0, 0]
for frame in acct_data:
    print "frame %d" % frames
    frames += 1
    mv_frac_bits[0] += frame["technique"]["motion-vectors0"]
    for i in range(1, 5):
        frame_fbits = frame["technique"]["motion-flags%d" % i]
        frame_vbits = frame["technique"]["motion-vectors%d" % i]

        mvf_frac_bits[i] += frame["technique"]["motion-flags%d" % i]
        mv_frac_bits[i] += frame["technique"]["motion-vectors%d" % i]

        print "  level %d> flags: %8.1f B    vectors: %8.1f B" % (i,
                                                              float(frame_fbits) / 64,
                                                                float(frame_vbits)/64)
mvf_bytes = [float(b) / 64 for b in mvf_frac_bits]
mv_bytes = [float(b) / 64 for b in mv_frac_bits]

print "mv data per frame:"
print "  level 0> flags:      --- Bpf  vectors: %8.1f Bpf" % (mv_bytes[0] / frames)
for i in range(1, 5):
    print "  level %d> flags: %8.1f Bpf  vectors: %8.1f Bpf" % (i,
                                                              mvf_bytes[i] / frames,
                                                              mv_bytes[i] / frames)


