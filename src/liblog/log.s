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
	.file	"log.c"
	.section	.rodata
	.align	2
.LC0:
	.ascii	"%04d-%02d-%02d %02d:%02d:%02d\000"
	.text
	.align	2
	.global	get_local_time
	.thumb
	.thumb_func
	.type	get_local_time, %function
get_local_time:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r4, r5, r7, lr}
	sub	sp, sp, #32
	add	r7, sp, #16
	str	r0, [r7, #4]
	add	r3, r7, #8
	mov	r0, r3
	bl	time
	add	r3, r7, #8
	mov	r0, r3
	bl	localtime
	str	r0, [r7, #12]
	ldr	r3, [r7, #12]
	ldr	r3, [r3, #20]
	addw	r4, r3, #1900
	ldr	r3, [r7, #12]
	ldr	r3, [r3, #16]
	adds	r5, r3, #1
	ldr	r3, [r7, #12]
	ldr	r3, [r3, #12]
	ldr	r2, [r7, #12]
	ldr	r2, [r2, #8]
	ldr	r1, [r7, #12]
	ldr	r1, [r1, #4]
	ldr	r0, [r7, #12]
	ldr	r0, [r0]
	str	r0, [sp, #12]
	str	r1, [sp, #8]
	str	r2, [sp, #4]
	str	r3, [sp]
	mov	r3, r5
	mov	r2, r4
	movw	r1, #:lower16:.LC0
	movt	r1, #:upper16:.LC0
	ldr	r0, [r7, #4]
	bl	sprintf
	adds	r7, r7, #16
	mov	sp, r7
	@ sp needed
	pop	{r4, r5, r7, pc}
	.size	get_local_time, .-get_local_time
	.section	.rodata
	.align	2
.LC1:
	.ascii	"ls -r %s*|cut -c %d-99\000"
	.align	2
.LC2:
	.ascii	"r\000"
	.align	2
.LC3:
	.ascii	"execute command failed: %s\000"
	.text
	.align	2
	.global	getMaxFileNo
	.thumb
	.thumb_func
	.type	getMaxFileNo, %function
getMaxFileNo:
	@ args = 0, pretend = 0, frame = 248
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r4, r7, lr}
	sub	sp, sp, #252
	add	r7, sp, #0
	str	r0, [r7, #4]
	movs	r3, #0
	str	r3, [r7, #244]
	add	r3, r7, #136
	movs	r2, #100
	movs	r1, #0
	mov	r0, r3
	bl	memset
	add	r3, r7, #8
	movs	r2, #128
	movs	r1, #0
	mov	r0, r3
	bl	memset
	movs	r3, #0
	str	r3, [r7, #236]
	ldr	r0, [r7, #4]
	bl	strlen
	mov	r3, r0
	adds	r3, r3, #2
	add	r0, r7, #136
	ldr	r2, [r7, #4]
	movw	r1, #:lower16:.LC1
	movt	r1, #:upper16:.LC1
	bl	sprintf
	add	r3, r7, #136
	movw	r1, #:lower16:.LC2
	movt	r1, #:upper16:.LC2
	mov	r0, r3
	bl	popen
	str	r0, [r7, #236]
	ldr	r3, [r7, #236]
	cmp	r3, #0
	bne	.L3
	movw	r3, #:lower16:stdout
	movt	r3, #:upper16:stdout
	ldr	r4, [r3]
	bl	__errno_location
	mov	r3, r0
	ldr	r3, [r3]
	mov	r0, r3
	bl	strerror
	mov	r3, r0
	mov	r2, r3
	movw	r1, #:lower16:.LC3
	movt	r1, #:upper16:.LC3
	mov	r0, r4
	bl	fprintf
	mov	r3, #-1
	b	.L9
.L3:
	add	r0, r7, #8
	ldr	r3, [r7, #236]
	movs	r2, #128
	movs	r1, #1
	bl	fread
	mov	r3, r0
	cmp	r3, #0
	beq	.L5
	movs	r3, #0
	str	r3, [r7, #240]
	b	.L6
.L7:
	ldr	r3, [r7, #240]
	adds	r3, r3, #1
	str	r3, [r7, #240]
.L6:
	add	r2, r7, #8
	ldr	r3, [r7, #240]
	add	r3, r3, r2
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #10
	bne	.L7
	add	r2, r7, #8
	ldr	r3, [r7, #240]
	add	r3, r3, r2
	movs	r2, #0
	strb	r2, [r3]
	add	r3, r7, #8
	mov	r0, r3
	bl	atoi
	str	r0, [r7, #244]
	b	.L8
.L5:
	mov	r3, #-1
	str	r3, [r7, #244]
.L8:
	ldr	r0, [r7, #236]
	bl	pclose
	ldr	r3, [r7, #244]
.L9:
	mov	r0, r3
	adds	r7, r7, #252
	mov	sp, r7
	@ sp needed
	pop	{r4, r7, pc}
	.size	getMaxFileNo, .-getMaxFileNo
	.section	.rodata
	.align	2
.LC4:
	.ascii	"%s.%d\000"
	.align	2
.LC5:
	.ascii	"mv %s.%d %s.%d\000"
	.align	2
.LC6:
	.ascii	"mv %s %s.%d\000"
	.text
	.align	2
	.global	mvFiles
	.thumb
	.thumb_func
	.type	mvFiles, %function
mvFiles:
	@ args = 0, pretend = 0, frame = 320
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r7, lr}
	sub	sp, sp, #328
	add	r7, sp, #8
	adds	r3, r7, #4
	str	r0, [r3]
	mov	r3, r7
	str	r1, [r3]
	adds	r3, r7, #4
	movs	r1, #0
	ldr	r0, [r3]
	bl	access
	mov	r3, r0
	cmp	r3, #0
	beq	.L11
	mov	r3, #-1
	b	.L18
.L11:
	add	r3, r7, #212
	movs	r2, #100
	movs	r1, #0
	mov	r0, r3
	bl	memset
	adds	r3, r7, #4
	ldr	r0, [r3]
	bl	getMaxFileNo
	str	r0, [r7, #316]
	movs	r3, #0
	str	r3, [r7, #312]
	ldr	r3, [r7, #316]
	cmp	r3, #0
	bge	.L13
	mov	r3, #-1
	b	.L18
.L13:
	ldr	r3, [r7, #316]
	adds	r2, r3, #1
	mov	r3, r7
	ldr	r3, [r3]
	cmp	r2, r3
	bne	.L14
	adds	r2, r7, #4
	add	r0, r7, #212
	ldr	r3, [r7, #316]
	ldr	r2, [r2]
	movw	r1, #:lower16:.LC4
	movt	r1, #:upper16:.LC4
	bl	sprintf
	ldr	r3, [r7, #316]
	subs	r3, r3, #1
	str	r3, [r7, #316]
	add	r3, r7, #212
	mov	r0, r3
	bl	unlink
	bl	sync
.L14:
	add	r3, r7, #12
	mov	r0, r3
	movs	r3, #200
	mov	r2, r3
	movs	r1, #0
	bl	memset
	ldr	r3, [r7, #316]
	str	r3, [r7, #312]
	b	.L15
.L16:
	ldr	r3, [r7, #312]
	adds	r3, r3, #1
	adds	r2, r7, #4
	add	r0, r7, #12
	str	r3, [sp, #4]
	adds	r3, r7, #4
	ldr	r3, [r3]
	str	r3, [sp]
	ldr	r3, [r7, #312]
	ldr	r2, [r2]
	movw	r1, #:lower16:.LC5
	movt	r1, #:upper16:.LC5
	bl	sprintf
	add	r3, r7, #12
	mov	r0, r3
	bl	system
	ldr	r3, [r7, #312]
	subs	r3, r3, #1
	str	r3, [r7, #312]
.L15:
	ldr	r3, [r7, #312]
	cmp	r3, #0
	bgt	.L16
	mov	r3, r7
	ldr	r3, [r3]
	cmp	r3, #1
	ble	.L17
	ldr	r3, [r7, #312]
	adds	r1, r3, #1
	adds	r3, r7, #4
	adds	r2, r7, #4
	add	r0, r7, #12
	str	r1, [sp]
	ldr	r3, [r3]
	ldr	r2, [r2]
	movw	r1, #:lower16:.LC6
	movt	r1, #:upper16:.LC6
	bl	sprintf
	add	r3, r7, #12
	mov	r0, r3
	bl	system
.L17:
	movs	r3, #0
.L18:
	mov	r0, r3
	add	r7, r7, #320
	mov	sp, r7
	@ sp needed
	pop	{r7, pc}
	.size	mvFiles, .-mvFiles
	.section	.rodata
	.align	2
.LC7:
	.ascii	"rm -f %s\000"
	.text
	.align	2
	.global	logLimit
	.thumb
	.thumb_func
	.type	logLimit, %function
logLimit:
	@ args = 0, pretend = 0, frame = 240
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r7, lr}
	sub	sp, sp, #240
	add	r7, sp, #0
	str	r0, [r7, #12]
	str	r1, [r7, #8]
	str	r2, [r7, #4]
	add	r3, r7, #108
	movs	r2, #128
	movs	r1, #0
	mov	r0, r3
	bl	memset
	movs	r3, #0
	str	r3, [r7, #236]
	add	r3, r7, #16
	mov	r1, r3
	ldr	r0, [r7, #12]
	bl	stat
	mov	r3, r0
	cmp	r3, #-1
	bne	.L20
	add	r3, r7, #108
	ldr	r2, [r7, #12]
	movw	r1, #:lower16:.LC7
	movt	r1, #:upper16:.LC7
	mov	r0, r3
	bl	sprintf
	add	r3, r7, #108
	mov	r0, r3
	bl	system
	mov	r3, #-1
	b	.L23
.L20:
	ldr	r2, [r7, #60]
	ldr	r3, [r7, #8]
	cmp	r2, r3
	ble	.L22
	ldr	r1, [r7, #4]
	ldr	r0, [r7, #12]
	bl	mvFiles
	str	r0, [r7, #236]
.L22:
	ldr	r3, [r7, #236]
.L23:
	mov	r0, r3
	adds	r7, r7, #240
	mov	sp, r7
	@ sp needed
	pop	{r7, pc}
	.size	logLimit, .-logLimit
	.section	.rodata
	.align	2
.LC8:
	.ascii	"%02X \000"
	.text
	.align	2
	.global	getBufString
	.thumb
	.thumb_func
	.type	getBufString, %function
getBufString:
	@ args = 0, pretend = 0, frame = 32
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r7, lr}
	sub	sp, sp, #32
	add	r7, sp, #0
	str	r0, [r7, #12]
	str	r1, [r7, #8]
	str	r2, [r7, #4]
	movs	r3, #0
	str	r3, [r7, #28]
	movs	r3, #0
	str	r3, [r7, #20]
	movs	r3, #0
	strb	r3, [r7, #24]
	ldr	r3, [r7, #12]
	cmp	r3, #0
	beq	.L24
	ldr	r3, [r7, #8]
	cmp	r3, #0
	ble	.L24
	movs	r3, #0
	str	r3, [r7, #28]
	b	.L28
.L29:
	ldr	r3, [r7, #28]
	ldr	r2, [r7, #12]
	add	r3, r3, r2
	ldrb	r3, [r3]	@ zero_extendqisi2
	mov	r2, r3
	add	r3, r7, #20
	movw	r1, #:lower16:.LC8
	movt	r1, #:upper16:.LC8
	mov	r0, r3
	bl	sprintf
	add	r3, r7, #20
	mov	r1, r3
	ldr	r0, [r7, #4]
	bl	strcat
	ldr	r3, [r7, #28]
	adds	r3, r3, #1
	str	r3, [r7, #28]
.L28:
	ldr	r2, [r7, #28]
	ldr	r3, [r7, #8]
	cmp	r2, r3
	blt	.L29
.L24:
	adds	r7, r7, #32
	mov	sp, r7
	@ sp needed
	pop	{r7, pc}
	.size	getBufString, .-getBufString
	.section	.rodata
	.align	2
.LC9:
	.ascii	"\012[%s][%s][%s()][%d]: \000"
	.align	2
.LC10:
	.ascii	"%s\012\000"
	.text
	.align	2
	.global	debugBufFormat2fp
	.thumb
	.thumb_func
	.type	debugBufFormat2fp, %function
debugBufFormat2fp:
	@ args = 12, pretend = 0, frame = 1064
	@ frame_needed = 1, uses_anonymous_args = 1
	push	{r4, r7, lr}
	subw	sp, sp, #1076
	add	r7, sp, #8
	add	r4, r7, #12
	str	r0, [r4]
	add	r0, r7, #8
	str	r1, [r0]
	adds	r1, r7, #4
	str	r2, [r1]
	mov	r2, r7
	str	r3, [r2]
	add	r3, r7, #1040
	movs	r2, #0
	str	r2, [r3]
	adds	r3, r3, #4
	movs	r2, #0
	str	r2, [r3]
	adds	r3, r3, #4
	movs	r2, #0
	str	r2, [r3]
	adds	r3, r3, #4
	movs	r2, #0
	str	r2, [r3]
	adds	r3, r3, #4
	movs	r2, #0
	str	r2, [r3]
	adds	r3, r3, #4
	add	r3, r7, #12
	ldr	r3, [r3]
	cmp	r3, #0
	beq	.L30
	add	r3, r7, #1040
	mov	r0, r3
	bl	get_local_time
	add	r3, r7, #8
	add	r1, r7, #1040
	add	r0, r7, #12
	mov	r2, r7
	ldr	r2, [r2]
	str	r2, [sp, #4]
	adds	r2, r7, #4
	ldr	r2, [r2]
	str	r2, [sp]
	ldr	r3, [r3]
	mov	r2, r1
	movw	r1, #:lower16:.LC9
	movt	r1, #:upper16:.LC9
	ldr	r0, [r0]
	bl	fprintf
	addw	r3, r7, #1092
	str	r3, [r7, #1060]
	add	r3, r7, #12
	ldr	r2, [r7, #1060]
	ldr	r1, [r7, #1088]
	ldr	r0, [r3]
	bl	vfprintf
	add	r3, r7, #16
	mov	r0, r3
	mov	r3, #1024
	mov	r2, r3
	movs	r1, #0
	bl	memset
	add	r3, r7, #16
	mov	r2, r3
	ldr	r1, [r7, #1084]
	ldr	r0, [r7, #1080]
	bl	getBufString
	add	r2, r7, #16
	add	r3, r7, #12
	movw	r1, #:lower16:.LC10
	movt	r1, #:upper16:.LC10
	ldr	r0, [r3]
	bl	fprintf
	add	r3, r7, #12
	ldr	r0, [r3]
	bl	fflush
	movw	r3, #:lower16:stdout
	movt	r3, #:upper16:stdout
	ldr	r3, [r3]
	add	r2, r7, #12
	ldr	r2, [r2]
	cmp	r2, r3
	beq	.L30
	movw	r3, #:lower16:stderr
	movt	r3, #:upper16:stderr
	ldr	r3, [r3]
	add	r2, r7, #12
	ldr	r2, [r2]
	cmp	r2, r3
	beq	.L30
	add	r3, r7, #12
	ldr	r0, [r3]
	bl	fclose
.L30:
	addw	r7, r7, #1068
	mov	sp, r7
	@ sp needed
	pop	{r4, r7, pc}
	.size	debugBufFormat2fp, .-debugBufFormat2fp
	.ident	"GCC: (Linaro GCC 4.9-2016.02) 4.9.4 20151028 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
