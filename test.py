#!/usr/bin/python

import sys

sys.path.insert(0, './bin')
import hide

p = hide.Project()
print p.GetLanguageName()
b = hide.Buffer('asdasda')
print b.GetName()
p.RemoveBuffer('')
p.AddBuffer(hide.Buffer(''))
