import hide

class LogModelRow:
    def __init__(self, msg):
        self.__msg = hide.LoggerMessage(msg)

    def ToVimString(self):
        return str(self.__msg)


class LoggerSink(hide.ILoggerSink):
    def __init__(self, model):
        super(LoggerSink, self).__init__()
        self.__model = model

    def PrintMessage(self, msg):
        self.__model.Append(LogModelRow(msg))
