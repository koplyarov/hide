#!/usr/bin/python

import sys

sys.path.insert(0, './bin')
import hide

class LoggerSink(hide.ILoggerSink):
    def __init__(self):
        super(LoggerSink, self).__init__()
    def PrintMessage(self, msg):
        print msg

class BuildLogListener(hide.IBuildLogListener):
    def __init__(self):
        super(BuildLogListener, self).__init__()
    def OnLine(self, line):
    print "BUILD: " + line.GetText()
    def OnFinished(self, succeeded):
        print "BUILD " + ("SUCCEEDED" if succeeded else "FAILED")

sink = LoggerSink()
sink.__disown__()
hide.Logger.AddSink(sink);
hide.Logger.SetLogLevel(hide.LogLevel.Debug);

p = hide.Project.CreateAuto(['.*\\bCMakeFiles\\b.*', '.*\\.git\\b.*'])
#for f in p.GetFiles():
        #print f.GetFilename()

build_log_listener = BuildLogListener()
build_log_listener.__disown__()
build_process = p.GetBuildSystem().BuildAll()
build_process.GetLog().AddListener(build_log_listener)
del build_process

#print p.GetBuildSystem().GetTargets()
#p.GetBuildSystem().BuildFile(p.GetFileByPath('hide/lang_plugins/cpp/clang/Index.cpp'))

hide.Logger.RemoveSink(sink);
