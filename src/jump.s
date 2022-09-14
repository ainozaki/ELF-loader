.global jump_start
jump_start:
    sub	sp, sp, #0x20
    str	x0, [sp, #24]
    str	x1, [sp, #16]
    str	x2, [sp, #8]
    ldr	x0, [sp, #24]
    ldr	x1, [sp, #8]
    mov	x2, sp
    mov	sp, x0
    blr	x1
    nop
    add	sp, sp, #0x20
    ret