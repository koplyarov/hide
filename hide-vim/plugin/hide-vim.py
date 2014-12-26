import sys
from threading import RLock

sys.path.insert(0, './bin')
import hide


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


class LogModelRow:
    def __init__(self, msg):
        self.__msg = msg
        self.__str = self.__ToVimString()

    def __ToVimString(self):
        return str(self.__msg)

    def ToVimString(self):
        return self.__str


class BuildLogModelRow:
    def __init__(self, type, line):
        self.__type = type
        self.__line = line
        self.__str = self.__ToVimString()

    def __ToVimString(self):
        if self.__type == 'serviceMsg':
            return self.__line
        else:
            if self.__line.GetIssue() is None:
                return self.__line.GetText()
            else:
                issue = self.__line.GetIssue()
                return str(issue.GetType()).upper() + ': ' + issue.GetText()

    def ToVimString(self):
        return self.__str


class LoggerSink(hide.ILoggerSink):
    def __init__(self, model):
        super(LoggerSink, self).__init__()
        self.__model = model

    def PrintMessage(self, msg):
        self.__model.Append(LogModelRow(msg))


class BuildProcessListener(hide.IBuildProcessListener):
    def __init__(self, model, mutex):
        super(BuildProcessListener, self).__init__()
        self.__mutex = mutex
        self.__model = model
        self.finished = False

    def OnLine(self, line):
        self.__model.Append(BuildLogModelRow('buildLogMsg', line))

    def OnFinished(self, succeeded):
        with self.__mutex:
            self.finished = True
            self.succeeded = succeeded
            self.__model.Append(BuildLogModelRow('serviceMsg', 'BUILD ' + ('SUCCEEDED' if succeeded else 'FAILED')))


class HidePlugin:
    def __init__(self):
        hide.SetCurrentThreadName('vim')

        self.__mutex = RLock()
        self.logModel = Model(self.__mutex)
        self.buildLogModel = Model(self.__mutex)

        self.loggerSink = LoggerSink(self.logModel)
        hide.Logger.AddSink(self.loggerSink)
        hide.Logger.SetLogLevel(hide.LogLevel.Debug)

        self.__buildProcess = None
        self.__buildProcessListener = None

        self.project = hide.Project.CreateAuto(['.*\\bCMakeFiles\\b.*', '.*\\.git\\b.*'])

    def GetBuildTargets(self):
        return self.project.GetBuildSystem().GetTargets()

    def BuildInProgress(self):
        with self.__mutex:
            return self.__buildProcessListener != None and not self.__buildProcessListener.finished

    def __DoBuild(self, targetName, buildFunc):
        with self.__mutex:
            if self.BuildInProgress():
                return False
            self.buildLogModel.Clear()
            self.buildLogModel.Append(BuildLogModelRow('serviceMsg', 'Building "' + targetName + '":'))
        self.__buildProcess = None
        self.__buildProcess = buildFunc(self.project.GetBuildSystem())
        self.__buildProcessListener = BuildProcessListener(self.buildLogModel, self.__mutex)
        self.__buildProcess.AddListener(self.__buildProcessListener)
        return True

    def BuildAll(self):
        return self.__DoBuild('all', lambda bs: bs.BuildAll())

    def BuildTarget(self, target):
        return self.__DoBuild(target, lambda bs: bs.BuildTarget(target))

    def BuildFile(self, filename):
        return self.__DoBuild(filename, lambda bs: bs.BuildFile(self.project.GetFileByPath(filename)))

    def __del__(self):
        hide.Logger.RemoveSink(self.loggerSink)
