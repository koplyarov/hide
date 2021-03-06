if !exists('s:BufferHighlighterPrototype')
	let s:BufferHighlighterPrototype = {}

	function s:BufferHighlighterPrototype._Init()
		let self._filename = expand('%')
		call hide#Utils#Python("hidePlugin.CreateSyntaxHighlighter('".self._filename."')")
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
		call hide#Utils#Python("hidePlugin.DeleteSyntaxHighlighter('".self._filename."')")
	endf

	function s:BufferHighlighterPrototype._LinkHighlights()
		highlight link HideHighlightUnknown Variable
		highlight link HideHighlightConstant Constant
		highlight link HideHighlightNamedConstant Variable
		highlight link HideHighlightVariable Variable
		highlight link HideHighlightMacro Macro
		highlight link HideHighlightFunction Function
		highlight link HideHighlightType Type
		highlight link HideHighlightNamespace Type
		highlight link HideHighlightStatement Statement
		highlight link HideHighlightConditional Conditional
		highlight link HideHighlightRepeat Repeat
		highlight link HideHighlightLabel Label
		highlight link HideHighlightOperator Operator
		highlight link HideHighlightException Exception
		highlight link HideHighlightKeyword Keyword
		highlight link HideHighlightImport Import
	endf

	function s:BufferHighlighterPrototype._UnlinkHighlights()
		highlight link HideHighlightUnknown NONE
		highlight link HideHighlightConstant NONE
		highlight link HideHighlightNamedConstant NONE
		highlight link HideHighlightVariable NONE
		highlight link HideHighlightMacro NONE
		highlight link HideHighlightFunction NONE
		highlight link HideHighlightType NONE
		highlight link HideHighlightNamespace NONE
		highlight link HideHighlightStatement NONE
		highlight link HideHighlightConditional NONE
		highlight link HideHighlightRepeat NONE
		highlight link HideHighlightLabel NONE
		highlight link HideHighlightOperator NONE
		highlight link HideHighlightException NONE
		highlight link HideHighlightKeyword NONE
		highlight link HideHighlightImport NONE
	endf

	function s:BufferHighlighterPrototype._ResetHighlightsInCurWindow()
		silent! syntax clear HideHighlightNamedConstant HideHighlightVariable HideHighlightFunction HideHighlightType HideHighlightKeyword
	endf

	function s:BufferHighlighterPrototype.Sync(forceFullUpdate)
		call hide#Utils#Python("hidePlugin.GetSyntaxHighlighter('".self._filename."').UpdateHighlights(".a:forceFullUpdate.")")
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
