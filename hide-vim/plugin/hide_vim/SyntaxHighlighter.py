from collections import defaultdict
from six import iteritems
from threading import Lock

from .Profiler import *

import hide
import vim


class SyntaxHighlighterListener(hide.IContextUnawareSyntaxHighlighterListener):
    def __init__(self):
        super(SyntaxHighlighterListener, self).__init__()
        self.__mutex = Lock()
        self.__files = dict()

    def OnVisibleFilesChanged(self, diff):
        pass

    def OnWordsChanged(self, filename, diff):
        if filename.endswith(".cpp"):
            pass
        with self.__mutex:
            try:
                file_entry = self.__files[filename]
            except KeyError:
                file_entry = self.__files[filename] = dict()

            for w in diff.GetRemoved():
                file_entry[w.GetWord()] = None
            for w in diff.GetAdded():
                file_entry[w.GetWord()] = w.GetCategory()

    def GetChangedFiles(self):
        with self.__mutex:
            files = self.__files
            self.__files = dict()
            return files


class SyntaxHighlighterFileEntry:
    def __init__(self, bannedWords):
        self.__vimCommands = []
        self.__words = dict()
        self.__bannedWords = bannedWords

    def ApplyDiff(self, words):
        added_words = { word: category for (word, category) in iteritems(words) if not category is None }
        removed_words = [ (word, category) for (word, category) in iteritems(words) if category is None ]

        for w, c in removed_words:
            del self.__words[w]

        self.__words.update(added_words)

        if not removed_words:
            update_vim_commands = self.__AddHighlights(added_words)
            self.__vimCommands += update_vim_commands
            return update_vim_commands
        else:
            self.__vimCommands += self.__AddHighlights(self.__words) # TODO: Reimplement this in a more efficient way
            return self.__vimCommands

    def GetVimCommands(self):
        return self.__vimCommands

    def __AddHighlights(self, addedWords):
        vim_commands = []
        banned_words = self.__bannedWords

        categories = defaultdict(list)
        for w, c in iteritems(self.__words):
            categories[c].append(w)

        for c, words in iteritems(categories):
            syntax_cmd_init = 'syn keyword HideHighlight{}'.format(c)
            syntax_cmd_len = len(syntax_cmd_init) + 1
            one_command_words = [ syntax_cmd_init ]
            for w in words:
                if w.lower() in banned_words:
                    if w.lower() != 'concealends':
                        vim_commands.append(r'syn match {} /\<{}\>/'.format(c, w))
                        pass
                    continue

                escaped_w = w.replace(r'|', r'\|')

                if syntax_cmd_len + 1 + len(escaped_w) >= 512:
                    vim_commands.append(' '.join(one_command_words))
                    syntax_cmd_len = len(syntax_cmd_init) + 1
                    one_command_words = [ syntax_cmd_init ]

                one_command_words.append(escaped_w)
                syntax_cmd_len += len(escaped_w) + 1

            if len(one_command_words) > 1:
                vim_commands.append(' '.join(one_command_words))

        return vim_commands



class SyntaxHighlighterShared:
    def __init__(self, highlighter):
        self.__logger = hide.NamedLogger('SyntaxHighlighter')
        self.__listener = SyntaxHighlighterListener()
        self.__highlighter = highlighter
        self.__highlighter.AddListener(self.__listener)
        self.__words = dict()
        self.__files = dict()
        self.__bannedWords = { 'contains', 'oneline', 'fold', 'display', 'extend', 'concealends' }

    def __del__(self):
        self.__highlighter.RemoveListener(self.__listener)

    def UpdateHighlights(self, forceFullUpdate):
        logger = self.__logger
        log_level_error = hide.LogLevel(hide.LogLevel.Error)
        files = self.__listener.GetChangedFiles()
        if not files and not forceFullUpdate:
            return

        vim_commands = []
        for f in iteritems(files):
            filename = f[0]
            diff = f[1]
            try:
                file_entry = self.__files[filename]
            except KeyError:
                file_entry = self.__files[filename] = SyntaxHighlighterFileEntry(self.__bannedWords)
            vim_commands += file_entry.ApplyDiff(diff)

        if forceFullUpdate:
            vim_commands = []
            self.__ResetHighlights()
            for f in iteritems(self.__files):
                file_entry = f[1]
                vim_commands += file_entry.GetVimCommands()

        p = Profiler()
        for c in vim_commands:
            try:
                vim.command(c)
            except vim.error as e:
                logger.Log(log_level_error, 'exec failed ({}): {}'.format(e, c))
        logger.Log(log_level_error, 'UpdateHighlights: {}'.format(p.Reset()))

    def __ResetHighlights(self):
        vim.command('silent! syntax clear HideHighlightNamedConstant HideHighlightVariable HideHighlightFunction HideHighlightType HideHighlightKeyword')


class SyntaxHighlighter:
    shared = None

    def __init__(self, highlighter):
        if not SyntaxHighlighter.shared: # TODO: Reimplement this in a better way
            SyntaxHighlighter.shared = SyntaxHighlighterShared(highlighter)

    def UpdateHighlights(self, forceFullUpdate):
        return SyntaxHighlighter.shared.UpdateHighlights(forceFullUpdate)

