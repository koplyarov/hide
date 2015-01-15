function s:BufferHighlighter()
	if !exists('s:BufferHighlighterPrototype')
		let s:BufferHighlighterPrototype = {}

		function s:BufferHighlighterPrototype._Init()
			let self._autocmdGroup = 'HideBufferHighlighter_'.bufnr('')
			exec 'augroup '.self._autocmdGroup
			exec 'au '.self._autocmdGroup.' BufWinEnter <buffer> call b:hideBufferHighlighter._SetHighlightsInCurWindow()'
			exec 'au '.self._autocmdGroup.' BufWinLeave <buffer> if exists("b:hideBufferHighlighter") | call b:hideBufferHighlighter._ResetHighlightsInCurWindow() | end'
			call self._SetHighlightsInCurWindow()
		endf

		function s:BufferHighlighterPrototype.Deinit()
			exec 'au! '.self._autocmdGroup
			exec 'augroup! '.self._autocmdGroup
			call self._ResetHighlightsInCurWindow()
		endf

		function s:BufferHighlighterPrototype._SetHighlightsInCurWindow()
			call self._ResetHighlightsInCurWindow()

			for s in self._symbols
				exec 'syn keyword HideHighlight'.s.category.' '.s.word
				call hide#Utils#Log('Info', 'symbol: '.string(s))
			endfor

			highlight link HideHighlightType Type
			highlight link HideHighlightFunction Function
		endf

		function s:BufferHighlighterPrototype._ResetHighlightsInCurWindow()
			silent! syntax clear HideHighlightType HideHighlightFunction
		endf
	end

	let self = deepcopy(s:BufferHighlighterPrototype)

	let self._symbols = [ { 'word': 'NinjaCMakeBackend', 'category': 'Type' }, { 'word': 'GetTargets', 'category': 'Function' } ]
	call self._Init()

	return self
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
