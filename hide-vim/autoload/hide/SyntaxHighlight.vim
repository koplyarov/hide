if !exists('s:BufferHighlighterPrototype')
	let s:BufferHighlighterPrototype = {}

	function s:BufferHighlighterPrototype._Init()
		let self._filename = expand('%')
		python hidePlugin.CreateSyntaxHighlighter(vim.eval('self._filename'))
		let self._autocmdGroup = 'HideBufferHighlighter_'.bufnr('')
		exec 'augroup '.self._autocmdGroup
		exec 'au '.self._autocmdGroup.' BufWinEnter <buffer> call b:hideBufferHighlighter.Sync(0)'
		exec 'au '.self._autocmdGroup.' BufReadPost <buffer> call b:hideBufferHighlighter.Sync(1)'
		call self._LinkHighlights()
		call self.Sync(1)
	endf

	function s:BufferHighlighterPrototype.Deinit()
		exec 'au! '.self._autocmdGroup
		exec 'augroup! '.self._autocmdGroup
		call self._ResetHighlightsInCurWindow()
		call self._UnlinkHighlights()
		python hidePlugin.DeleteSyntaxHighlighter(vim.eval('self._filename'))
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

	function s:BufferHighlighterPrototype._ResetHighlightsInCurWindow()
		silent! syntax clear HideHighlightNamedConstant HideHighlightVariable HideHighlightFunction HideHighlightType HideHighlightKeyword
	endf

	function s:BufferHighlighterPrototype.Sync(forceFullUpdate)
		python hidePlugin.GetSyntaxHighlighter(vim.eval('self._filename')).UpdateHighlights(int(vim.eval('a:forceFullUpdate')))
	endf

	function s:BufferHighlighterPrototype.Sync_static()
		if exists('b:hideBufferHighlighter')
			call b:hideBufferHighlighter.Sync(0)
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
