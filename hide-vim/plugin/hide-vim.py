import sys
from threading import RLock

sys.path.insert(0, './bin')
import hide

class LoggerSink(hide.ILoggerSink):
    def __init__(self, log, mutex):
        super(LoggerSink, self).__init__()
        self.log = log
        self.mutex = mutex

    def PrintMessage(self, msg):
        with self.mutex:
            self.log.append(str(msg))


class BuildProcessListener(hide.IBuildProcessListener):
    def __init__(self, buildLog, mutex):
        super(BuildProcessListener, self).__init__()
        self.buildLog = buildLog
        self.mutex = mutex
        self.finished = False

    def OnLine(self, line):
        with self.mutex:
            self.buildLog.append(line.GetText())

    def OnFinished(self, succeeded):
        with self.mutex:
            self.finished = True
            self.succeeded = succeeded
            self.buildLog.append("BUILD " + ("SUCCEEDED" if succeeded else "FAILED"))


class HidePlugin:
    def __init__(self):
        self.mutex = RLock()
        self.log = [ ]
        self.buildLog = [ ]
        self.buildProcess = None
        self.buildProcessListener = None
        self.sink = LoggerSink(self.log, self.mutex)
        hide.Logger.AddSink(self.sink);
        hide.Logger.SetLogLevel(hide.LogLevel.Debug);
        self.project = hide.Project.CreateAuto(['.*\\bCMakeFiles\\b.*', '.*\\.git\\b.*'])

    def GetLogLines(self, lineOfs):
        with self.mutex:
            newLen = len(self.log)
            return self.log[lineOfs : newLen + 1]

    def GetBuildLogLines(self, lineOfs):
        with self.mutex:
            newLen = len(self.buildLog)
            return self.buildLog[lineOfs : newLen + 1]

    def GetBuildTargets(self):
        return self.project.GetBuildSystem().GetTargets()

    def BuildInProgress(self):
        with self.mutex:
            return self.buildProcessListener != None and not self.buildProcessListener.finished

    def __DoBuild(self, targetName, buildFunc):
        with self.mutex:
            if self.BuildInProgress():
                return False
            self.buildLog = [ 'Building "' + targetName + '":' ]
        self.buildProcess = None
        self.buildProcess = buildFunc(self.project.GetBuildSystem())
        self.buildProcessListener = BuildProcessListener(self.buildLog, self.mutex)
        self.buildProcess.AddListener(self.buildProcessListener)
        return True

    def BuildAll(self):
        return self.__DoBuild('all', lambda bs: bs.BuildAll())

    def BuildTarget(self, target):
        return self.__DoBuild(target, lambda bs: bs.BuildTarget(target))

    def BuildFile(self, filename):
        return self.__DoBuild(filename, lambda bs: bs.BuildFile(self.project.GetFileByPath(filename)))

    def __del__(self):
        hide.Logger.RemoveSink(self.sink);
