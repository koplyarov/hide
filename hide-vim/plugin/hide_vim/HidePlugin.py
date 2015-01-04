from threading import RLock

import hide

from BuildProcessListener import BuildProcessListener
from BuildProcessListener import BuildLogModelRow
from LoggerSink import LoggerSink
from Model import Model


class HidePlugin:
    def __init__(self):
        hide.SetCurrentThreadName('vim')

        self.__mutex = RLock()
        self.logModel = Model(self.__mutex)
        self.buildLogModel = Model(self.__mutex)

        self.loggerSink = LoggerSink(self.logModel)
        hide.Logger.AddSink(self.loggerSink)
        hide.Logger.SetLogLevel(hide.LogLevel.Debug)

        self.logger = hide.NamedLogger('HideVimPlugin')

        self.__buildProcess = None
        self.__buildProcessListener = None

        self.project = hide.Project.CreateAuto(['.*\\bCMakeFiles\\b.*', '.*\\.git\\b.*'])

    def __del__(self):
        self.InterruptBuild()
        hide.Logger.RemoveSink(self.loggerSink)

    def GetBuildTargets(self):
        return self.project.GetBuildSystem().GetTargets()

    def BuildInProgress(self):
        with self.__mutex:
            return self.__buildProcessListener != None and not self.__buildProcessListener.finished

    def InterruptBuild(self):
        with self.__mutex:
            if not self.BuildInProgress():
                return
        if not self.__buildProcess is None:
            self.__buildProcess.Interrupt()

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
