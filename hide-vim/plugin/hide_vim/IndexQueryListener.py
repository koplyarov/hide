from threading import Condition

import hide

from Utils import LocationAsVimDictionary

class IndexQueryModelRow:
    def __init__(self, type, entry):
        self.__type = type
        self.__entry = entry if type == 'serviceMsg' else hide.IndexQueryEntry(entry)

    def ToVimString(self):
        if self.__type == 'serviceMsg':
            return self.__entry
        else:
            return self.__entry.GetName() + " (" + self.__entry.GetLocation().GetFilename() + ")"

    def GetLocationAsVimDictionary(self):
        if self.__type == 'queryEntry':
            return LocationAsVimDictionary(self.__entry.GetLocation())
        else:
            return '{}'


class IndexQueryListener(hide.IIndexQueryListener):
    def __init__(self, model):
        super(IndexQueryListener, self).__init__()
        self.__conditionVar = Condition()
        self.__model = model
        self.__finished = False

    def OnEntry(self, entry):
        self.__model.Insert(-1, IndexQueryModelRow('queryEntry', entry))

    def OnFinished(self):
        with self.__conditionVar:
            self.__finished = True
            self.__conditionVar.notifyAll()
            self.__model.Remove(-1)

    def WaitForFinished(self, timeout):
        with self.__conditionVar:
            self.__conditionVar.wait(timeout)
            return self.__finished

    def Finished(self):
        with self.__conditionVar:
            return self.__finished
