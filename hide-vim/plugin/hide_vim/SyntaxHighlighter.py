from threading import RLock

import hide
import vim

class SyntaxHighlighterListener(hide.IContextUnawareSyntaxHighlighterListener):
    def __init__(self):
        super(SyntaxHighlighterListener, self).__init__()
        self.__mutex = RLock()
        self.__words = {}
        self.__modified = False

    def OnWordCategoryChanged(self, word, category):
        with self.__mutex:
            self.__modified = True
            if category != hide.WordCategory(hide.WordCategory.NoneCategory):
                self.__words[word] = hide.WordCategory(category)
            elif word in self.__words:
                del self.__words[word]

    def GetWordsIfModified(self):
        with self.__mutex:
            if not self.__modified:
                return None
            self.__modified = False
            return self.__words.copy()


class SyntaxHighlighter:
    def __init__(self, highlighter):
        self.__listener = SyntaxHighlighterListener()
        self.__highlighter = highlighter
        self.__highlighter.AddListener(self.__listener)

    def __del__(self):
        self.__highlighter.RemoveListener(self.__listener)

    def GetWordsIfModifiedAsVimObj(self):
        words = self.__listener.GetWordsIfModified()
        if words is None:
            return vim.eval('string(g:hide#Utils#null)')
        return "{" + ",".join([ "'" + w + "':'" + words[w].ToString() + "'" for w in words ]) + "}"
