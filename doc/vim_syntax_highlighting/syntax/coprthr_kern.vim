" Vim syntax file
" Language: COPRTHR OpenCL kernel code
" Maintainer: James Ross
" Version: 0.2
" Last Change: 2013-03-25
"
" Description: Syntax highlighting for COPRTHR OpenCL kernels
 
 
"if exists("b:current_syntax")
"  finish
"endif
 
if version < 600
  so <sfile>:p:h/c.vim
else
  runtime! syntax/c.vim
  unlet b:current_syntax
endif
 

" Types
syn keyword coprthrType char uchar short ushort int uint long ulong float double
syn match coprthrType "\(char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)\(2\|3\|4\|8\|16\)"
syn keyword coprthrType image2d_t
syn keyword coprthrType sampler_t
 
" Constants
syn match coprthrConstant "CLK_ADDRESS_\(REPEAT\|CLAMP\|NONE\)"
syn match coprthrConstant "CLK_ADDRESS_\(CLAMP_TO_EDGE\|MIRRORED_REPEAT\)"
syn match coprthrConstant "CLK_NORMALIZED_COORDS_\(TRUE\|FALSE\)"
syn keyword coprthrConstant CLK_LOCAL_MEM_FENCE

" Pre Processor
syn keyword coprthrPreProc global
syn keyword coprthrPreProc local
syn keyword coprthrPreProc constant
syn keyword coprthrPreProc private
syn keyword coprthrPreProc __global
syn keyword coprthrPreProc __local
syn keyword coprthrPreProc __constant
syn keyword coprthrPreProc __private
syn keyword coprthrPreProc __read_only
syn keyword coprthrPreProc __write_only
syn keyword coprthrPreProc __kernel
syn keyword coprthrPreProc restrict
syn keyword coprthrPreProc __restrict
syn match coprthrPreProc "__builtin_vector_\(char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)\(2\|3\|4\|8\|16\)"
syn match coprthrPreProc "vector_\(char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)\(2\|3\|4\|8\|16\)"

" Functions
syn keyword coprthrFunction reqd_work_group_size
syn keyword coprthrFunction get_work_dim
syn keyword coprthrFunction get_global_size
syn keyword coprthrFunction get_global_id
syn keyword coprthrFunction get_local_size
syn keyword coprthrFunction get_local_id
syn keyword coprthrFunction get_num_groups
syn keyword coprthrFunction get_group_id
syn keyword coprthrFunction sqrt acos acosh asin asinh tan tanh cbrt ceil cos cosh erfc erf exp exp2 exp10 expm1 fabs floor log log2 log10 log1p logb rint round rsqrt sin sinh sqrt tan tanh tgamma tfunc clamp dot normalize cross read_imagei read_imageui read_imagef
syn match coprthrFunction "read_image\(f\|i\|ui\|h\)"
syn match coprthrFunction "write_image\(f\|i\|ui\|h\)"

" Vector Component Addressing 
syn match coprthrStructure "\.[w-z]\{1,4\}"
syn match coprthrStructure "\.[s][0-9a-fA-F]\{1,16\}"
syn match coprthrStructure "\.\(lo\|hi\)"

let b:current_syntax = "coprthr_kern" 

hi def link coprthrType Type
hi def link coprthrPreProc PreProc
hi def link coprthrFunction Function
hi def link coprthrConstant Constant
hi def link coprthrStructure Structure
