let s:plugin_path = escape(expand('<sfile>:p:h'), '\')

exe 'pyfile '.fnameescape(s:plugin_path).'/hide-vim.py'

let s:log = []
let s:buildLog = []
function s:SyncEverything()
	let linesNum = len(s:log)
	python vim.command('let s:log += ' + str(hidePlugin.GetLogLines(string.atoi(vim.eval('len(s:log)')))))
	if len(s:log) != linesNum
		call s:UpdateLogWindow(s:logCategory, s:log)
	end

	let linesNum = len(s:buildLog)
	python vim.command('let s:buildLog += ' + str(hidePlugin.GetBuildLogLines(string.atoi(vim.eval('len(s:buildLog)')))))
	if len(s:buildLog) != linesNum
		call s:UpdateLogWindow(s:buildLogCategory, s:buildLog)
	end
endf

function s:UpdateLogWindow(category, list)
	if !s:HideBufInitialized(a:category)
		return
	end

	let oldEventignore = &eventignore
	let prevBuf = bufnr('')
	try
		set eventignore=all
		execute 'buffer '.s:hideBufs[a:category]
		setlocal noro

		let prevLen = len(getline('^', '$'))
		if prevLen == 1 && empty(getline(1))
			if !empty(a:list)
				call setline(1, a:list[0])
			end
		elseif prevLen > len(a:list)
			normal ggdG
			let prevLen = 1
			if !empty(a:list)
				call setline(1, a:list[0])
			end
		end
		call append('$', a:list[prevLen : ])
	finally
		let currentTab = tabpagenr()
		try
			for i in range(1, tabpagenr('$'))
				exec 'tabnext '.i
				let tabWin = winnr()
				try
					for j in range(0, winnr('$'))
						exec j.'wincmd w'
						if bufnr('') == s:hideBufs[a:category]
							normal G
						end
					endfor
				finally
					exec tabWin.'wincmd w'
				endtry

				if len(GetTabVar(i, 'NERDTreeBufName')) > 0
					return 1
				endif
			endfor
		finally
			exec 'tabnext '.currentTab
		endtry

		let &ei = oldEventignore

		setlocal ro
		setlocal nomod
		execute 'buffer '.prevBuf
	endtry
endf

function s:GetTabVar(tabnr, var)
	let currentTab = tabpagenr()
	let oldEventignore = &eventignore

	try
		set eventignore=all
		exec 'tabnext ' . a:tabnr

		if exists('t:' . a:var)
			exec 'return {"value": t:'.a:var.'}'
		endif
		return {}
	finally
		exec 'tabnext ' . currentTab
		let &ei = oldEventignore
	endtry
endfunction

function s:HideBufInitialized(category)
	return has_key(s:hideBufs, a:category)
endf

function s:InitHideBuf(category, buf)
	if s:HideBufInitialized(a:category)
		return
	end

	let s:hideBufs[a:category] = a:buf

	let prevBuf = bufnr('')
	execute 'buffer '.a:buf
	try
		setlocal ma
		setlocal noswf
		setlocal ro
		setlocal ft=hide-log
		setlocal nocul
	finally
		execute 'buffer '.prevBuf
	endtry
endf

let s:logCategory = 'log'
let s:buildLogCategory = 'build log'
let s:hideBufs = { }
function s:OpenHideWindow(category, focus)
	let buf = bufnr('HIDE '.a:category, 1)
	if !s:HideBufInitialized(a:category)
		call s:InitHideBuf(a:category, buf)
	end

	let windowNumber = bufwinnr(buf)
	if windowNumber == -1
		botright 10split
	else
		exec windowNumber.'wincmd w'
	end

	execute 'buffer '.buf
	" TODO: reimplement
	if a:category == s:logCategory
		call s:UpdateLogWindow(a:category, s:log)
	elseif a:category == s:buildLogCategory
		call s:UpdateLogWindow(a:category, s:buildLog)
	end
endf

function s:TimerTick()
	call s:SyncEverything()
	call feedkeys("f\e")
endf

function s:BuildSystemException(msg)
	return "HIDE.BuildSystemException: ".a:msg
endf

function s:DoBuild(methodCall)
	exec 'python vim.command("let l:res = " + ("1" if hidePlugin.'.a:methodCall.' else "0"))'
	if !res
		throw s:BuildSystemException('Another build already in progress!')
	end
	let s:buildLog = [ ]
	call s:SyncEverything()
	call s:OpenHideWindow(s:buildLogCategory, 0)
endf

function s:GetBuildTargets(A, L, P)
	python vim.command('let l:res = join(' + str(list(hidePlugin.GetBuildTargets())) + ', "\n")')
	call s:SyncEverything()
	return res
endf

autocmd CursorHold,CursorHoldI * call <SID>TimerTick()

python import vim
python import string
python hidePlugin = HidePlugin()

call s:SyncEverything()

command! -nargs=0 HideLog call <SID>OpenHideWindow(s:logCategory, 0)
command! -nargs=0 HideBuildLog call <SID>OpenHideWindow(s:buildLogCategory, 0)
command! -nargs=0 HideBuildAll call <SID>DoBuild('BuildAll()')
command! -nargs=? -complete=custom,<SID>GetBuildTargets HideBuild call <SID>DoBuild('BuildTarget("<args>")')
command! -nargs=? -complete=file HideBuildFile call <SID>DoBuild('BuildFile("<args>")')


au CursorMoved,CursorMovedI,CmdWinEnter,CmdWinLeave * call <SID>SyncEverything()
