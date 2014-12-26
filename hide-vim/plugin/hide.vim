let s:plugin_path = escape(expand('<sfile>:p:h'), '\')

exe 'pyfile '.fnameescape(s:plugin_path).'/hide-vim.py'

"=================================================================

let s:hideBufs = { }

let s:logBufInfo = { 'id': 'log', 'displayName': 'HIDE log', 'filetype': 'hide-log', 'modelName': 'logModel' }
let s:buildLogBufInfo = { 'id': 'buildLog', 'displayName': 'HIDE build log', 'filetype': 'hide-build-log', 'modelName': 'buildLogModel' }

"=================================================================

function s:SyncEverything()
	for bufId in keys(s:hideBufs)
		call s:hideBufs[bufId].Sync()
	endfor
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
			botright 10split
		else
			exec windowNumber.'wincmd w'
		end

		execute 'keepjumps buffer '.buf.bufNum
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

function s:DoBuild(methodCall)
	exec 'python vim.command("let l:res = " + ("1" if hidePlugin.'.a:methodCall.' else "0"))'
	if !res
		throw s:BuildSystemException('Another build already in progress!')
	end
	call s:SyncEverything()
	call s:OpenHideWindow(s:buildLogBufInfo, 0)
endf

function s:GetBuildTargets(A, L, P)
	python vim.command('let l:res = join(' + str(list(hidePlugin.GetBuildTargets())) + ', "\n")')
	call s:SyncEverything()
	return res
endf

function s:BuildInProgress()
	exec 'python vim.command("return " + ("1" if hidePlugin.BuildInProgress() else "0"))'
endf

autocmd CursorHold * call <SID>TimerTick()
autocmd CursorHoldI * call <SID>TimerTickI()

python import vim
python import string
python hidePlugin = HidePlugin()

au VimLeavePre * python del hidePlugin
au BufWritePre * if <SID>BuildInProgress() | throw s:BuildSystemException('Save prevented due to build in progress!') | end

call s:SyncEverything()

command! -nargs=0 HideLog call <SID>OpenHideWindow(s:logBufInfo, 0)
command! -nargs=0 HideBuildLog call <SID>OpenHideWindow(s:buildLogBufInfo, 0)
command! -nargs=0 HideBuildAll call <SID>DoBuild('BuildAll()')
command! -nargs=? -complete=custom,<SID>GetBuildTargets HideBuild call <SID>DoBuild('BuildTarget("<args>")')
command! -nargs=? -complete=file HideBuildFile call <SID>DoBuild('BuildFile("<args>")')


au CursorMoved,CursorMovedI,CmdWinEnter,CmdWinLeave * call <SID>SyncEverything()
