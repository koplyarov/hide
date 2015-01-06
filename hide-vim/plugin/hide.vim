let s:plugin_path = escape(expand('<sfile>:p:h'), '\')

"=================================================================

let s:hideBufs = { }

let s:logBufInfo = { 'id': 'log', 'displayName': 'HIDE log', 'filetype': 'hide-log', 'modelName': 'logModel', 'autoScroll': 1 }

let s:buildLogBufInfo = { 'id': 'buildLog', 'displayName': 'HIDE build log', 'filetype': 'hide-build-log', 'modelName': 'buildLogModel', 'autoScroll': 1 }
function s:buildLogBufInfo.Action(idx)
	exec 'python vim.command("let l:location = " + hidePlugin.buildLogModel.GetRow('.a:idx.').GetLocationAsVimDictionary())'
	call s:GotoLocation(location)
	return !empty(location)
endf

let s:indexQueryBufInfo = { 'id': 'indexQuery', 'displayName': 'HIDE index query', 'filetype': 'hide-index-query', 'modelName': 'indexQueryModel', 'autoScroll': 0 }


"=================================================================

function s:GotoLocation(location)
	if empty(a:location)
		return
	end

	let max_winnr = winnr('$')
	for w in [ winnr() ] + range(max_winnr, 1, -1)
		exec w.'wincmd w'
		if !exists('w:isHideWindow') && !exists('b:hideBuffer')
			execute 'e '.a:location.filename
			call setpos('.', [ bufnr(''), a:location.line, a:location.column, 0 ])
			normal zz
			break
		end
	endfor
endf

function s:SyncEverything()
	for bufId in keys(s:hideBufs)
		call s:hideBufs[bufId].Sync()
	endfor
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

function s:TimerTick()
	call s:SyncEverything()
	call feedkeys("f\e")
endf

function s:TimerTickI()
	call s:SyncEverything()
	let noop_keys = (col('$') > 1) ? (col('.') == 1 ? "\<Right>\<Left>" : "\<Left>\<Right>") : "\ei"
	call feedkeys(noop_keys, 'n')
endf

function s:BuildSystemException(msg)
	return "HIDE.BuildSystemException: ".a:msg
endf

function s:IndexQueryException(msg)
	return "HIDE.IndexQueryException: ".a:msg
endf

function s:DoBuild(methodCall)
	exec 'python vim.command("let l:res = " + ("1" if hidePlugin.'.a:methodCall.' else "0"))'
	if !res
		throw s:BuildSystemException('Another build already in progress!')
	end
	call s:SyncEverything()
	call s:OpenHideWindow(s:buildLogBufInfo, 0)
endf

function s:StopBuild()
	python hidePlugin.InterruptBuild()
endf

function s:GetBuildTargets(A, L, P)
	python vim.command('let l:res = join(' + str(list(hidePlugin.GetBuildTargets())) + ', "\n")')
	call s:SyncEverything()
	return res
endf

function s:BuildInProgress()
	exec 'python vim.command("return " + ("1" if hidePlugin.BuildInProgress() else "0"))'
endf

function s:DoStartQueryIndex(methodCall)
	exec 'python vim.command("let l:res = " + ("1" if hidePlugin.'.a:methodCall.' else "0"))'
	if !res
		throw s:IndexQueryException('Another index query already in progress!')
	end
	call s:SyncEverything()
	call s:OpenHideWindow(s:indexQueryBufInfo, 0)
endf

function s:SetLogLevel(logLevel)
	execute 'python hide.Logger.SetLogLevel(hide.LogLevel.'.a:logLevel.')'
endf

function s:GetLogLevels(A, L, P)
	return join([ 'Debug', 'Info', 'Warning', 'Error' ], "\n")
endf

autocmd CursorHold * call <SID>TimerTick()
autocmd CursorHoldI * call <SID>TimerTickI()

python import sys
exe 'python sys.path.insert(0, "'.fnameescape(s:plugin_path).'")'

python << endpython

import vim
import string

import hide_vim
import hide

hidePlugin = hide_vim.HidePlugin()

endpython

au VimLeavePre * python del hidePlugin
au BufWritePre * if <SID>BuildInProgress() | throw s:BuildSystemException('Save prevented due to build in progress!') | end

call s:SyncEverything()

command! -nargs=1 -complete=custom,<SID>GetLogLevels HideLogLevel call <SID>SetLogLevel('<args>')
command! -nargs=0 HideLog call <SID>OpenHideWindow(s:logBufInfo, 0)
command! -nargs=0 HideBuildLog call <SID>OpenHideWindow(s:buildLogBufInfo, 0)
command! -nargs=0 HideBuildAll call <SID>DoBuild('BuildAll()')
command! -nargs=0 HideStopBuild call <SID>StopBuild()
command! -nargs=? -complete=custom,<SID>GetBuildTargets HideBuild call <SID>DoBuild('BuildTarget("<args>")')
command! -nargs=? -complete=file HideBuildFile call <SID>DoBuild('BuildFile("<args>")')
command! -nargs=1 HideIndexQuery call <SID>DoStartQueryIndex('QuerySymbolsBySubstring("<args>")')


au CursorMoved,CursorMovedI,CmdWinEnter,CmdWinLeave * call <SID>SyncEverything()
