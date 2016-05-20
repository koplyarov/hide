#!/usr/bin/python

import sys
import time
import threading

sys.path.insert(0, './bin')
import hide

class LoggerSink(hide.ILoggerSink):
    def __init__(self):
        super(LoggerSink, self).__init__()
    def PrintMessage(self, msg):
        print(msg.ToString())

hide.SetCurrentThreadName('main')

sink = LoggerSink()
sink.__disown__()
hide.Logger.AddSink(sink);
hide.Logger.SetLogLevel(hide.LogLevel.Debug);

p = hide.Project.CreateAuto(['.*\\bCMakeFiles\\b.*', '.*\\.git\\b.*'])

time.sleep(45)

hide.Logger.RemoveSink(sink);
