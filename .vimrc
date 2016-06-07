"call g:cpp_plugin.autocompleteSettings.setAutoInvokationKeys('\<C-N>')
"
nmap <F1> yyjp>>^dWis_logger.Info() << "<ESC>A";<ESC>:s/\((\<Bar>, \)\([A-Za-z0-9_:]*\*\?\<Bar>const [A-Za-z0-9:_]*&\) \([A-Za-z0-9_]*\)\ze[,)]/\1" << \3 << "/ge<CR>:noh<CR>
call g:buildsystem.setAvailableBuildConfigs( { 'host': CMakeBuildConfig(8, './build/') } )

au BufNew *.h,*.hpp,*.c,*.cpp set complete-=i

if exists('g:load_hide_plugin') && g:load_hide_plugin == 1
	if has('python3')
		python3 import sys
		python3 sys.path.insert(0, 'build/bin')
		python3 sys.path.insert(0, 'build/bin/Debug')
	else
		python import sys
		python sys.path.insert(0, 'build/bin')
		python sys.path.insert(0, 'build/bin/Debug')
	end
	let g:timer._handlers = {}
	set updatetime=1000
	set runtimepath+=./hide-vim
	nmap <C-B> :HideBuildAll<CR>
	nmap <C-F7> :HideBuildFile %<CR>
	nmap <C-LeftMouse> <LeftMouse>:HideIndexQuery <C-R>=expand('<cword>')<CR><CR>
	nmap g<C-LeftMouse> <LeftMouse>:HideIndexQuery <C-R>=expand('<cword>')<CR><CR>
	nmap <C-]> :HideIndexQuery <C-R>=expand('<cword>')<CR><CR>
	nmap g<C-]> :HideIndexQuery <C-R>=expand('<cword>')<CR><CR>
end

let g:cpp_plugin.indexer.builder.autoBuild = 1
call g:cpp_plugin.indexer.builder.addCustomRegex('c++', '/[ \t]*BEGIN_CLANG_WRAPPER_NO_DISPOSE[ \t]*\(([A-Za-z0-9_]*)\)/\1/t/')
call g:cpp_plugin.indexer.builder.addCustomRegex('c++', '/[ \t]*BEGIN_CLANG_WRAPPER_NO_DISPOSE[ \t]*\(([A-Za-z0-9_]*)\)/\1Ptr/t/')
call g:cpp_plugin.indexer.builder.addCustomRegex('c++', '/[ \t]*BEGIN_CLANG_WRAPPER[ \t]*\(([A-Za-z0-9_]*),[ \t]*[^)]*\)/\1/t/')
call g:cpp_plugin.indexer.builder.addCustomRegex('c++', '/[ \t]*BEGIN_CLANG_WRAPPER[ \t]*\(([A-Za-z0-9_]*),[ \t]*[^)]*\)/\1Ptr/t/')
call g:cpp_plugin.indexer.builder.addCustomRegex('c++', '/[ \t]*HIDE_DECLARE_PTR[ \t]*\(([A-Za-z0-9_]*)\)/\1Ptr/t/')
call g:cpp_plugin.indexer.builder.addCustomRegex('c++', '/[ \t]*HIDE_DECLARE_ARRAY[ \t]*\(([A-Za-z0-9_]*)\)/\1Array/t/')
call g:cpp_plugin.indexer.builder.addCustomRegex('c++', '/[ \t]*using[ \t]*([A-Za-z0-9_]*)[ \t]*=/\1/t/')

if exists('g:c_std_includes') && exists('g:cpp_std_includes') && exists('g:platform_includes')
	let g:include_priorities = [ 'hide/.*', 'clang[^/].*', 'wigwag.*', 'rethread.*', 'boost.*', g:platform_includes, g:cpp_std_includes, g:c_std_includes ]
end

function! GetCppNamespaceFromPath(path)
	let res = []
	if len(a:path) > 1 && a:path[0] == 'clang'
		call add(res, 'clang')
	endif
	if len(a:path) > 1 && a:path[0] == 'hide'
		call add(res, 'hide')
		if len(a:path) > 2 && a:path[1] == 'fs'
			call add(res, a:path[1])
		endif
	endif
	return res
endf

set path+=/usr/lib/llvm-3.5/include

set path+=thirdparty/rethread
set path+=thirdparty/wigwag/include
let g:include_directories = [ 'thirdparty/rethread', 'thirdparty/wigwag/include' ]
