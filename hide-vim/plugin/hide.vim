let s:plugin_path = escape(expand('<sfile>:p:h'), '\')

"=================================================================

function s:LocalFunc(funcName)
	let sid = matchstr(expand('<sfile>'), '<SNR>\zs\d\+\ze_LocalFunc$')
	return function('<SNR>'.sid.'_'.a:funcName)
endf

function s:GotoLocationAction(idx) dict
	let location = hide#Utils#Python('vim.command("return " + hidePlugin.'.self.modelName.'.GetRow('.a:idx.').GetLocationAsVimDictionary())')
	call hide#Utils#GotoLocation(location)
	return !empty(location)
endf

"=================================================================

let s:hideBufs = { }

let s:logBufInfo = { 'id': 'log', 'displayName': 'HIDE log', 'filetype': 'hide-log', 'modelName': 'logModel', 'autoScroll': 1 }

let s:buildLogBufInfo = { 'id': 'buildLog', 'displayName': 'HIDE build log', 'filetype': 'hide-build-log', 'modelName': 'buildLogModel', 'Action': s:LocalFunc('GotoLocationAction'), 'autoScroll': 1 }

let s:indexQueryBufInfo = { 'id': 'indexQuery', 'displayName': 'HIDE index query', 'filetype': 'hide-index-query', 'modelName': 'indexQueryModel', 'Action': s:LocalFunc('GotoLocationAction'), 'autoScroll': 0 }


"=================================================================

function s:SyncEverything()
	call hide#SyntaxHighlight#SyncBufferHighlighters()
	for bufId in keys(s:hideBufs)
		call s:hideBufs[bufId].Sync()
	endfor
endf

function s:SyncEverythingFrequent()
	let now = localtime()
	if mode() != 'V' && mode() != 'v' && mode() != "\<C-V>" && (!exists('s:lastSyncTime') || s:lastSyncTime != now)
		try
			call s:SyncEverything()
		finally
			let s:lastSyncTime = now
		endtry
	end
endf

function s:CreateHideWindow()
	let curWin = winnr()
	try
		botright 10split
		let w:isHideWindow = 1
		return winnr()
	finally
		exec curWin.'wincmd w'
	endtry
endf

function s:FindHideWindow()
	let curWin = winnr()
	try
		let max_winnr = winnr('$')
		for w in range(max_winnr, 1, -1)
			exec w.'wincmd w'
			if exists('w:isHideWindow')
				return w
			end
		endfor
	finally
		exec curWin.'wincmd w'
	endtry
	return -1
endf

function s:OpenHideWindow(bufInfo, focus)
	if !has_key(s:hideBufs, a:bufInfo.id)
		let s:hideBufs[a:bufInfo.id] = hide#Buffer#Buffer(a:bufInfo)
	end

	let curWin = winnr()
	try
		let buf = s:hideBufs[a:bufInfo.id]
		let windowNumber = bufwinnr(buf.bufNum)
		if windowNumber == -1
			let windowNumber = s:FindHideWindow()
		end
		if windowNumber == -1
			let windowNumber = s:CreateHideWindow()
		end

		exec windowNumber.'wincmd w'
		exec 'keepjumps buffer '.buf.bufNum

		call buf.Sync()
	finally
		if !a:focus
			exec curWin.'wincmd w'
		end
	endtry
endf

function s:BuildSystemException(msg)
	return "HIDE.BuildSystemException: ".a:msg
endf

function s:IndexQueryException(msg)
	return "HIDE.IndexQueryException: ".a:msg
endf

function s:DoBuild(methodCall)
	let res = hide#Utils#Python('vim.command("return " + ("1" if hidePlugin.'.a:methodCall.' else "0"))')
	if !res
		throw s:BuildSystemException('Another build already in progress!')
	end
	call s:SyncEverything()
	call s:OpenHideWindow(s:buildLogBufInfo, 0)
endf

function s:StopBuild()
	call hide#Utils#Python('hidePlugin.InterruptBuild()')
endf

function s:GetBuildTargets(A, L, P)
	let res = hide#Utils#Python("vim.command('return join(' + str(list(hidePlugin.GetBuildTargets())) + ', \"\n\")')")
	call s:SyncEverything()
	return res
endf

function s:BuildInProgress()
	return hide#Utils#Python('vim.command("return " + ("1" if hidePlugin.BuildInProgress() else "0"))')
endf

function s:DoStartQueryIndex(methodCall)
	let res = hide#Utils#Python('vim.command("return " + ("1" if hidePlugin.'.a:methodCall.' else "0"))')
	if !res
		throw s:IndexQueryException('Another index query already in progress!')
	end
	call s:SyncEverything()
	let finished = hide#Utils#Python('vim.command("return " + ("0" if hidePlugin.IndexQueryInProgress() else "1"))')
	let singleMatch = hide#Utils#Python('vim.command("return " + ("1" if hidePlugin.IndexQueryHasSingleMatch() else "0"))')
	if finished && singleMatch
		call s:indexQueryBufInfo.Action(0)
	else
		call s:OpenHideWindow(s:indexQueryBufInfo, 1)
	end
endf

function s:SetLogLevel(logLevel)
	call hide#Utils#Python('hide.Logger.SetLogLevel(hide.LogLevel.'.a:logLevel.')')
endf

function s:GetLogLevels(A, L, P)
	return join([ 'Debug', 'Info', 'Warning', 'Error' ], "\n")
endf

call hide#Utils#Python('import sys')
call hide#Utils#Python('sys.path.insert(0, "'.fnameescape(s:plugin_path).'")')

call hide#Utils#Python('import vim, string, hide_vim, hide')
call hide#Utils#Python('hidePlugin = hide_vim.HidePlugin()')

au VimLeavePre * call hide#Utils#Python('del hidePlugin')
au BufWritePre * if <SID>BuildInProgress() | throw s:BuildSystemException('Save prevented due to build in progress!') | end

call s:SyncEverything()
call hide#Utils#AddTimer(s:LocalFunc('SyncEverything'))

command! -nargs=1 -complete=custom,<SID>GetLogLevels HideLogLevel call <SID>SetLogLevel('<args>')
command! -nargs=0 HideLog call <SID>OpenHideWindow(s:logBufInfo, 0)
command! -nargs=0 HideBuildLog call <SID>OpenHideWindow(s:buildLogBufInfo, 0)
command! -nargs=0 HideBuildAll call <SID>DoBuild('BuildAll()')
command! -nargs=0 HideStopBuild call <SID>StopBuild()
command! -nargs=? -complete=custom,<SID>GetBuildTargets HideBuild call <SID>DoBuild('BuildTarget("<args>")')
command! -nargs=? -complete=file HideBuildFile call <SID>DoBuild('BuildFile("<args>")')
command! -nargs=1 HideIndexQuery call <SID>DoStartQueryIndex('QuerySymbolsByName("<args>")')


au CursorMoved,CmdWinEnter,CmdWinLeave * call <SID>SyncEverythingFrequent()

au BufNew,BufRead *.h,*.hpp,*.c,*.cpp call hide#SyntaxHighlight#EnableBufferSyntaxHighlighting()
