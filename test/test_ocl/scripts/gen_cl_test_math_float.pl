#!/usr/bin/perl
# 
# Copyright (c) 2009-2010 Brown Deer Technology, LLC.  All Rights Reserved.
#
# This software was developed by Brown Deer Technology, LLC.
# For more information contact info@browndeertechnology.com
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# DAR #

@builtins_unary = qw(
	acos acosh acospi asin asinh asinpi atan atanh atanpi
	cbrt ceil cos cosh cospi
	erfc erf exp exp2 exp10 expm1
	fabs floor
	lgamma log log2 log10 log1p logb
	rint round rsqrt
	sin sinh sinpi sqrt
);

@builtins_binary = qw(
	atan2 atan2pi
	copysign
	fdim fmax fmin fmod
	hypot
	nextafter
	pow powr
	remainder
);


foreach $f (@builtins_unary) {

		printf "\n/* $f */\n";
		printf "__kernel void\n";
		printf "test_math_".$f."_kern(\n";
		printf "\t__global float* a0,\n";
		printf "\t__global float* b0";
		printf ")\n";
		printf "{\n";
		printf "\tuint gtid = get_global_id(0);\n";
		printf "\tfloat tmp0 = a0\[gtid];\n";
		printf "\tb0\[gtid] = $f(tmp0);\n";
		printf "}\n";

}


foreach $f (@builtins_binary) {

		printf "\n/* $f */\n";
		printf "__kernel void\n";
		printf "test_math_".$f."_kern(\n";
		printf "\t__global float* a0,\n";
		printf "\t__global float* a1,\n";
		printf "\t__global float* b0";
		printf ")\n";
		printf "{\n";
		printf "\tuint gtid = get_global_id(0);\n";
		printf "\tfloat tmp0 = a0\[gtid];\n";
		printf "\tfloat tmp1 = a1\[gtid];\n";
		printf "\tb0\[gtid] = $f(tmp0,tmp1);\n";
		printf "}\n";

}

