from threading import RLock

import hide
import vim

class SyntaxHighlighterListener(hide.IContextUnawareSyntaxHighlighterListener):
    def __init__(self):
        super(SyntaxHighlighterListener, self).__init__()
        self.__mutex = RLock()
        self.__words = {}

    def OnWordCategoryChanged(self, word, category):
        with self.__mutex:
            self.__words[word] = hide.WordCategory(category)

    def GetChangedWords(self):
        with self.__mutex:
            words = self.__words
            self.__words = {}
            return words


class SyntaxHighlighter:
    def __init__(self, highlighter):
        self.__listener = SyntaxHighlighterListener()
        self.__highlighter = highlighter
        self.__highlighter.AddListener(self.__listener)

    def __del__(self):
        self.__highlighter.RemoveListener(self.__listener)

    def GetChangedWordsAsVimDictionary(self):
        words = self.__listener.GetChangedWords()
        return "{" + ",".join([ "'" + w + "':'" + words[w].ToString() + "'" for w in words ]) + "}"
