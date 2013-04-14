" Vim syntax file
" Language: C STDCL
" Maintainer: James Ross
" Version: 0.2
" Last Change: 2013-03-25
"
" Description: Syntax highlighting for STDCL
 
 
"if exists("b:current_syntax")
"	finish
"endif
 
if version < 600
	so <sfile>:p:h/c.vim
else
	runtime! syntax/c.vim
	unlet b:current_syntax
endif
 
" Types
syn match stdclType "cl_\(char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)"
syn match stdclType "cl_\(char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)\(2\|3\|4\|8\)"
syn match stdclType "cl_\(char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)16"
syn keyword stdclType cldev_info
syn keyword stdclType clmulti_array
syn keyword stdclType clndrange_struct
syn keyword stdclType clstat_info
syn keyword stdclType clvector
 
" Constants {{{
syn keyword stdclConstant __STDCL__
syn keyword stdclConstant stddev
syn keyword stdclConstant stdcpu
syn keyword stdclConstant stdgpu
syn keyword stdclConstant stdrpu
syn keyword stdclConstant stdacc
syn keyword stdclConstant stdnpu
syn keyword stdclConstant STDCL_EVENTLIST_MAX
syn keyword stdclConstant CL_DEVICE_TYPE_RPU
syn keyword stdclConstant CLCONTEXT
syn keyword stdclConstant CONTEXT
syn match stdclConstant "CLLD_\(DEFAULT\|LAZY\|NOW\|NOBUILD\|GLOBAL\)"
syn match stdclConstant "CL_MEM_\(RO\|WO\|RW\|HOST\|DEVICE\|NOCOPY\|DETACHED\|NOFORCE\|IMAGE2D\|CLBUF\|GLBUF\|GLTEX2D\|GLTEX3D\|GLRBUF\)"
syn match stdclConstant "CL_MCTL_\(GET_STATUS\|GET_DEVNUM\|SET_DEVNUM\|MARK_CLEAN\|SET_IMAGE2D\|SET_USRFLAGS\|CLR_USRFLAGS\)"
syn keyword stdclConstant CLMEM_MAGIC
syn keyword stdclConstant CL_FAST
syn keyword stdclConstant CL_EVENT_WAIT
syn keyword stdclConstant CL_EVENT_NOWAIT
syn keyword stdclConstant CL_EVENT_RELEASE
syn keyword stdclConstant CL_EVENT_NORELEASE
syn keyword stdclConstant CL_KERNEL_EVENT
syn keyword stdclConstant CL_MEM_EVENT
syn keyword stdclConstant CL_ALL_EVENT

" }}}
 
" Functions
syn keyword stdclFunction clcontext_create
syn keyword stdclFunction clcontext_create_stdnpu
syn keyword stdclFunction clcontext_destroy
syn keyword stdclFunction clgetndev
syn keyword stdclFunction clstat
syn keyword stdclFunction clgetdevinfo
syn keyword stdclFunction clfreport_devinfo
syn keyword stdclFunction clload
syn keyword stdclFunction clloadb
syn keyword stdclFunction clbuild
syn keyword stdclFunction clopen
syn keyword stdclFunction clsopen
syn keyword stdclFunction clsym
syn keyword stdclFunction clclose
syn keyword stdclFunction clerror
syn keyword stdclFunction clsizeofmem
syn keyword stdclFunction clmalloc
syn keyword stdclFunction clfree
syn keyword stdclFunction clmattach
syn keyword stdclFunction clmdetach
syn keyword stdclFunction clmctl_va
syn keyword stdclFunction clmrealloc
syn keyword stdclFunction clmsync
syn keyword stdclFunction clmcopy
syn keyword stdclFunction clmemptr
syn keyword stdclFunction clglmalloc
syn keyword stdclFunction clglmsync
syn keyword stdclFunction clmctl
syn keyword stdclFunction clarg_set_global
syn keyword stdclFunction clarg_set
syn keyword stdclFunction clarg_set_local
syn keyword stdclFunction clforka
syn keyword stdclFunction clfork
syn keyword stdclFunction clwait
syn keyword stdclFunction clwaitev
syn keyword stdclFunction clflush
syn keyword stdclFunction oclperror
syn keyword stdclFunction clperror
syn keyword stdclFunction oclperror_str
syn keyword stdclFunction clperror_str

" Macros 
syn keyword stdclMacro clndrange_init1d
syn keyword stdclMacro clndrange_init2d
syn keyword stdclMacro clndrange_init3d
syn keyword stdclMacro clndrange_set1d
syn keyword stdclMacro clndrange_set2d
syn keyword stdclMacro clndrange_set3d

let b:current_syntax = "stdcl"
 
hi def link stdclType Type
hi def link stdclFunction Function
hi def link stdclMacro PreProc
hi def link stdclConstant Constant
