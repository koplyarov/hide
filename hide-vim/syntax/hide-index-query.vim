if exists("b:current_syntax")
	finish
endif

syntax match searching /^Searching\.\.\.$/
syntax match noSymbolsFound /^No symbols found\.$/
syntax match symbolName /^\(\S\)\+\ze (.*)$/

highlight link searching Include
highlight link noSymbolsFound Include
highlight link symbolName Type

highlight focusedLine ctermbg=darkred guibg=darkred

let b:current_syntax = "hide-index-query"
