from threading import RLock

class ModelEvent:
    def __init__(self, type, index, row):
        self.type = type
        self.index = index
        self.row = row

    def ToVimDictionary(self):
        return "{ 'type': '" + self.type + "', 'index': " + str(self.index) + ", 'row': '" + (self.row.ToVimString().replace("'", "''") if not self.row is None else "") + "' }"


class Model:
    def __init__(self):
        self.__mutex = RLock()
        self.__rows = []
        self.__events = []

    def Append(self, row):
        with self.__mutex:
            idx = len(self.__rows)
            self.__rows.append(row)
            self.__events.append(ModelEvent('inserted', idx, row))

    def Insert(self, idx, row):
        with self.__mutex:
            if idx < 0:
                idx = idx + len(self.__rows)
            self.__rows.insert(idx, row)
            self.__events.append(ModelEvent('inserted', idx, row))

    def Remove(self, idx):
        with self.__mutex:
            if idx < 0:
                idx = idx + len(self.__rows)
            del self.__rows[idx]
            self.__events.append(ModelEvent('removed', idx, None))

    def Clear(self):
        with self.__mutex:
            self.__rows = []
            self.__events = [ ModelEvent('reset', 0, None) ]

    def GetEvents(self):
        with self.__mutex:
            events = self.__events
            self.__events = []
            return events

    def GetRow(self, idx):
        with self.__mutex:
            return self.__rows[idx]
