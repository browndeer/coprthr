filetype on
au Syntax opencl source ~/.vim/syntax/opencl.vim
au BufNewFile,BufRead *.c,*.cpp,*.h,*.hpp set ft=opencl
au Syntax stdcl source ~/.vim/syntax/stdcl.vim
au BufNewFile,BufRead *.c,*.cpp,*.h,*.hpp set ft=stdcl
au Syntax cl source ~/.vim/syntax/cl.vim
au BufNewFile,BufRead *.cl,*.clh set ft=cl
au Syntax coprthr_kern source ~/.vim/syntax/coprthr_kern.vim
au BufNewFile,BufRead *.cl set ft=coprthr_kern
set tabstop=5
syntax on

