from threading import RLock

import hide

from .Utils import LocationAsVimDictionary

class BuildLogModelRow:
    def __init__(self, type, line):
        self.__type = type
        self.__line = line if type == 'serviceMsg' else hide.BuildLogLine(line)

    def ToVimString(self):
        if self.__type == 'serviceMsg':
            return self.__line
        else:
            if self.__line.GetIssue() is None:
                return self.__line.GetText()
            else:
                issue = self.__line.GetIssue()
                return str(issue.GetType()).upper() + ': ' + issue.GetText()

    def GetLocationAsVimDictionary(self):
        if self.__type == 'buildLogMsg' and not (self.__line.GetIssue() is None):
            return LocationAsVimDictionary(self.__line.GetIssue().GetLocation())
        else:
            return '{}'


class BuildProcessListener(hide.IBuildProcessListener):
    def __init__(self, model):
        super(BuildProcessListener, self).__init__()
        self.__mutex = RLock()
        self.__model = model
        self.__finished = False

    def OnLine(self, line):
        self.__model.Append(BuildLogModelRow('buildLogMsg', line))

    def OnFinished(self, status):
        with self.__mutex:
            self.__finished = True
            self.__status = status
            self.__model.Append(BuildLogModelRow('serviceMsg', 'BUILD ' + str(status).upper()))

    def Finished(self):
        with self.__mutex:
            return self.__finished
