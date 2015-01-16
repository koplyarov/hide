if !exists('s:BufferHighlighterPrototype')
	let s:BufferHighlighterPrototype = {}

	function s:BufferHighlighterPrototype._Init()
		let self._filename = expand('%')
		python hidePlugin.CreateSyntaxHighlighter(vim.eval('self._filename'))
		let self._autocmdGroup = 'HideBufferHighlighter_'.bufnr('')
		exec 'augroup '.self._autocmdGroup
		exec 'au '.self._autocmdGroup.' BufWinEnter <buffer> call b:hideBufferHighlighter._BufWinEnterHandler()'
		exec 'au '.self._autocmdGroup.' BufWinLeave <buffer> if exists("b:hideBufferHighlighter") | call b:hideBufferHighlighter._BufWinLeaveHandler() | end'
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

		for w in keys(self._words)
			exec 'syn keyword HideHighlight'.self._words[w].' '.w
		endfor
	endf

	function s:BufferHighlighterPrototype._ResetHighlightsInCurWindow()
		silent! syntax clear HideHighlightNamedConstant HideHighlightVariable HideHighlightFunction HideHighlightType HideHighlightKeyword
	endf

	function s:BufferHighlighterPrototype.Sync()
		python vim.command('let l:words = ' + hidePlugin.GetSyntaxHighlighter(vim.eval('self._filename')).GetWordsIfModifiedAsVimObj())

		if hide#Utils#IsNull(words)
			return
		end

		let self._words = words
		call self._SetHighlightsInCurWindow()
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
	let b:hideBufferHighlighter = s:BufferHighlighter()
endf

function hide#SyntaxHighlight#DisableBufferSyntaxHighlighting()
	if exists('b:hideBufferHighlighter')
		call b:hideBufferHighlighter.Deinit()
		unlet b:hideBufferHighlighter
	end
endf
