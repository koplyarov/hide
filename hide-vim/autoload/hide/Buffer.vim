function s:BufferViewException(msg)
	return "HIDE.BufferViewException: ".a:msg
endf

"bufInfo: { 'id': <string>, 'displayName': <string>, 'filetype': <string> }
function hide#Buffer#Buffer(bufInfo)
	if !exists('s:BufferPrototype')
		let s:BufferPrototype = {}

		function s:BufferPrototype._Init()
			let prevBuf = bufnr('')
			execute 'keepjumps buffer '.self.bufNum
			try
				setlocal ma
				setlocal noswf
				setlocal ro
				exec 'setlocal ft='.self.filetype
				setlocal nocul
				let b:hideBuffer = self
				nmap <buffer> <CR> :call b:hideBuffer._EnterPressed()<CR>
				nmap <buffer> <2-LeftMouse> :call b:hideBuffer._EnterPressed()<CR>
				au BufWinEnter <buffer> call b:hideBuffer._SetHighlights()
				au BufWinLeave <buffer> if exists('b:hideBuffer') | call b:hideBuffer._ResetHighlights() | end
			finally
				execute 'keepjumps buffer '.prevBuf
			endtry
		endf

		function s:BufferPrototype._SetHighlights()
			call self._ResetHighlights()
			if !empty(self._focusedLine)
				let self._focusedLineMatchId = matchadd('focusedLine', '\%'.(self._focusedLine + 1).'l')
			end
		endf

		function s:BufferPrototype._ResetHighlights()
			if has_key(self, '_focusedLineMatchId')
				call matchdelete(self._focusedLineMatchId)
				unlet self._focusedLineMatchId
			end
		endf

		function s:BufferPrototype._UpdateHighlights()
			call self._ForEachMyWindow(self._SetHighlights)
		endf

		function s:BufferPrototype._FocusLine(lineNum)
			let self._focusedLine = a:lineNum
			call self._UpdateHighlights()
		endf

		function s:BufferPrototype._EnterPressed()
			let idx = line('.') - 1
			let focus_cur_line = (has_key(self, 'Action')) && self.Action(idx)
			call self._FocusLine(focus_cur_line ? idx : '')
		endf

		function s:BufferPrototype.Sync()
			exec 'python vim.command("let l:events = [" + str.join(",", map(lambda e: e.ToVimDictionary(), hidePlugin.'.self.modelName.'.GetEvents())) + "]")'
			for e in events
				if e.type == 'reset'
					let self._lines = []
				elseif e.type == 'inserted'
					for idx in range(e.begin, e.end - 1)
						exec 'python vim.command("let l:row = ''" + hidePlugin.'.self.modelName.'.GetRow('.idx.').ToVimString() + "''")'
						call insert(self._lines, row, idx)
					endfor
				else
					throw s:BufferViewException('Unknown ModelEvent type: "'.e.type.'"')
				end
			endfor

			if !empty(events)
				try
					call self._DoInBuffer(self._UpdateData)
				finally
					call self._ForEachMyWindow(self._ScrollDown)
				endtry
			end
		endf

		function s:BufferPrototype._UpdateData()
			setlocal noro
			try
				let prevLen = len(getline('^', '$'))
				if prevLen == 1 && empty(getline(1))
					if !empty(self._lines)
						call setline(1, self._lines[0])
					end
				elseif prevLen > len(self._lines)
					keepjumps normal ggdG
					let prevLen = 1
					if !empty(self._lines)
						call setline(1, self._lines[0])
					end
				end
				call append('$', self._lines[prevLen : ])
			finally
				setlocal ro
				setlocal nomod
			endtry
		endf

		function s:BufferPrototype._ScrollDown()
			keepjumps normal G
		endf

		function s:BufferPrototype._DoInBuffer(func)
			let oldEventignore = &eventignore
			let curWinView = winsaveview()
			let prevBuf = bufnr('')
			try
				set eventignore=all
				execute 'keepjumps buffer '.self.bufNum
				call call(a:func, [], self)
			finally
				let &ei = oldEventignore
				execute 'keepjumps buffer '.prevBuf
				call winrestview(curWinView)
			endtry
		endf

		function s:BufferPrototype._ForEachMyWindow(func)
			let oldEventignore = &eventignore
			let currentTab = tabpagenr()
			try
				for i in range(1, tabpagenr('$'))
					exec 'tabnext '.i
					let tabWin = winnr()
					try
						for j in range(1, winnr('$'))
							exec j.'wincmd w'
							if bufnr('') == self.bufNum
								call call(a:func, [], self)
							end
						endfor
					finally
						exec tabWin.'wincmd w'
					endtry
				endfor
			finally
				exec 'tabnext '.currentTab
				let &ei = oldEventignore
			endtry
		endf
	end

	let self = copy(s:BufferPrototype)

	let self.bufNum = bufnr(a:bufInfo.displayName, 1)
	let self._lines = [ ]
	let self._focusedLine = ''
	call extend(self, a:bufInfo)
	call self._Init()

	return self
endf
