if exists("b:current_syntax")
	finish
endif

syntax match logTime /\[\d\{2}\/\d\{2}\/\d\{4} \d\{2}:\d\{2}:\d\{2}.\d\{3}\]/
syntax match logLevelDebug /\[Debug\]/
syntax match logLevelInfo /\[Info\]/
syntax match logLevelWarning /\[Warning\]/
syntax match logLevelError /\[Error\]/
syntax region logText matchgroup=logThread start=/\({[_a-zA-Z0-9()]*}\)/ end=/$/ contains=logClass
syntax match logClass /\[\(\(Debug\>\)\@!\&\(Info\>\)\@!\&\(Warning\>\)\@!\&\(Error\>\)\@!\&\([A-Za-z_][A-Za-z_0-9:]*\)\)\k\+\]/

highlight link logTime String
highlight link logLevelDebug Comment
highlight link logLevelInfo Special
highlight link logLevelWarning StatusLine
highlight link logLevelError Error
highlight link logThread String
highlight link logClass Function

let b:current_syntax = "hide-log"
