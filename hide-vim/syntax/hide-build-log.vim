if exists("b:current_syntax")
	finish
endif

syntax match buildWarning /\[Warning\]/
syntax match buildError /\[Error\]/
syntax match buildSucceeded /BUILD SUCCEEDED$/
syntax match buildFailed /^BUILD FAILED$/

highlight link buildWarning StatusLine
highlight link buildError Error
highlight link buildSucceeded Type
highlight link buildFailed Error

let b:current_syntax = "hide-build-log"
