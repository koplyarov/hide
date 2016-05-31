import sys

if sys.version_info < (3, 0):
    def iterkeys(i):
        return i.iterkeys()
    def itervalues(i):
        return i.itervalues()
    def iteritems(i):
        return i.iteritems()
else:
    def iterkeys(i):
        return i.keys()
    def itervalues(i):
        return i.values()
    def iteritems(i):
        return i.items()
