#!/usr/bin/python

import sys

sys.path.insert(0, './bin')
import hide

p = hide.Project.CreateAuto(['.*\\bCMakeFiles\\b.*', '.*\\.git\\b.*'])
for f in p.GetFiles():
        print f.GetFilename()
p.GetBuildSystem().BuildAll()
