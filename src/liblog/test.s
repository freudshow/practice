	.syntax unified
	.arch armv7-a
	.eabi_attribute 27, 3
	.eabi_attribute 28, 1
	.fpu vfpv3-d16
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.thumb
	.file	"test.c"
	.section	.rodata
	.align	2
.LC2:
	.ascii	"test.c\000"
	.align	2
.LC3:
	.ascii	"world\000"
	.align	2
.LC4:
	.ascii	"[hello %s!!!]\000"
	.align	2
.LC5:
	.ascii	"-------------out done\000"
	.align	2
.LC0:
	.byte	1
	.byte	2
	.byte	104
	.byte	-2
	.align	2
.LC1:
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	51
	.byte	20
	.byte	4
	.byte	0
	.byte	53
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	9
	.byte	3
	.byte	80
	.byte	0
	.byte	0
	.byte	1
	.byte	-108
	.byte	66
	.byte	-33
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	51
	.byte	20
	.byte	4
	.byte	0
	.byte	53
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	9
	.byte	3
	.byte	80
	.byte	0
	.byte	0
	.byte	1
	.byte	-108
	.byte	66
	.byte	-33
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	51
	.byte	20
	.byte	4
	.byte	0
	.byte	53
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	9
	.byte	3
	.byte	80
	.byte	0
	.byte	0
	.byte	1
	.byte	-108
	.byte	66
	.byte	-33
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	51
	.byte	20
	.byte	4
	.byte	0
	.byte	53
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	9
	.byte	3
	.byte	80
	.byte	0
	.byte	0
	.byte	1
	.byte	-108
	.byte	66
	.byte	-33
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	51
	.byte	20
	.byte	4
	.byte	0
	.byte	53
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	9
	.byte	3
	.byte	80
	.byte	0
	.byte	0
	.byte	1
	.byte	-108
	.byte	66
	.byte	-33
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	51
	.byte	20
	.byte	4
	.byte	0
	.byte	53
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	9
	.byte	3
	.byte	80
	.byte	0
	.byte	0
	.byte	1
	.byte	-108
	.byte	66
	.byte	-33
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	51
	.byte	20
	.byte	4
	.byte	0
	.byte	53
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	9
	.byte	3
	.byte	80
	.byte	0
	.byte	0
	.byte	1
	.byte	-108
	.byte	66
	.byte	-33
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	52
	.byte	20
	.byte	4
	.byte	0
	.byte	3
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	10
	.byte	3
	.byte	81
	.byte	0
	.byte	0
	.byte	1
	.byte	-107
	.byte	-115
	.byte	-4
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	52
	.byte	20
	.byte	4
	.byte	0
	.byte	3
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	10
	.byte	3
	.byte	81
	.byte	0
	.byte	0
	.byte	1
	.byte	-107
	.byte	-115
	.byte	-4
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	52
	.byte	20
	.byte	4
	.byte	0
	.byte	3
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	10
	.byte	3
	.byte	81
	.byte	0
	.byte	0
	.byte	1
	.byte	-107
	.byte	-115
	.byte	-4
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	52
	.byte	20
	.byte	4
	.byte	0
	.byte	3
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	10
	.byte	3
	.byte	81
	.byte	0
	.byte	0
	.byte	1
	.byte	-107
	.byte	-115
	.byte	-4
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	52
	.byte	20
	.byte	4
	.byte	0
	.byte	3
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	10
	.byte	3
	.byte	81
	.byte	0
	.byte	0
	.byte	1
	.byte	-107
	.byte	-115
	.byte	-4
	.byte	22
	.byte	104
	.byte	32
	.byte	0
	.byte	-61
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	52
	.byte	20
	.byte	4
	.byte	0
	.byte	3
	.byte	0
	.byte	21
	.byte	18
	.byte	25
	.byte	32
	.byte	1
	.byte	0
	.byte	8
	.byte	10
	.byte	3
	.byte	81
	.byte	0
	.byte	0
	.byte	1
	.byte	-107
	.byte	-115
	.byte	-4
	.byte	22
	.text
	.align	2
	.global	main
	.thumb
	.thumb_func
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 424
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r7, lr}
	sub	sp, sp, #440
	add	r7, sp, #16
	movw	r3, #:lower16:.LC0
	movt	r3, #:upper16:.LC0
	add	r2, r7, #420
	ldr	r0, [r3]	@ unaligned
	str	r0, [r2]	@ unaligned
	adds	r2, r7, #4
	movw	r3, #:lower16:.LC1
	movt	r3, #:upper16:.LC1
	mov	r0, r2
	mov	r1, r3
	mov	r3, #416
	mov	r2, r3
	bl	memcpy
	movw	r3, #:lower16:stdout
	movt	r3, #:upper16:stdout
	ldr	r0, [r3]
	movw	r3, #:lower16:.LC3
	movt	r3, #:upper16:.LC3
	str	r3, [sp, #12]
	movw	r3, #:lower16:.LC4
	movt	r3, #:upper16:.LC4
	str	r3, [sp, #8]
	mov	r3, #416
	str	r3, [sp, #4]
	adds	r3, r7, #4
	str	r3, [sp]
	movs	r3, #69
	movw	r2, #:lower16:__FUNCTION__.5274
	movt	r2, #:upper16:__FUNCTION__.5274
	movw	r1, #:lower16:.LC2
	movt	r1, #:upper16:.LC2
	bl	debugBufFormat2fp
	movw	r0, #:lower16:.LC5
	movt	r0, #:upper16:.LC5
	bl	puts
	movs	r0, #0
	bl	exit
	.size	main, .-main
	.section	.rodata
	.align	2
	.type	__FUNCTION__.5274, %object
	.size	__FUNCTION__.5274, 5
__FUNCTION__.5274:
	.ascii	"main\000"
	.ident	"GCC: (Linaro GCC 4.9-2016.02) 4.9.4 20151028 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
