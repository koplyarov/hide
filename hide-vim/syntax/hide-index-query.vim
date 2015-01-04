if exists("b:current_syntax")
	finish
endif

syntax match searching /^Searching\.\.\.$/

highlight link searching Include

highlight focusedLine ctermbg=darkred guibg=darkred

let b:current_syntax = "hide-index-query"
