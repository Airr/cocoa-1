.global main

cas32:
	    casx   [%o0],%o1,%o2
	    retl
	    mov    %o2,%o0

compare_and_swap:
	  ld    [%o1],%g1       ! set the old value
	  cas   [%o0],%g1,%o2   ! try the CAS
	  cmp   %g1,%o2
	  be,a  true
	  mov   1,%o0           ! return TRUE/1
	  mov   0,%o0           ! return FALSE/0
	  st    %o2,[%o1]       ! store existing value in memory

true: retl
	  nop

		
main:

save	%sp, -64, %sp
mov		1, %o1
mov		2, %o2
st		%o1, [%fp - 4]
st		%o2, [%fp - 8]
mov		%fp, %o0
sub		%o0, 4, %o0
mov		%o0,%o1
ld		[%fp - 8],%o2

b compare_and_swap


ret
restore
 