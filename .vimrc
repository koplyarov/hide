"call g:cpp_plugin.autocompleteSettings.setAutoInvokationKeys('\<C-N>')

let g:cpp_plugin.indexer.builder.autoBuild = 1
call g:cpp_plugin.indexer.builder.addCustomRegex('c++', '/[ \t]*BEGIN_CLANG_WRAPPER_NO_DISPOSE[ \t]*\(([A-Za-z0-9_]*)\)/\1/s/')
call g:cpp_plugin.indexer.builder.addCustomRegex('c++', '/[ \t]*BEGIN_CLANG_WRAPPER_NO_DISPOSE[ \t]*\(([A-Za-z0-9_]*)\)/\1Ptr/s/')
call g:cpp_plugin.indexer.builder.addCustomRegex('c++', '/[ \t]*BEGIN_CLANG_WRAPPER[ \t]*\(([A-Za-z0-9_]*),[ \t]*[^)]*\)/\1/s/')
call g:cpp_plugin.indexer.builder.addCustomRegex('c++', '/[ \t]*BEGIN_CLANG_WRAPPER[ \t]*\(([A-Za-z0-9_]*),[ \t]*[^)]*\)/\1Ptr/s/')

if exists('g:c_std_includes') && exists('g:cpp_std_includes') && exists('g:platform_includes')
	let g:include_priorities = [ g:c_std_includes, g:cpp_std_includes, g:platform_includes, '^clang[^/].*', 'clang/', '.*' ]
end

function! GetCppNamespaceFromPath(path)
	let res = []
	if len(a:path) > 1 && a:path[0] == 'clang'
		call add(res, 'clang')
	endif
	return res
endf

set path+=/usr/lib/llvm-3.5/include
