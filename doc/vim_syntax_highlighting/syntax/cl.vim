" Vim syntax file
" Language: OpenCL
" Maintainer: James Ross
" Version: 0.5
" Last Change: 2013-03-25
"
" Description: Syntax highlighting for OpenCL 1.2
 
if exists("b:current_syntax")
  finish
endif
 
if version < 600
  so <sfile>:p:h/c.vim
else
  runtime! syntax/c.vim
  unlet b:current_syntax
endif
 
" Enumerated Types
syn keyword clType cl_channel_order cl_channel_type cl_command_queue_info cl_command_queue_properties

" [6.1.1] Built-in Data Types
syn keyword clType bool
syn keyword clType half
syn keyword clType quad
syn keyword clType char
syn keyword clType uchar
syn keyword clType short
syn keyword clType ushort
syn keyword clType int
syn keyword clType uint
syn keyword clType long
syn keyword clType ulong
syn keyword clType float
syn keyword clType double
syn keyword clType ptrdiff_t
syn keyword clType intptr_t
syn keyword clType uintptr_t

" [6.1.2] Built-in Vector Data Types
syn match clType "\(bool\|half\|quad\|char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)\(2\|3\|4\|8\|16\)"

" [6.1.3] Other Built-in Data Types
syn keyword clType image2d_t
syn keyword clType image3d_t
syn keyword clType image2d_array_t
syn keyword clType image1d_t
syn keyword clType image1d_buffer_t
syn keyword clType image1d_array_t
syn keyword clType sampler_t
syn keyword clType event_t

" [6.1.4] Reserved Data Types
syn keyword clType complex
syn keyword clType imaginary
syn match clType "float\(2\|3\|4\|8\|16\)x\(2\|3\|4\|8\|16\)"
syn match clType "double\(2\|3\|4\|8\|16\)x\(2\|3\|4\|8\|16\)"

" [6.1.7] Vector Component Addressing
syn match clStructure "\.[w-z]\{1,4\}"
syn match clStructure "\.[s][0-9a-fA-F]\{1,16\}"
syn match clStructure "\.\(lo\|hi\|odd\|even\)"

" [6.2] Conversions and Type Casting
syn match clFunction "convert_\(bool\|half\|quad\|char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)"
syn match clFunction "convert_\(bool\|half\|quad\|char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)\(2\|3\|4\|8\|16\)"
syn match clFunction "convert_\(bool\|half\|quad\|char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)_rt\(e\|p\|z\|n\)"
syn match clFunction "convert_\(bool\|half\|quad\|char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)\(2\|3\|4\|8\|16\)_rt\(e\|p\|z\|n\)"
syn match clFunction "as_\(bool\|half\|quad\|char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)"
syn match clFunction "as_\(bool\|half\|quad\|char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)\(2\|3\|4\|8\|16\)"
syn match clFunction "as_\(bool\|half\|quad\|char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)_sat_rt\(e\|p\|z\|n\)"
syn match clFunction "as_\(bool\|half\|quad\|char\|uchar\|short\|ushort\|int\|uint\|long\|ulong\|float\|double\)\(2\|3\|4\|8\|16\)_sat_rt\(e\|p\|z\|n\)"

" [6.5] Address Space Qualifiers
syn keyword clConstant __global
syn keyword clConstant global
syn keyword clConstant __constant
syn keyword clConstant constant
syn keyword clConstant __local
syn keyword clConstant local
syn keyword clConstant __private
syn keyword clConstant private
syn keyword clConstant __const
syn keyword clConstant const
syn keyword clConstant restrict
syn keyword clConstant volatile


" [6.6] Access Qualifiers
syn keyword clConstant __read_only
syn keyword clConstant read_only
syn keyword clConstant __write_only
syn keyword clConstant write_only

" [6.7] Function Qualifiers
syn keyword clConstant __kernel
syn keyword clConstant kernel
syn keyword clConstant __attribute__
syn keyword clFunction vec_type_hint
syn keyword clFunction work_group_size_hint
syn keyword clFunction reqd_work_group_size

" [6.10] Preprocessor Directives and Macros
syn keyword clPreProc __FILE__
syn keyword clPreProc __func__
syn keyword clPreProc __LINE__
syn keyword clPreProc __OPENCL_VERSION__
syn keyword clPreProc __CL_VERSION_1_0__
syn keyword clPreProc __CL_VERSION_1_1__
syn keyword clPreProc __CL_VERSION_1_2__
syn keyword clPreProc __OPENCL_C_VERSION__
syn keyword clPreProc __ENDIAN_LITTLE__
syn keyword clPreProc __IMAGE_SUPPORT__
syn keyword clPreProc __FAST_RELAXED_MATH__
syn keyword clPreProc FP_FAST_FMA
syn keyword clPreProc FP_FAST_FMAF
syn keyword clPreProc FP_FAST_FMA_HALF
syn keyword clFunction __kernel_exec
" OpenCL Extensions
syn keyword clConstant cl_khr_select_fprounding_mode
syn keyword clConstant cl_khr_global_int32_base_atomics
syn keyword clConstant cl_khr_global_int32_extended_atomics
syn keyword clConstant cl_khr_local_int32_base_atomics
syn keyword clConstant cl_khr_local_int32_extended_atomics
syn keyword clConstant cl_khr_int64_base_atomics
syn keyword clConstant cl_khr_int64_extended_atomics
syn keyword clConstant cl_khr_fp16
syn keyword clConstant cl_khr_fp64
syn keyword clConstant cl_khr_3d_image_writes
syn keyword clConstant cl_khr_byte_addressable_store
syn keyword clConstant CL_APPLE_gl_sharing
syn keyword clConstant CL_KHR_gl_sharing


" [6.11.1] Specify Type Attributes
syn keyword clFunction aligned
syn keyword clFunction packed
syn keyword clFunction endian

" [6.12.2] Math Constants
syn keyword clConstant MAXFLOAT
syn keyword clConstant HUGE_VALF
syn keyword clConstant HUGE_VAL
syn keyword clConstant INFINITY
syn keyword clConstant NAN
syn keyword clConstant M_E_F
syn keyword clConstant M_LOG2E_F
syn keyword clConstant M_LOG10E_F
syn keyword clConstant M_LN2_F
syn keyword clConstant M_LN10_F
syn keyword clConstant M_PI_F
syn keyword clConstant M_PI_2_F
syn keyword clConstant M_PI_4_F
syn keyword clConstant M_1_PI_F
syn keyword clConstant M_2_PI_F
syn keyword clConstant M_2_SQRTPI_F
syn keyword clConstant M_SQRT2_F
syn keyword clConstant M_SQRT1_2_F

" [6.12.1] Work-Item Built-in Functions
syn keyword clFunction get_work_dim
syn keyword clFunction get_global_size
syn keyword clFunction get_global_id
syn keyword clFunction get_local_size
syn keyword clFunction get_local_id
syn keyword clFunction get_num_groups
syn keyword clFunction get_group_id
syn keyword clFunction get_global_size

" [6.12.2] Math Built-in Functions
syn keyword clFunction acos
syn keyword clFunction acosh
syn keyword clFunction acospi
syn keyword clFunction asin
syn keyword clFunction asinh
syn keyword clFunction asinpi
syn keyword clFunction atan
syn keyword clFunction atan2
syn keyword clFunction atanh
syn keyword clFunction atanpi
syn keyword clFunction atan2pi
syn keyword clFunction cbrt
syn keyword clFunction ceil
syn keyword clFunction copysign
syn keyword clFunction cos
syn keyword clFunction cosh
syn keyword clFunction cospi
syn keyword clFunction erfc
syn keyword clFunction erf
syn keyword clFunction exp
syn keyword clFunction exp2
syn keyword clFunction exp10
syn keyword clFunction expm1
syn keyword clFunction fabs
syn keyword clFunction fdim
syn keyword clFunction floor
syn keyword clFunction fma
syn keyword clFunction fmax
syn keyword clFunction fmin
syn keyword clFunction fmod
syn keyword clFunction fract
syn keyword clFunction frexp
syn keyword clFunction hypot
syn keyword clFunction ilogb
syn keyword clFunction ldexp
syn keyword clFunction lgamma
syn keyword clFunction lgamma_r
syn keyword clFunction log
syn keyword clFunction log2
syn keyword clFunction log10
syn keyword clFunction log1p
syn keyword clFunction logb
syn keyword clFunction mad
syn keyword clFunction modf
syn keyword clFunction nan
syn keyword clFunction nextafter
syn keyword clFunction pow
syn keyword clFunction pown
syn keyword clFunction powr
syn keyword clFunction remainder
syn keyword clFunction remquo
syn keyword clFunction rint
syn keyword clFunction rootn
syn keyword clFunction round
syn keyword clFunction rsqrt
syn keyword clFunction sin
syn keyword clFunction sincos
syn keyword clFunction sinh
syn keyword clFunction sinpi
syn keyword clFunction sqrt
syn keyword clFunction tan
syn keyword clFunction tanh
syn keyword clFunction tanpi
syn keyword clFunction tgamma
syn keyword clFunction trunc
syn match clFunction "\(half\|native\)_\(cos\|divide\|exp\|exp2\|exp10\|log\|log2\|log10\|powr\|recip\|rsqrt\|sin\|sqrt\|tan\)"

" [6.12.3] Integer Built-in Functions
syn keyword clFunction abs
syn keyword clFunction abs_diff
syn keyword clFunction add_sat
syn keyword clFunction hadd
syn keyword clFunction rhadd
syn keyword clFunction clamp
syn keyword clFunction clz
syn keyword clFunction mad_hi
syn keyword clFunction mad24
syn keyword clFunction mad_sat
syn keyword clFunction max
syn keyword clFunction min
syn keyword clFunction mul_hi
syn keyword clFunction rotate
syn keyword clFunction sub_sat
syn keyword clFunction popcount
syn keyword clFunction upsample
syn keyword clFunction mad24
syn keyword clFunction mul24

" [6.12.4] Common Built-in Functions
syn keyword clFunction degrees
syn keyword clFunction mix
syn keyword clFunction radians
syn keyword clFunction step 
syn keyword clFunction smoothstep
syn keyword clFunction sign

" [6.12.5] Geometric Built-in Functions
syn keyword clFunction clFunction
syn keyword clFunction cross
syn keyword clFunction dot
syn keyword clFunction distance
syn keyword clFunction length
syn keyword clFunction normalize
syn keyword clFunction fast_distance
syn keyword clFunction fast_length
syn keyword clFunction fast_normalize

" [6.12.6] Relational Built-in Functions
syn keyword clFunction isequal
syn keyword clFunction isnotequal
syn keyword clFunction isgreater
syn keyword clFunction isgreaterequal
syn keyword clFunction isless
syn keyword clFunction islessequal
syn keyword clFunction islessgreater
syn keyword clFunction isfinite
syn keyword clFunction isinf
syn keyword clFunction isnan
syn keyword clFunction isnormal
syn keyword clFunction isordered
syn keyword clFunction isunordered
syn keyword clFunction signbit
syn keyword clFunction any
syn keyword clFunction all
syn keyword clFunction bitselect
syn keyword clFunction select

" [6.12.7] Vector Data Load/Store
syn keyword clFunction vload vstore vload_half vstore_half vloada_half vstorea_half
syn match clFunction "\(vload\|vstore\|vload_half\|vstore_half\|vloada_half\|vstorea_half\)\(2\|3\|4\|8\|16\)"

" [6.12.8, 6.12.9] Synchronization and Explicit Memory Fence Functions
syn keyword clFunction mem_fence
syn keyword clFunction read_mem_fence
syn keyword clFunction write_mem_fence
syn keyword clFunction barrier
syn keyword clConstant CLK_LOCAL_MEM_FENCE
syn keyword clConstant CLK_GLOBAL_MEM_FENCE

" [6.12.10] Async Copies and Prefetch Functions
syn keyword clFunction async_work_group_copy
syn keyword clFunction async_work_group_strided_copy
syn keyword clFunction wait_group_events
syn keyword clFunction prefetch

" [6.12.11] Atomic Functions
syn keyword clFunction atom_add
syn keyword clFunction atom_sub
syn keyword clFunction atom_xchg
syn keyword clFunction atom_inc
syn keyword clFunction atom_dec
syn keyword clFunction atom_cmpxchg
syn keyword clFunction atom_min
syn keyword clFunction atom_max
syn keyword clFunction atom_and
syn keyword clFunction atom_or
syn keyword clFunction atom_xor

" [6.12.12] Miscellaneous Vector Functions
syn keyword clFunction vec_step
syn keyword clFunction shuffle
syn keyword clFunction shuffle2

" [6.12.13] printf Function
syn keyword clFunction printf

" [6.12.14] Image Read and Write Built-in Functions
syn match clFunction "read_image\(f\|i\|ui\|h\)"
syn match clFunction "write_image\(f\|i\|ui\|h\)"

" [6.12.14.5] Image Query Functions
syn match clFunction "get_image_\(width\|height\|depth\|array_size\|dim\|channel_data_type\|channel_order\)"

" [6.12.14.1] Sampler Declaration Fields
syn match clConstant "CLK_ADDRESS_\(REPEAT\|CLAMP\|NONE\)"
syn match clConstant "CLK_ADDRESS_\(CLAMP_TO_EDGE\|MIRRORED_REPEAT\)"
syn match clConstant "CLK_NORMALIZED_COORDS_\(TRUE\|FALSE\)"
syn keyword clConstant CLK_FILTER_NEAREST
syn keyword clConstant CLK_FILTER_LINEAR

let b:current_syntax = "cl" 	

hi def link clType Type
hi def link clPreProc PreProc
hi def link clFunction Function
hi def link clConstant Constant
hi def link clStructure Structure
