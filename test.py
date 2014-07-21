#!/usr/bin/python

import sys

sys.path.insert(0, './bin')
import hide

h = hide.Hide()
print h.GetLanguageName()
b = hide.Buffer('asdasda')
print b.GetName()
h.RemoveBuffer('')
h.AddBuffer(hide.Buffer(''))
