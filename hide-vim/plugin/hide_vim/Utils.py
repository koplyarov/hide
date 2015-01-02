def LocationAsVimDictionary(l):
    return '{ "filename": "' + l.GetFilename() + '", "line": ' + str(l.GetLine()) + ', "column": ' + str(l.GetColumn()) + ' }'
