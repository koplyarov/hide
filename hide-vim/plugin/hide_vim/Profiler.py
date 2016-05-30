import hide
import time


class ProfilerResult:
    def __init__(self, wallTime, clockTime):
        self.wallTime = wallTime
        self.clockTime = clockTime

    def __str__(self):
        return "{{ wall: {}, clock: {} }}".format(ProfilerResult.__FormatTimeDuration(self.wallTime), ProfilerResult.__FormatTimeDuration(self.clockTime))

    def __FormatTimeDuration(d):
        h = int(d / 3600)
        m = int(d / 60) % 60
        s = int(d) % 60
        ms = int(d * 1000) % 1000
        us = int(d * 1000000) % 1000

        result = ""

        if h > 0:
            result += "{}h".format(h)

        if m > 0 or result:
            result += "{}{}m".format(" " if result else "", m)

        if s > 0 or result:
            result += "{}{}".format(" " if result else "", s)
            if ms > 0:
                result += ".{:03d}".format(ms)
            result += "s"

        if ms > 0 and not result:
            result += "{}".format(ms)
            if us > 0:
                result += ".{:03d}".format(ms)
            result += "ms"

        if us > 0 and not result:
            result += "{}us".format(us)

        return result

class Profiler:
    def __init__(self):
        self.wallTime = time.time()
        self.clockTime = time.clock()
        self.Reset()

    def Reset(self):
        newWallTime = time.time()
        newClockTime = time.clock()
        result = ProfilerResult(newWallTime - self.wallTime, newClockTime - self.clockTime)
        self.wallTime = newWallTime
        self.clockTime = newClockTime
        return result
