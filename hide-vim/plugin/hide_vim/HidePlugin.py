from threading import RLock

import hide

from .Adapters import *
from .BuildProcessListener import *
from .IndexQueryListener import *
from .LoggerSink import *
from .Model import *
from .SyntaxHighlighter import *


class HidePlugin:
    def __init__(self):
        hide.SetCurrentThreadName('vim')

        self.logModel = Model()
        self.buildLogModel = Model()
        self.indexQueryModel = Model()

        self.loggerSink = LoggerSink(self.logModel)
        hide.Logger.AddSink(self.loggerSink)

        self.logger = hide.NamedLogger('HideVimPlugin')

        self.__buildProcess = None
        self.__buildProcessListener = None

        self.__indexQuery = None
        self.__indexQueryListener = None

        self.__syntaxHighlighters = {}

        self.project = hide.Project.CreateAuto(['^(.*/)?\\bCMakeFiles\\b(/.*)?$', '^(.*/)?\\.git\\b(/.*)?$'])

    def close(self):
        self.InterruptBuild()
        hide.Logger.RemoveSink(self.loggerSink)
        for sh in itervalues(self.__syntaxHighlighters):
            sh.close()
        self.__syntaxHighlighters.clear()

    def CreateSyntaxHighlighter(self, filename):
        self.logger.Log(hide.LogLevel.Debug, 'CreateSyntaxHighlighter(' + filename + ')')
        self.__syntaxHighlighters[filename] = SyntaxHighlighter(self.project, filename)

    def GetSyntaxHighlighter(self, filename):
        return self.__syntaxHighlighters[filename]

    def DeleteSyntaxHighlighter(self, filename):
        self.logger.Log(hide.LogLevel.Debug, 'DeleteSyntaxHighlighter({})'.format(filename))
        try:
            self.__syntaxHighlighters[filename].close()
        except Exception as e:
            self.logger.Log(hide.LogLevel.Debug, 'DeleteSyntaxHighlighter({}) failed: {}'.format(filename, traceback.format_exc(e)))
        del self.__syntaxHighlighters[filename]

    def GetBuildTargets(self):
        return self.project.GetBuildSystem().GetTargets()

    def BuildInProgress(self):
        return self.__buildProcessListener != None and not self.__buildProcessListener.Finished()

    def InterruptBuild(self):
        if not self.BuildInProgress():
            return
        if not self.__buildProcess is None:
            self.__buildProcess.Interrupt()

    def __DoBuild(self, targetName, buildFunc):
        if self.BuildInProgress():
                return False
        self.buildLogModel.Clear()
        self.buildLogModel.Append(BuildLogModelRow('serviceMsg', 'Building "' + targetName + '":'))
        self.__buildProcess = None
        self.__buildProcess = buildFunc(self.project.GetBuildSystem())
        self.__buildProcessListener = BuildProcessListener(self.buildLogModel)
        self.__buildProcess.AddListener(self.__buildProcessListener)
        return True

    def BuildAll(self):
        return self.__DoBuild('all', lambda bs: bs.BuildAll())

    def BuildTarget(self, target):
        return self.__DoBuild(target, lambda bs: bs.BuildTarget(target))

    def BuildFile(self, filename):
        return self.__DoBuild(filename, lambda bs: bs.BuildFile(self.project.GetFileByPath(filename)))

    def IndexQueryInProgress(self):
        return self.__indexQueryListener != None and not self.__indexQueryListener.Finished()

    def IndexQueryHasSingleMatch(self):
        return self.__indexQueryListener != None and self.__indexQueryListener.SingleMatch()

    def __DoStartIndexQuery(self, queryFunc):
        if self.IndexQueryInProgress():
            return False
        self.indexQueryModel.Clear()
        self.indexQueryModel.Append(IndexQueryModelRow('serviceMsg', 'Searching...'))
        self.__indexQuery = None
        self.__indexQuery = queryFunc(self.project.GetIndexer())
        self.__indexQueryListener = IndexQueryListener(self.indexQueryModel)
        self.__indexQuery.AddListener(self.__indexQueryListener)
        self.__indexQueryListener.WaitForFinished(0.2)
        return True

    def QuerySymbolsBySubstring(self, substring):
        return self.__DoStartIndexQuery(lambda indexer: indexer.QuerySymbolsBySubstring(substring))

    def QuerySymbolsByName(self, symbolName):
        return self.__DoStartIndexQuery(lambda indexer: indexer.QuerySymbolsByName(symbolName))
