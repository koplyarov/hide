class ModelEvent:
    def __init__(self, type, begin, end):
        self.type = type
        self.begin = begin
        self.end = end

    def ToVimDictionary(self):
        return "{ 'type': '" + self.type + "', 'begin': " + str(self.begin) + ", 'end': " + str(self.end) + " }"


class Model:
    def __init__(self, mutex):
        self.__mutex = mutex
        self.__rows = []
        self.__events = []

    def Append(self, row):
        with self.__mutex:
            idx = len(self.__rows)
            self.__rows.append(row)
            self.__events.append(ModelEvent('inserted', idx, idx + 1))

    def Clear(self):
        with self.__mutex:
            self.__rows = []
            self.__events = [ ModelEvent('reset', 0, 0) ]

    def GetEvents(self):
        with self.__mutex:
            events = self.__events
            self.__events = []
            return events

    def GetRow(self, idx):
        with self.__mutex:
            return self.__rows[idx]
