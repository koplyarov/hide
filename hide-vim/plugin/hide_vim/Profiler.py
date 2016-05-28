import hide
import time


class Profiler:
    def __init__(self):
        self.wall_time = time.time()
        self.clock_time = time.clock()
        self.Reset()

    def Reset(self):
        new_wall_time = time.time()
        new_clock_time = time.clock()
        result = (new_wall_time - self.wall_time, new_clock_time - self.clock_time)
        self.wall_time = new_wall_time
        self.clock_time = new_clock_time
        return result
