function hide#Utils#Python(code)
	if has('python3')
		execute 'python3 '.a:code
	elseif has('python')
		execute 'python '.a:code
	else
		throw "No python support!"
	end
endf

function hide#Utils#Log(category, logLevel, text)
	let log_message_text = (type(a:text) == type('') ? a:text : string(a:text))
	call hide#Utils#Python('hidePlugin.logger.Log(hide.LogLevel.'.a:logLevel.', "'.log_message_text.'")')
endf

let g:hide#Utils#null = { '___null___': '' }

function hide#Utils#IsNull(value)
	return type(a:value) == type(g:hide#Utils#null) && a:value == g:hide#Utils#null
endf

function hide#Utils#GotoLocation(location)
	if empty(a:location)
		return
	end

	let max_winnr = winnr('$')
	for w in [ winnr() ] + range(max_winnr, 1, -1)
		exec w.'wincmd w'
		if !exists('w:isHideWindow') && !exists('b:hideBuffer')
			let buf_num = bufnr(a:location.filename)
			if buf_num == -1
				execute 'e '.a:location.filename
			else
				execute 'b '.buf_num
			end
			call setpos('.', [ bufnr(''), a:location.line, a:location.column, 0 ])
			normal zz
			break
		end
	endfor
endf


let s:timerHandlers = []

function hide#Utils#AddTimer(handlerFunc)
	call add(s:timerHandlers, a:handlerFunc)
endf

function hide#Utils#RemoveTimer(handlerFunc)
	for i in range(0, len(s:timerHandlers) - 1)
		if s:timerHandlers[i] == a:handlerFunc
			call remove(s:timerHandlers, i)
			return
		end
	endfor
endf

function s:OnTimer()
	for H in s:timerHandlers
		call call(H, [])
	endfor
endf

function s:TimerTick()
	call s:OnTimer()
	call feedkeys("f\e")
endf

function s:TimerTickI()
	call s:OnTimer()
	let noop_keys = (col('$') > 1) ? (col('.') == 1 ? "\<Right>\<Left>" : "\<Left>\<Right>") : "\ei"
	call feedkeys(noop_keys, 'n')
endf

autocmd CursorHold * call <SID>TimerTick()
autocmd CursorHoldI * call <SID>TimerTickI()



function hide#Utils#ForEachWindow(func, args, ...)
	let oldEventignore = &eventignore
	let currentTab = tabpagenr()
	try
		for i in range(1, tabpagenr('$'))
			exec 'tabnext '.i
			let tabWin = winnr()
			try
				exec 'wincmd p'
				let tabWinPrev = winnr()
				try
					let windows = range(1, winnr('$'))
					for j in windows
						exec j.'wincmd w'
						if a:0 == 1
							call call(a:func, a:args, a:1)
						else
							call call(a:func, a:args)
						end
					endfor
				finally
					exec ''.tabWinPrev.'wincmd w'
				endtry
			finally
				exec ''.tabWin.'wincmd w'
			endtry
		endfor
	finally
		exec 'tabnext '.currentTab
		let &ei = oldEventignore
	endtry
endf
