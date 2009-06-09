/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#define mymemset(where, what, howmany) asm("\n\tcld\n\trep\n\tstosl\n" : : "D" (where), "a" (what), "c" (howmany))

#define ASM_DECODE_SC(z) "                    \n\
    movl (%0,%1),%10                          \n\
    movd 4(%0,%1),%%mm0                       \n\
    sarl $1,%10                               \n\
    movd %10,%%mm1                            \n\
    punpcklbw %%mm1,%%mm0                     \n\
    jnc DecodeSC" #z "                        \n\
    psraw $8,%%mm0                            \n\
    subl $4,%1                                \n\
DecodeSC" #z ":                               \n\
    paddw %%mm0,%%mm3                         \n\
    addl $8,%1                                \n\
    paddw %%mm3,%%mm2                         \n\
"

#define ASM_DECODE_SU(z) "                    \n\
    movq (%0,%1),%%mm3                        \n\
    addl $8,%1                                \n\
    movq %%mm3,%%mm1                          \n\
    psubsw %%mm2,%%mm3                        \n\
    movq %%mm1,%%mm2                          \n\
"

#define ASM_DECODE_MC(z) "                    \n\
    movswl (%0,%1),%10                        \n\
    movd (%0,%1),%%mm0                        \n\
    sarl $1,%10                               \n\
    psrlq $16,%%mm0                           \n\
    movd %10,%%mm1                            \n\
    punpcklbw %%mm1,%%mm0                     \n\
    jnc DecodeMC" #z "                        \n\
    psraw $8,%%mm0                            \n\
    subl $2,%1                                \n\
DecodeMC" #z ":                               \n\
    punpcklwd %%mm0,%%mm0                     \n\
    paddw %%mm0,%%mm3                         \n\
    addl $4,%1                                \n\
    paddw %%mm3,%%mm2                         \n\
"

#define ASM_DECODE_MU(z) "                    \n\
    movd (%0,%1),%%mm3                        \n\
    punpcklwd %%mm3,%%mm3                     \n\
    addl $4,%1                                \n\
    movq %%mm3,%%mm1                          \n\
    psubsw %%mm2,%%mm3                        \n\
    movq %%mm1,%%mm2                          \n\
"

#define ASM_LOOP_VYFY "                       \n\
    movq %%mm2,%%mm1                          \n\
    movq %%mm1,%%mm0                          \n\
    punpckhwd %%mm1,%%mm1                     \n\
    punpcklwd %%mm0,%%mm0                     \n\
    pmaddwd %%mm4,%%mm1                       \n\
    pmaddwd %%mm4,%%mm0                       \n\
    psrad %%mm7,%%mm1                         \n\
    psrad %%mm7,%%mm0                         \n\
    movq %%mm1,%%mm6                          \n\
    paddd  (%12,%11),%%mm0                    \n\
    psrlq $16,%%mm5                           \n\
    movq %%mm0, (%12,%11)                     \n\
    punpckldq %%mm5,%%mm5                     \n\
    paddd 8(%12,%11),%%mm1                    \n\
    paddsw %%mm5,%%mm4                        \n\
    movq %%mm1,8(%12,%11)                     \n\
    movq %%mm4,%%mm0                          \n\
    pxor %%mm1,%%mm1                          \n\
    addl $16,%11                              \n\
    pcmpgtw %%mm1,%%mm4                       \n\
    testl %11,%1                              \n\
    pandn %%mm0,%%mm4                         \n\
"

#define ASM_LOOP_VYFN "                       \n\
    movq %%mm2,%%mm1                          \n\
    movq %%mm1,%%mm0                          \n\
    punpckhwd %%mm1,%%mm1                     \n\
    punpcklwd %%mm0,%%mm0                     \n\
    pmaddwd %%mm4,%%mm1                       \n\
    pmaddwd %%mm4,%%mm0                       \n\
    psrad %%mm7,%%mm1                         \n\
    psrad %%mm7,%%mm0                         \n\
    movq %%mm1,%%mm6                          \n\
    paddd  (%12,%11),%%mm0                    \n\
    paddd 8(%12,%11),%%mm1                    \n\
    movq %%mm0, (%12,%11)                     \n\
    movq %%mm1,8(%12,%11)                     \n\
    addl $16,%11                              \n\
    testl %11,%1                              \n\
"

#define ASM_LOOP_VNFN "                       \n\
    pxor %%mm6,%%mm6                          \n\
    pxor %%mm1,%%mm1                          \n\
    psubd %%mm2,%%mm6                         \n\
    pxor %%mm0,%%mm0                          \n\
    punpckhwd %%mm6,%%mm1                     \n\
    punpcklwd %%mm6,%%mm0                     \n\
    psrad %%mm7,%%mm1                         \n\
    psrad %%mm7,%%mm0                         \n\
    movq %%mm1,%%mm6                          \n\
    paddd  (%12,%11),%%mm0                    \n\
    paddd 8(%12,%11),%%mm1                    \n\
    movq %%mm0, (%12,%11)                     \n\
    movq %%mm1,8(%12,%11)                     \n\
    addl $16,%11                              \n\
    testl %11,%1                              \n\
"

#define ASM_LOOP(q,z) "                       \n\
MyLoop" #z #q ":                              \n\
    " ASM_DECODE_##z(q) ASM_LOOP_##q "        \n\
    js MyLoop" #z #q "                        \n\
    orl %1,%1                                 \n\
    js MyDone                                 \n\
    jz NotOverflow" #z #q "                   \n\
    movl $8,%10                               \n\
    subl %10,%11                              \n\
    andl %11,%10                              \n\
    movq (%12,%11),%%mm0                      \n\
    movl %10,%6                               \n\
    psubd %%mm6,%%mm0                         \n\
    movq %%mm0,(%12,%11)                      \n\
NotOverflow" #z #q ":                         \n\
    testl $2,%5                               \n\
    jnz MyStage2                              \n\
    movl $1,%5                                \n\
    movl %14,%0                               \n\
    movl %15,%1                               \n\
    movl %16,%10                              \n\
    movq %17,%%mm2                            \n\
    movq %18,%%mm3                            \n\
    orl %11,%11                               \n\
    movl %10,%4                               \n\
    js NewSampleType" #q "                    \n\
    jmp MyDone                                \n\
"

#define ASM_CORE(q) "                         \n\
NewSampleType" #q ":                          \n\
    cmpl $2,%4                                \n\
    jz MyLoopSC" #q "                         \n\
    jnc MyLoopSU" #q "                        \n\
    jpo MyLoopMC" #q "                        \n\
    " ASM_LOOP(q,MU) "                        \n\
    " ASM_LOOP(q,SC) "                        \n\
    " ASM_LOOP(q,SU) "                        \n\
    " ASM_LOOP(q,MC) "                        \n\
"
