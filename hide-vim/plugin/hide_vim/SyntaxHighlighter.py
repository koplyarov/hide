from collections import defaultdict
from threading import Lock

from .Profiler import *
from .Adapters import iteritems

import hide
import traceback
import vim


class FileEntry:
    def __init__(self):
        self.words = dict()
        self.categories = dict()
        self.__commandsCache = []

    def GetCommands(self):
        if not self.__commandsCache:
            cmd_cache = []
            for category, words in iteritems(self.categories):
                syntax_cmd_init = 'syn keyword HideHighlight{} '.format(category)
                syntax_cmd_len = len(syntax_cmd_init)
                cmd_assembler = [ syntax_cmd_init ]
                for word, (dummy, vim_name, use_match) in iteritems(words):
                    if use_match:
                        if vim_name:
                            cmd_cache.append(vim_name)
                        continue

                    if syntax_cmd_len + 1 + len(vim_name) >= 512:
                        cmd_cache.append(' '.join(cmd_assembler))
                        syntax_cmd_len = len(syntax_cmd_init)
                        cmd_assembler = [ syntax_cmd_init ]

                    cmd_assembler.append(vim_name)
                    syntax_cmd_len += len(vim_name) + 1

                if len(cmd_assembler) > 1:
                    cmd_cache.append(' '.join(cmd_assembler))

            self.__commandsCache = cmd_cache
        return self.__commandsCache

    def InvalidateCommands(self):
        self.__commandsCache = []


class SyntaxHighlighterListener(hide.IContextUnawareSyntaxHighlighterListener):
    __bannedWords = { 'contains', 'oneline', 'fold', 'display', 'extend', 'concealends' }

    def __init__(self, mutex, files):
        super(SyntaxHighlighterListener, self).__init__()
        self.__mutex = mutex
        self.__files = files

    def OnWordsChanged(self, filename, diff):
        with self.__mutex: # TODO: optimize mutex locks
            try:
                file_entry = self.__files[filename]
            except KeyError:
                file_entry = self.__files[filename] = FileEntry()

            banned_words = SyntaxHighlighterListener.__bannedWords
            words = file_entry.words
            categories = file_entry.categories

            for w in diff.GetRemoved():
                word = w.GetWord()
                try:
                    word_entry = words.pop(word)
                    category = word_entry[1]
                    category_entry = categories[category]
                    del category_entry[word]
                except KeyError:
                    pass

            for w in diff.GetAdded():
                word = w.GetWord()
                category = w.GetCategory()
                try:
                    word_entry = words[word]
                except KeyError:
                    word_lower = word.lower()
                    if word_lower in banned_words:
                        vim_name = r'syn match {} /\<{}\>/'.format(category, word) if word_lower != 'concealends' else ''
                        use_match = True
                    else:
                        vim_name = word.replace(r'|', r'\|')
                        use_match = False
                    word_entry = words[word] = (category, vim_name, use_match)

                try:
                    category_entry = categories[category]
                    category_entry[word] = word_entry
                except KeyError:
                    categories[category] = { word: word_entry }

            file_entry.InvalidateCommands()


class SyntaxHighlighterFileListener(hide.IContextUnawareSyntaxHighlighterFileListener):
    def __init__(self, mutex, visibleFiles):
        super(SyntaxHighlighterFileListener, self).__init__()
        self.__mutex = mutex
        self.__visibleFiles = visibleFiles
        self.__logger = hide.NamedLogger('SyntaxHighlighterFileListener')

    def OnVisibleFilesChanged(self, diff):
        with self.__mutex:
            visible_files = self.__visibleFiles
            for f in diff.GetRemoved():
                visible_files.remove(f)
            for f in diff.GetAdded():
                visible_files.add(f)


class SyntaxHighlighterShared:
    def __init__(self, highlighter):
        self.__logger = hide.NamedLogger('SyntaxHighlighter')
        self.mutex = Lock()
        self.files = dict()
        self.__listener = SyntaxHighlighterListener(self.mutex, self.files)
        self.highlighter = highlighter
        self.highlighter.AddListener(self.__listener)

    def __del__(self): # TODO: remove __del__ method
        try:
            self.highlighter.RemoveListener(self.__listener)
        except:
            self.__logger.Log(hide.LogLevel(hide.LogLevel.Error), '__del__ failed: '.format(traceback.format_exc(e)))


class SyntaxHighlighter:
    shared = None

    def __init__(self, project, filename):
        if not SyntaxHighlighter.shared:
            SyntaxHighlighter.shared = SyntaxHighlighterShared(project.GetContextUnawareSyntaxHighlighter())
        self.__logger = hide.NamedLogger('SyntaxHighlighter')
        self.__mutex = Lock()
        self.__visibleFiles = set()
        self.__listener = SyntaxHighlighterFileListener(self.__mutex, self.__visibleFiles)
        self.highlighterFile = SyntaxHighlighter.shared.highlighter.GetFileContext(filename)
        self.highlighterFile.AddListener(self.__listener)

    def close(self):
        try:
            self.highlighterFile.RemoveListener(self.__listener)
        except:
            self.__logger.Log(hide.LogLevel(hide.LogLevel.Error), 'close failed: '.format(traceback.format_exc(e)))

    def UpdateHighlights(self, forceFullUpdate):
        if not forceFullUpdate:
            return

        logger = self.__logger
        log_level_error = hide.LogLevel(hide.LogLevel.Error)

        p = Profiler()

        vim_commands = []
        self.__ResetHighlights()
        with SyntaxHighlighter.shared.mutex:
            with self.__mutex:
                for filename, file_entry in iteritems(SyntaxHighlighter.shared.files):
                    vim_commands += file_entry.GetCommands()

        logger.Log(log_level_error, 'UpdateHighlights phase 1: {}'.format(p.Reset()))

        for c in vim_commands:
            try:
                vim.command(c)
            except vim.error as e:
                logger.Log(log_level_error, 'exec failed ({}): {}'.format(e, c))

        logger.Log(log_level_error, 'UpdateHighlights phase 2: {}'.format(p.Reset()))

    def __ResetHighlights(self):
        vim.command('silent! syntax clear HideHighlightNamedConstant HideHighlightVariable HideHighlightFunction HideHighlightType HideHighlightKeyword')

