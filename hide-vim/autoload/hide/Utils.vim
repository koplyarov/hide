function hide#Utils#Log(logLevel, text)
	execute 'python hidePlugin.logger.Log(hide.LogLevel.'.a:logLevel.', "'.string(a:text).'")'
endf

let g:hide#Utils#null = { '___null___': '' }

function hide#Utils#IsNull(value)
	return type(a:value) == type(g:hide#Utils#null) && a:value == g:hide#Utils#null
endf
