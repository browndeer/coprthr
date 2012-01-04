
#ifndef _FAST_SETJMP_H
#define _FAST_SETJMP_H

typedef long int fast_jmp_buf[8];

int fast_setjmp( fast_jmp_buf env);
void fast_longjmp( fast_jmp_buf env, int val);
int fast_barrier( int flags );

#endif

