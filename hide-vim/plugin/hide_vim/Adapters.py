import sys

if sys.version_info < (3, 0):
    def iteritems(i):
        return i.iteritems()
else:
    def iteritems(i):
        return i.items()
