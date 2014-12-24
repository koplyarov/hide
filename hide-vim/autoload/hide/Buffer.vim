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
			finally
				execute 'keepjumps buffer '.prevBuf
			endtry
		endf

		function s:BufferPrototype.Sync()
			let prevLinesNum = len(self.lines)
			call self.UpdateLines()
			if len(self.lines) != prevLinesNum
				call self.Update()
			end
		endf

		function s:BufferPrototype._UpdateData()
			let prevLen = len(getline('^', '$'))
			if prevLen == 1 && empty(getline(1))
				if !empty(self.lines)
					call setline(1, self.lines[0])
				end
			elseif prevLen > len(self.lines)
				keepjumps normal ggdG
				let prevLen = 1
				if !empty(self.lines)
					call setline(1, self.lines[0])
				end
			end
			call append('$', self.lines[prevLen : ])
		endf

		function s:BufferPrototype.Update()
			let oldEventignore = &eventignore
			let curWinView = winsaveview()
			let prevBuf = bufnr('')
			try
				set eventignore=all
				execute 'keepjumps buffer '.self.bufNum
				setlocal noro

				call self._UpdateData()
			finally
				let currentTab = tabpagenr()
				try
					for i in range(1, tabpagenr('$'))
						exec 'tabnext '.i
						let tabWin = winnr()
						try
							for j in range(0, winnr('$'))
								exec j.'wincmd w'
								if bufnr('') == self.bufNum
									keepjumps normal G
								end
							endfor
						finally
							exec tabWin.'wincmd w'
						endtry
					endfor
				finally
					exec 'tabnext '.currentTab
				endtry

				let &ei = oldEventignore

				setlocal ro
				setlocal nomod
				execute 'keepjumps buffer '.prevBuf
				call winrestview(curWinView)
			endtry
		endf
	end

	let self = copy(s:BufferPrototype)

	let self.bufNum = bufnr(a:bufInfo.displayName, 1)
	let self.lines = [ ]
	call extend(self, a:bufInfo)
	call self._Init()

	return self
endf
