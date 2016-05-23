from collections import defaultdict
from six import iteritems
from threading import Lock

import hide
import vim


class SyntaxHighlighterListener(hide.IContextUnawareSyntaxHighlighterListener):
    def __init__(self):
        super(SyntaxHighlighterListener, self).__init__()
        self.__mutex = Lock()
        self.__words = dict()

    def OnWordsChanged(self, diff):
        with self.__mutex:
            for w in diff.GetRemoved():
                self.__words[w.GetWord()] = None
            for w in diff.GetAdded():
                self.__words[w.GetWord()] = w.GetCategory()

    def GetChangedWords(self):
        with self.__mutex:
            words = self.__words
            self.__words = dict()
            return words


class SyntaxHighlighter:
    def __init__(self, highlighter):
        self.__logger = hide.NamedLogger('SyntaxHighlighter')
        self.__listener = SyntaxHighlighterListener()
        self.__highlighter = highlighter
        self.__highlighter.AddListener(self.__listener)
        self.__words = dict()
        self.__bannedWords = { 'contains', 'oneline', 'fold', 'display', 'extend', 'concealends' }

    def __del__(self):
        self.__highlighter.RemoveListener(self.__listener)

    def UpdateHighlights(self, forceFullUpdate):
        words = self.__listener.GetChangedWords()
        if not words and not forceFullUpdate:
            return

        added_words = { word: category for (word, category) in iteritems(words) if not category is None }
        removed_words = [ (word, category) for (word, category) in iteritems(words) if category is None ]

        for w, c in removed_words:
            del self.__words[w]

        self.__words.update(added_words)

        if not removed_words and not forceFullUpdate:
            self.__AddHighlights(added_words)
        else:
            self.__SetHighlights()

    def __AddHighlights(self, addedWords):
        banned_words = self.__bannedWords
        logger = self.__logger
        log_level_error = hide.LogLevel(hide.LogLevel.Error)

        categories = defaultdict(list)
        for w, c in iteritems(self.__words):
            categories[c].append(w)

        for c, words in iteritems(categories):
            syntax_cmd_init = 'syn keyword HideHighlight%s' % c
            syntax_cmd = syntax_cmd_init
            for w in words:
                if w.lower() in banned_words:
                    if w.lower() != 'concealends':
                        vim.command(r'syn match %s /\<%s\>/' % (c, w))
                    continue

                escaped_w = w.replace(r'|', r'\|')

                if len(syntax_cmd) + 1 + len(escaped_w) >= 512:
                    try:
                        vim.command(syntax_cmd)
                    except vim.error as e:
                        logger.Log(log_level_error, 'exec failed (%s): %s' % (e.message, syntax_cmd))
                    syntax_cmd = syntax_cmd_init

                syntax_cmd = '%s %s' % (syntax_cmd, escaped_w)

            try:
                vim.command(syntax_cmd)
            except vim.error as e:
                logger.Log(log_level_error, 'exec failed (%s): %s' % (e.message, syntax_cmd))

    def __SetHighlights(self):
        self.__ResetHighlights()
        self.__AddHighlights(self.__words)

    def __ResetHighlights(self):
        vim.command('silent! syntax clear HideHighlightNamedConstant HideHighlightVariable HideHighlightFunction HideHighlightType HideHighlightKeyword')
