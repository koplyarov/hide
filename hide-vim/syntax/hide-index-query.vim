if exists("b:current_syntax")
	finish
endif

syntax match searching /^Searching\.\.\.$/
syntax match noSymbolsFound /^No symbols found\.$/
syntax match symbolName /^\(\S\)\+\ze (.*)$/

highlight link searching Include
highlight link noSymbolsFound Include
highlight link symbolName Type

let bg_color = synIDattr(hlID('FoldColumn'), 'bg#')
exec 'highlight focusedLine ctermbg='.bg_color.' guibg='.bg_color

let b:current_syntax = "hide-index-query"
