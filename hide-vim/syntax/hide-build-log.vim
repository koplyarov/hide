if exists("b:current_syntax")
	finish
endif

syntax match buildStarted /^Building ".*":$/
syntax match buildWarning /^WARNING:\ze /
syntax match buildError /^ERROR:\ze /
syntax match buildSucceeded /^BUILD SUCCEEDED$/
syntax match buildFailed /^BUILD FAILED$/
syntax match buildInterrupted /^BUILD INTERRUPTED$/

highlight link buildStarted Include
highlight link buildWarning StatusLine
highlight link buildError Error
highlight link buildSucceeded Type
highlight link buildFailed Error
highlight link buildInterrupted StatusLine

let bg_color = synIDattr(hlID('FoldColumn'), 'bg#')
silent! exec 'highlight focusedLine guibg='.bg_color
silent! exec 'highlight focusedLine ctermbg='.bg_color

let b:current_syntax = "hide-build-log"
