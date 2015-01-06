function hide#Utils#Log(logLevel, text)
	execute 'python hidePlugin.logger.Log(hide.LogLevel.'.a:logLevel.', "'.string(a:text).'")'
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
			execute 'e '.a:location.filename
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
