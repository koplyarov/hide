#!/usr/bin/python

import sys

sys.path.insert(0, './bin')
import hide

class LoggerSink(hide.ILoggerSink):
    def __init__(self):
        super(LoggerSink, self).__init__()
    def PrintMessage(self, msg):
        print msg

sink = LoggerSink()
sink.__disown__()
hide.Logger.RegisterSink(sink);
hide.Logger.SetLogLevel(hide.LogLevel.Debug);

p = hide.Project.CreateAuto(['.*\\bCMakeFiles\\b.*', '.*\\.git\\b.*'])
#for f in p.GetFiles():
        #print f.GetFilename()

#p.GetBuildSystem().BuildAll()
p.GetBuildSystem().BuildFile(p.GetFileByPath('hide/Buffer.cpp'))
