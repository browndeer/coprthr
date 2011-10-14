!! stdcl.f90
!!
!! Copyright (c) 2011 Brown Deer Technology, LLC.  All Rights Reserved.
!!
!! This software was developed by Brown Deer Technology, LLC.
!! For more information contact info@browndeertechnology.com
!!
!! This program is free software: you can redistribute it and/or modify it
!! under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
!! as published by the Free Software Foundation.
!!
!! This program is distributed in the hope that it will be useful,
!! but WITHOUT ANY WARRANTY; without even the implied warranty of
!! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!! GNU General Public License for more details.
!!
!! You should have received a copy of the GNU General Public License
!! along with this program.  If not, see <http://www.gnu.org/licenses/>.
!!

!! /* DAR */

	module stdcl

	use iso_c_binding

	implicit none

	integer, parameter:: CL_INT = C_INT
	integer, parameter:: CL_FLOAT = C_FLOAT

	integer, parameter :: CLLD_DEFAULT = 0
	integer, parameter :: CLLD_LAZY    = 1
	integer, parameter :: CLLD_NOW     = 2
	integer, parameter :: CLLD_NOBUILD = 4
	integer, parameter :: CLLD_GLOBAL  = 8

	integer, parameter :: CL_MEM_RO			= 1
	integer, parameter :: CL_MEM_WO			= 2
	integer, parameter :: CL_MEM_RW			= 3
	integer, parameter :: CL_MEM_HOST		= 256
	integer, parameter :: CL_MEM_DEVICE		= 512
	integer, parameter :: CL_MEM_NOCOPY		= 1024
	integer, parameter :: CL_MEM_DETACHED	= 4096 
	integer, parameter :: CL_MEM_NOFORCE	= 8192
	integer, parameter :: CL_MEM_IMAGE2D	= 65536

	integer, parameter :: CL_EVENT_WAIT			= 1
	integer, parameter :: CL_EVENT_NOWAIT		= 2
	integer, parameter :: CL_EVENT_NORELEASE 	= 8
	integer, parameter :: CL_KERNEL_EVENT		= 16
	integer, parameter :: CL_MEM_EVENT			= 32
	integer, parameter :: CL_ALL_EVENT			= (CL_KERNEL_EVENT+CL_MEM_EVENT)


	TYPE(C_PTR), bind(C,NAME="stddev") ::  stddev
	TYPE(C_PTR), bind(C,NAME="stdcpu") ::  stdcpu
	TYPE(C_PTR), bind(C,NAME="stdgpu") ::  stdgpu
	TYPE(C_PTR), bind(C,NAME="stdrpu") ::  stdrpu

	type, bind(C) :: clndrange_struct

		integer(C_SIZE_T) :: dim
		integer(C_SIZE_T), dimension(4) :: gtid_offset
   	integer(C_SIZE_T), dimension(4) :: gtid
   	integer(C_SIZE_T), dimension(4) :: ltid

	end type clndrange_struct


	interface 

		!!!! dynamic CL program loader declarations

		integer(C_INT) function clgetndev( cp ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp

		end function clgetndev


		type(C_PTR) function clopen( cp, filename, flags ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			character(kind=C_CHAR), intent(in) :: filename(*)
			integer(C_INT), value, intent(in) :: flags

		end function clopen


		type(C_PTR) function clsopen( cp, srcstr, flags ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			character(kind=C_CHAR), intent(in) :: srcstr(*)
			integer(C_INT), value, intent(in) :: flags

		end function clsopen


		type(C_PTR) function clsym( cp, handle, symbol, flags ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			type(C_PTR), value, intent(in) :: handle
			character(kind=C_CHAR), intent(in) :: symbol(*)
			integer(C_INT), value, intent(in) :: flags

		end function clsym


		integer(C_INT) function clclose( cp, handle ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			type(C_PTR), value, intent(in) :: handle

		end function clclose


		type(C_PTR) function clbuild( cp, handle, options, flags ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			type(C_PTR), value, intent(in) :: handle
			character(kind=C_CHAR), intent(in) :: options(*)
			integer(C_INT), value, intent(in) :: flags

		end function clbuild



		!!!! memory management declarations

		type(C_PTR) function clmalloc( cp, size, flags ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			integer(C_SIZE_T), value, intent(in) :: size
			integer(C_INT), value, intent(in) :: flags

		end function clmalloc


		type(C_PTR) function clmrealloc( cp, ptr, size, flags ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			type(C_PTR), value, intent(in) :: ptr
			integer(C_SIZE_T), value, intent(in) :: size
			integer(C_INT), value, intent(in) :: flags

		end function clmrealloc


		integer(C_INT) function clfree( ptr ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: ptr

		end function clfree


		integer(C_SIZE_T) function clsizeofmem( ptr ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: ptr

		end function clsizeofmem


		type(C_PTR) function clmsync( cp, devnum, ptr, flags ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			integer(C_INT), value, intent(in) :: devnum
			type(C_PTR), value, intent(in) :: ptr
			integer(C_INT), value, intent(in) :: flags

		end function clmsync


		type(C_PTR) function clmattach( cp, ptr ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			type(C_PTR), value, intent(in) :: ptr

		end function clmattach


		type(C_PTR) function clmdetach( ptr ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: ptr

		end function clmdetach




		!!!! kernel management declarations

!		type(clndrange_struct) function clndrange_init1d(gto0,gt0,lt0)
!
!			use iso_c_binding
!			implicit none
!
!			integer(C_INT), value :: gto0
!			integer(C_INT), value :: gt0
!			integer(C_INT), value :: lt0
!
!		end function clndrange_init1d
		

!		type(clndrange_struct) function clndrange_init2d(gto0,gt0,lt0, &
!				gto1,gt1,lt1)
!
!			use iso_c_binding
!			implicit none
!
!			integer(C_INT), value :: gto0
!			integer(C_INT), value :: gt0
!			integer(C_INT), value :: lt0
!			integer(C_INT), value :: gto1
!			integer(C_INT), value :: gt1
!			integer(C_INT), value :: lt1
!
!		end function clndrange_init2d
		

!		type(clndrange_struct) function clndrange_init3d(gto0,gt0,lt0, &
!				gto1,gt1,lt1,gto2,gt2,lt2)
!
!			use iso_c_binding
!			implicit none
!
!			integer(C_INT), value :: gto0
!			integer(C_INT), value :: gt0
!			integer(C_INT), value :: lt0
!			integer(C_INT), value :: gto1
!			integer(C_INT), value :: gt1
!			integer(C_INT), value :: lt1
!			integer(C_INT), value :: gto2
!			integer(C_INT), value :: gt2
!			integer(C_INT), value :: lt2
!
!		end function clndrange_init3d
		

		integer(C_INT) function clarg_set_global(cp, krn, argnum, ptr) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			type(C_PTR), value, intent(in) :: krn
			integer(C_INT), value, intent(in) :: argnum
			type(C_PTR), value, intent(in) :: ptr

		end function clarg_set_global


		type(C_PTR) function clfork(cp, devnum, krn, ndr_ptr, flags) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			integer(C_INT), value, intent(in) :: devnum
			type(C_PTR), value, intent(in) :: krn
			type(C_PTR), value, intent(in) :: ndr_ptr
			integer(C_INT), value, intent(in) :: flags

		end function clfork



		!!!! synchronization declarations

		integer(C_INT) function clflush( cp, devnum, flags ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			integer(C_INT), value, intent(in) :: devnum
			integer(C_INT), value, intent(in) :: flags

		end function clflush


		type(C_PTR) function clwait( cp, devnum, flags ) bind(C)

			use iso_c_binding
			implicit none

			type(C_PTR), value, intent(in) :: cp
			integer(C_INT), value, intent(in) :: devnum
			integer(C_INT), value, intent(in) :: flags

		end function clwait


	end interface



	!!!! clarg_set must be treated in special way 

	interface
		subroutine clSetKernelArg(krn,argnum,arg_sz,arg_ptr) &
		bind(C,NAME="clSetKernelArg")
			use iso_c_binding
			implicit none
			type(C_PTR), value, intent(in) :: krn
			integer(C_INT), value, intent(in) :: argnum
			integer(C_SIZE_T), value, intent(in) :: arg_sz
			type(C_PTR), value, intent(in) :: arg_ptr
		end subroutine clSetKernelArg
	end interface


	interface clarg_set
		module procedure clarg_set_int, clarg_set_float
	end interface clarg_set



	contains !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



	!!!! kernel management definitions

	type(clndrange_struct) function clndrange_init1d(gto0,gt0,lt0)

		use iso_c_binding
		implicit none

		integer(C_INT), value :: gto0
		integer(C_INT), value :: gt0
		integer(C_INT), value :: lt0
		
		clndrange_init1d = clndrange_struct( &
			1, (/ gto0,0,0,0 /), (/ gt0,0,0,0 /), (/ lt0,0,0,0 /) )

	end function clndrange_init1d


	type(clndrange_struct) function clndrange_init2d(gto0,gt0,lt0,gto1,gt1,lt1)

		use iso_c_binding
		implicit none

		integer(C_INT), value :: gto0
		integer(C_INT), value :: gt0
		integer(C_INT), value :: lt0
		integer(C_INT), value :: gto1
		integer(C_INT), value :: gt1
		integer(C_INT), value :: lt1
		
		clndrange_init2d = clndrange_struct( &
			1, (/ gto0,gto1,0,0 /), (/ gt0,gt1,0,0 /), (/ lt0,lt1,0,0 /) )

	end function clndrange_init2d


	type(clndrange_struct) function clndrange_init3d(gto0,gt0,lt0, &
		gto1,gt1,lt1, gto2,gt2,lt2)

		use iso_c_binding
		implicit none

		integer(C_INT), value :: gto0
		integer(C_INT), value :: gt0
		integer(C_INT), value :: lt0
		integer(C_INT), value :: gto1
		integer(C_INT), value :: gt1
		integer(C_INT), value :: lt1
		integer(C_INT), value :: gto2
		integer(C_INT), value :: gt2
		integer(C_INT), value :: lt2
		
		clndrange_init3d = clndrange_struct( &
			1, (/ gto0,gto1,0,0 /), (/ gt0,gt1,0,0 /), (/ lt0,lt1,0,0 /) )

	end function clndrange_init3d


 	integer(C_INT) function clarg_set_int( cp, krn, argnum, arg )

		use iso_c_binding
		implicit none
		type(C_PTR), value, intent(in) :: cp
		type(C_PTR), value, intent(in) :: krn
		integer(C_INT), value, intent(in) :: argnum
		integer(C_INT), target, intent(in) :: arg

		call clSetKernelArg(krn,argnum,4_8,C_LOC(arg))

		clarg_set_int = 0

	end function clarg_set_int


 	integer(C_INT) function clarg_set_float( cp, krn, argnum, arg )

		use iso_c_binding
		implicit none
		type(C_PTR), value, intent(in) :: cp
		type(C_PTR), value, intent(in) :: krn
		integer(C_INT), value, intent(in) :: argnum
		real(C_FLOAT), target, intent(in) :: arg

		call clSetKernelArg(krn,argnum,4_8,C_LOC(arg))

		clarg_set_float = 0

	end function clarg_set_float


	end module

