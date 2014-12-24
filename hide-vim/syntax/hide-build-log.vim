if exists("b:current_syntax")
	finish
endif

syntax match buildStarted /^Building ".*":$/
syntax match buildWarning /^WARNING:\ze /
syntax match buildError /^ERROR:\ze /
syntax match buildSucceeded /^BUILD SUCCEEDED$/
syntax match buildFailed /^BUILD FAILED$/

highlight link buildStarted Include
highlight link buildWarning StatusLine
highlight link buildError Error
highlight link buildSucceeded Type
highlight link buildFailed Error

let b:current_syntax = "hide-build-log"
