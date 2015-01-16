if !exists('s:BufferHighlighterPrototype')
	let s:BufferHighlighterPrototype = {}

	function s:BufferHighlighterPrototype._Init()
		let self._filename = expand('%')
		python hidePlugin.CreateSyntaxHighlighter(vim.eval('self._filename'))
		let self._autocmdGroup = 'HideBufferHighlighter_'.bufnr('')
		exec 'augroup '.self._autocmdGroup
		"exec 'au '.self._autocmdGroup.' BufWinEnter <buffer> call b:hideBufferHighlighter._BufWinEnterHandler()'
		"exec 'au '.self._autocmdGroup.' BufWinLeave <buffer> if exists("b:hideBufferHighlighter") | call b:hideBufferHighlighter._BufWinLeaveHandler() | end'
		call self.Sync()
		call self._BufWinEnterHandler()
	endf

	function s:BufferHighlighterPrototype.Deinit()
		exec 'au! '.self._autocmdGroup
		exec 'augroup! '.self._autocmdGroup
		call self._ResetHighlightsInCurWindow()
		python hidePlugin.DeleteSyntaxHighlighter(vim.eval('self._filename'))
	endf

	function s:BufferHighlighterPrototype._BufWinEnterHandler()
		call self.Sync()
		call self._SetHighlightsInCurWindow()
		call self._LinkHighlights()
	endf

	function s:BufferHighlighterPrototype._BufWinLeaveHandler()
		call self._UnlinkHighlights()
		call self._ResetHighlightsInCurWindow()
	endf

	function s:BufferHighlighterPrototype._LinkHighlights()
		highlight link HideHighlightNamedConstant Variable
		highlight link HideHighlightVariable Variable
		highlight link HideHighlightFunction Function
		highlight link HideHighlightType DefinedName
		highlight link HideHighlightKeyword Type
	endf

	function s:BufferHighlighterPrototype._UnlinkHighlights()
		highlight link HideHighlightNamedConstant NONE
		highlight link HideHighlightVariable NONE
		highlight link HideHighlightFunction NONE
		highlight link HideHighlightType NONE
		highlight link HideHighlightKeyword NONE
	endf

	function s:BufferHighlighterPrototype._SetHighlightsInCurWindow()
		call self._ResetHighlightsInCurWindow()

		call self._AddHighlights(self._words)
	endf

	function s:BufferHighlighterPrototype._AddHighlights(words)
		let banned_words = ['contains', 'oneline', 'fold', 'display', 'extend concealends']

		let categories = {}
		for w in keys(a:words)
			let cat = a:words[w]
			if !has_key(categories, cat)
				let categories[cat] = [ w ]
			else
				call add(categories[cat], w)
			end
		endfor

		for c in keys(categories)
			let syntax_cmd = 'syn keyword HideHighlight'.c
			let category_words = categories[c]
			for w in category_words
				if index(banned_words, tolower(w)) != -1
					if tolower(w) != 'concealends'
						exec 'syn match '.c.' /\<'.w.'\>/'
					end
					continue
				end

				if len(syntax_cmd) + len(' '.w) >= 512
					try
						exec syntax_cmd
					catch
						call hide#Utils#Log('BufferHighlighter', 'Error', 'exec failed ('.v:exception.'): '.syntax_cmd)
					endtry
					let syntax_cmd = 'syn keyword HideHighlight'.c
				end
				let syntax_cmd .= ' '.w
			endfor
			try
				exec syntax_cmd
			catch
				call hide#Utils#Log('BufferHighlighter', 'Error', 'exec failed ('.v:exception.'): '.syntax_cmd)
			endtry
		endfor
	endf

	function s:BufferHighlighterPrototype._ResetHighlightsInCurWindow()
		silent! syntax clear HideHighlightNamedConstant HideHighlightVariable HideHighlightFunction HideHighlightType HideHighlightKeyword
	endf

	function s:BufferHighlighterPrototype.Sync()
		python vim.command('let l:words = ' + hidePlugin.GetSyntaxHighlighter(vim.eval('self._filename')).GetChangedWordsAsVimDictionary())

		let added_words = filter(copy(words), 'words[v:key] != "NoneCategory"')
		let removed_words = filter(copy(words), 'words[v:key] == "NoneCategory"')

		let reset_highlights = !empty(removed_words)

		for w in keys(removed_words)
			if has_key(self._words, w)
				unlet self._words[w]
			end
		endfor

		for w in keys(added_words)
			let self._words[w] = added_words[w]
		endfor

		if !reset_highlights
			call self._AddHighlights(added_words)
		end

		if reset_highlights
			call self._SetHighlightsInCurWindow()
		end
	endf

	function s:BufferHighlighterPrototype.Sync_static()
		if exists('b:hideBufferHighlighter')
			call b:hideBufferHighlighter.Sync()
		end
	endf
end


function s:BufferHighlighter()

	let self = deepcopy(s:BufferHighlighterPrototype)

	let self._words = { }
	call self._Init()

	return self
endf


function hide#SyntaxHighlight#SyncBufferHighlighters()
	call hide#Utils#ForEachWindow(s:BufferHighlighterPrototype.Sync_static, [], {})
endf

function hide#SyntaxHighlight#EnableBufferSyntaxHighlighting()
	if !exists('b:hideBufferHighlighter')
		let b:hideBufferHighlighter = s:BufferHighlighter()
	end
endf

function hide#SyntaxHighlight#DisableBufferSyntaxHighlighting()
	if exists('b:hideBufferHighlighter')
		call b:hideBufferHighlighter.Deinit()
		unlet b:hideBufferHighlighter
	end
endf
