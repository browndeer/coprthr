#!/usr/bin/perl

for( $n = 2; $n <= 63; ++$n) {
	$n1 = $n-1;
	printf "#define __clarg_set".$n."(cp,krn,n,";
	printf "a0";
	for($i=1;$i<$n;++$i) {
		printf ",a$i";
	}
	printf ") do { __clarg_set(cp,krn,n,a0);";
	printf "__clarg_set".$n1."(cp,krn,n+1,";
	printf "a1";
	for($i=2;$i<$n;++$i) {
		printf ",a$i";
	}
	printf "); } while(0)\n";
}

