def EscapePathForVim(p):
    return p.replace('\\', '\\\\')

def LocationAsVimDictionary(l):
    return '{{ "filename": "{}", "line": {}, "column": {} }}'.format(EscapePathForVim(l.GetFilename()), int(l.GetLine()), int(l.GetColumn()))
