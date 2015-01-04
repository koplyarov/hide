function hide#Logger#Log(logLevel, text)
	execute 'python hidePlugin.logger.Log(hide.LogLevel.'.a:logLevel.', "'.string(a:text).'")'
endf
