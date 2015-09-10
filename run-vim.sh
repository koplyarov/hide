#!/bin/sh
export TERM='xterm-256color'
rm -rf ./.hide
vim --cmd "let g:load_hide_plugin = 1" --cmd "let g:loaded_TagHighlight = 1" -c HideLog $1
