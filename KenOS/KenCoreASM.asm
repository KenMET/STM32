;/*******************************************************************************
;  * @author  Ken
;  * @version V1.0.0
;  * @date    06-05-2020
;*******************************************************************************/
	
;		IMPORT  OSRunning               ; External references
;        IMPORT  OSPrioCur
;        IMPORT  OSPrioHighRdy
;        IMPORT  OSTCBCur
;        IMPORT  OSTCBHighRdy
;        IMPORT  OSIntNesting
;        IMPORT  OSIntExit
;        IMPORT  OSTaskSwHook
           
;        EXPORT  OSStartHighRdy               
;        EXPORT  OSCtxSw
;        EXPORT  OSIntCtxSw
;		EXPORT  OS_CPU_SR_Save                                      ; Functions declared in this file
;    	EXPORT  OS_CPU_SR_Restore       
;        EXPORT  PendSV_Handler

	IMPORT OSInterruptSum
	IMPORT OSRunning
	IMPORT p_OSTCBCur
	IMPORT p_OSTCBHighRdy
	IMPORT OSTaskSwHook
	IMPORT OSPrioCur
	IMPORT OSPrioHighRdy
 
	EXPORT OS_ENTER_CRITICAL
	EXPORT OS_EXIT_CRITICAL
	EXPORT OSStartHighRdy
	EXPORT OSCtxSw
	EXPORT OSIntCtxSw
	EXPORT PendSV_Handler	
     
NVIC_INT_CTRL   	EQU     0xE000ED04  ; 中断控制寄存器
NVIC_SYSPRI2    	EQU     0xE000ED20  ; 系统优先级寄存器(2)
NVIC_PENDSV_PRI 	EQU     0xFFFF0000  ; PendSV中断和系统节拍中断
                                        ; (都为最低，0xff).
NVIC_PENDSVSET  	EQU     0x10000000  ; 触发软件中断的值.

		PRESERVE8 
		
		AREA    |.text|, CODE, READONLY
        THUMB 

OS_CPU_SR_Save
    MRS     R0, PRIMASK  		;读取PRIMASK到R0,R0为返回值 
    CPSID   I					;PRIMASK=1,关中断(NMI和硬件FAULT可以响应)
    BX      LR			    	;返回

OS_CPU_SR_Restore
    MSR     PRIMASK, R0	   		;读取R0到PRIMASK中,R0为参数
    BX      LR					;返回



;/***************************************************************************************
;* 函数名称: OS_ENTER_CRITICAL
;* 功能描述: 进入临界区          
;* 参    数: None
;* 返 回 值: None
;/***************************************************************************************
OS_ENTER_CRITICAL
		CPSID   I                       ; Disable all the interrupts                                                         
		PUSH 	{R1,R2}      
		LDR 	R1, =OSInterruptSum	    ; OSInterrputSum++
        LDRB 	R2, [R1]
        ADD   	R2, R2, #1
        STRB 	R2, [R1]
		POP     {R1,R2}
  		BX LR


;/***************************************************************************************
;* 函数名称: OS_EXIT_CRITICAL
;* 功能描述: 退出临界区          
;* 参    数: None
;* 返 回 值: None
;*****************************************************************************************/
OS_EXIT_CRITICAL
		PUSH    {R1, R2}
		LDR     R1, =OSInterruptSum     ; OSInterrputSum--
        LDRB    R2, [R1]
        SUB     R2, R2, #1
        STRB    R2, [R1]
		MOV     R1,  #0	      
		CMP     R2,  #0			        ; if OSInterrputSum=0,enable 
                                        ; interrupts如果OSInterrputSum=0，
		;MSREQ  PRIMASK, R1   
        CPSIE   I
	    POP   	{R1, R2}
		BX LR

;/**************************************************************************************
;* 函数名称: OSStartHighRdy
;* 功能描述: 使用调度器运行第一个任务
;* 参    数: None
;* 返 回 值: None
;**************************************************************************************/  

OSStartHighRdy
        LDR     R4, =NVIC_SYSPRI2      ; set the PendSV exception priority
        LDR     R5, =NVIC_PENDSV_PRI
        STR     R5, [R4]

        MOV     R4, #0                 ; set the PSP to 0 for initial context switch call
        MSR     PSP, R4

        LDR     R4, =OSRunning         ; OSRunning = TRUE
        MOV     R5, #1
        STRB    R5, [R4]

                                       ;切换到最高优先级的任务
        LDR     R4, =NVIC_INT_CTRL     ;rigger the PendSV exception (causes context switch)
        LDR     R5, =NVIC_PENDSVSET
        STR     R5, [R4]

        CPSIE   I                      ;enable interrupts at processor level
OSStartHang
        B       OSStartHang            ;should never get here

;/**************************************************************************************
;* 函数名称: OSCtxSw
;* 功能描述: 任务级上下文切换         
;* 参    数: None
;* 返 回 值: None
;***************************************************************************************/
  
OSCtxSw
		PUSH    {R4, R5}
        LDR     R4, =NVIC_INT_CTRL  	;触发PendSV异常 (causes context switch)
        LDR     R5, =NVIC_PENDSVSET
        STR     R5, [R4]
		POP     {R4, R5}
        BX      LR

;/**************************************************************************************
;* 函数名称: OSIntCtxSw
;* 功能描述: 中断级任务切换
;* 参    数: None
;* 返 回 值: None
;***************************************************************************************/

OSIntCtxSw
		PUSH    {R4, R5}
        LDR     R4, =NVIC_INT_CTRL      ;触发PendSV异常 (causes context switch)
        LDR     R5, =NVIC_PENDSVSET
        STR     R5, [R4]
		POP     {R4, R5}
        BX      LR
        NOP

;/**************************************************************************************
;* 函数名称: OSPendSV
;* 功能描述: OSPendSV is used to cause a context switch.
;* 参    数: None
;* 返 回 值: None
;***************************************************************************************/

PendSV_Handler
    CPSID   I                                                   ; Prevent interruption during context switch
    MRS     R0, PSP                                             ; PSP is process stack pointer 如果在用PSP堆栈,则可以忽略保存寄存器,参考CM3权威中的双堆栈-白菜注
    CBZ     R0, PendSV_Handler_Nosave		                    ; Skip register save the first time

    SUBS    R0, R0, #0x20                                       ; Save remaining regs r4-11 on process stack
    STM     R0, {R4-R11}

    LDR     R1, =p_OSTCBCur                                       ; p_OSTCBCur->OSTCBStkPtr = SP;
    LDR     R1, [R1]
    STR     R0, [R1]                                            ; R0 is SP of process being switched out

                                                                ; At this point, entire context of process has been saved
PendSV_Handler_Nosave
    PUSH    {R14}                                               ; Save LR exc_return value
    LDR     R0, =OSTaskSwHook                                   ; OSTaskSwHook();
    BLX     R0
    POP     {R14}

    LDR     R0, =OSPrioCur                                      ; OSPrioCur = OSPrioHighRdy;
    LDR     R1, =OSPrioHighRdy
    LDRB    R2, [R1]
    STRB    R2, [R0]

    LDR     R0, =p_OSTCBCur                                       ; p_OSTCBCur  = p_OSTCBHighRdy;
    LDR     R1, =p_OSTCBHighRdy
    LDR     R2, [R1]
    STR     R2, [R0]

    LDR     R0, [R2]                                            ; R0 is new process SP; SP = p_OSTCBHighRdy->OSTCBStkPtr;
    LDM     R0, {R4-R11}                                        ; Restore r4-11 from new process stack
    ADDS    R0, R0, #0x20
    MSR     PSP, R0                                             ; Load PSP with new process SP
    ORR     LR, LR, #0x04                                       ; Ensure exception return uses process stack
    CPSIE   I
    BX      LR                                                  ; Exception return will restore remaining context
	NOP
	END
;SaveSPToCurTcb 					; 保存当前任务的堆顶指针到它的
;	ldr r4,=p_OSTCBCur 			; 取出当前任务的 PCB 地址
;	ldr r5,[r4]
;	str sp,[r5] 				; 保存当前任务的堆顶指针到它的
;								; TCB(因为 TaskCtrBlock 地址亦即
;								; OSTCBStkPtr 的地址) 
;
;OS_TASK_SW 						; 任务级的任务切换
;	stmfd sp!,{lr} 				; PC 入栈
;	stmfd sp!,{r0-r12,lr} 		; r0-r12，lr 入栈
;	PUAH_PSR
;	mrs r4,cpsr
;	stmfd sp!,{r4} 				; cpsr 入栈
;	SaveSPToCurTcb 				; 保存当前任务的堆顶指针到它的 TCB.
;								; TCB[OSPrioCur].OSTCBStkPtr = SP;
;	ldr r4,=p_OSTCBCur 			; 取出当前任务的 PCB 地址
;	ldr r5,[r4]
;	str sp,[r5] 				; 保存当前任务的堆顶指针到它的 TCB(因为
;								;TaskCtrBlock 地址亦即 OSTCBStkPtr 的地址)

;GetHigTcbSP 					; 取出更高优先级任务的堆顶指针到 SP ,
;								; SP = TCB[OSPrioCur].OSTCBStkPtr
;	ldr r6,=p_OSTCBHighRdy 		; 取出更高优先级就绪任务的 PCB 的地址
;	ldr r6,[r6]
;	ldr sp,[r6] 				; 取出更高优先级任务的堆顶指针到 SP
;	b POP_ALL 					; 根据设定的栈结构顺序出栈

;OSStartHighRdy
;	ldr r4,=p_OSTCBHighRdy 		; 取出最高优先级就绪任务的 PCB 的地址
;	ldr r4,[r4] 				; 取得任务的栈顶指针(因为 TaskCtrBlock 地址
;								; 亦即 OSTCBStkPtr 的地址)
;	ldr sp,[r4] 				; 任务的栈顶指针赋给 SP
;	b POP_ALL 					; 根据设定的栈结构顺序出栈
;
;POP_ALL							; 根据设定的栈结构顺序出栈
;	ldmfd sp!,{r4} 				; psr 出栈
;	msr CPSR_cxsf,r4
;	ldmfd sp!,{r0-r12,lr,pc} 	; r0-r12,lr,pc 出栈

;ASM_IRQHandler 					; 中断入口地址，在中断向量表初始化时被设置
;	sub lr,lr,#4 				; 计算中断返回地址
;	stmfd sp!,{r0-r3,r12,lr} 	; 保护现场,此时处于中断模式下，sp 指向中
;;								; 断模式下的堆栈.不能进行任务切换(各任务
;								; 堆栈处在管理模式堆栈).
;								; R4-R11装的是局部变量，在进行函数跳转时，
;								; 编译器它会自动保护它们，
;								; 即 C 语言函数返回时，寄存器 R4-R11、SP
;								; 不会改变，无需人为保护
;	bl OSIntEnter
;	bl C_IRQHandler 			; 调用 c 语言的中断处理程序
;	bl OSIntExit 				; 判断中断后是否有更高优先级的任务进入就
;								; 绪而需要进行任务切换
;	ldr r0,=OSIntCtxSwFlag 		;if(OSIntCtxSwFlag == 1) OSIntCtxSw()
;	ldr r1,[r0]
;	cmp r1,#1
;	beq OSIntCtxSw 				; 有更高优先级的任务进入了就绪状态，则跳
								; 转到 OSIntCtxSw 进行中断级的任务切换
;	ldmfd sp!,{r0-r3,r12,pc}^ 	; 不进行任务切换，出栈返回被中断的任务。
;								; 寄存器出栈同时将 spsr_irq 的值复制到
;								; CPSR，实现模式切换

;OSIntCtxSw 						; 把中断的返回地址保存到
;								; Int_Return_Addr_Save 变量中
;	ldr r0,=OSIntCtxSwFlag 		; 清 OSIntCtxSwFlag 标志位
;	mov r1,#0
;	str r1,[r0] 				; OSIntCtxSwFlag = 0
;	add sp,sp,#20 				; 调整SP,使之指向之前入栈的lr(中断模
;								; 式下的 lr 保存的是中断返回地址),
;	ldr r0,[sp] ; lr -> r0
;	ldr r1,=Int_Return_Addr_Save ; &Int_Return_Addr_Save -> r1
;	str r0,[r1] ; lr -> Int_Return_Addr_Save
;	ldr r0,=OS_TASK_SW_INT
;	str r0,[sp] 				; OS_TASK_SW_INT -> lr
;	sub sp,sp,#20 				; 调整 SP 回到栈顶
;	ldmfd sp!,{r0-r3,r12,pc}^ 	; 退出中断后跳到 OS_TASK_SW 而不返回

;OS_TASK_SW_INT 					; 中断级任务切换,中断服务时使用另外
;								; 的堆栈，所以要回到管理模式中才进行任务切换
;	sub sp,sp,#4 				; 为 PC 保留位置
;	stmfd sp!,{r0-r12,lr} 		; r0-r12，lr 入栈
;	ldr r0,=Int_Return_Addr_Save; 取出进入中断之前保存的 PC
;	ldr r0,[r0] 				; Int_Return_Addr_Save -> r0
;	add sp,sp,#56 				; 调整 SP
;	stmfd sp,{r0} 				; r0(PC)入栈
;	sub sp,sp,#56 				; 调整 SP 回到栈顶
;	b PUAH_PSR

;PUAH_PSR
;	mrs r4,cpsr
;	stmfd sp!,{r4} 				; cpsr 入栈

;SaveSPToCurTcb					; TCB[OSPrioCur].OSTCBStkPtr = SP;
;	ldr r4,=p_OSTCBCur 			; 取出当前任务的 PCB 地址
;	ldr r5,[r4]
;	str sp,[r5] 				; 保存当前任务的堆顶指针到它的 TCB(因为
;								;TaskCtrBlock 地址亦即 OSTCBStkPtr 的地址)

;GetHigTcbSP 					; 取出更高优先级任务的堆顶指针到 SP ,
;								; SP = TCB[OSPrioCur].OSTCBStkPtr
;	ldr r6,=p_OSTCBHighRdy 		; 取出更高优先级就绪任务的 PCB 的地址
;	ldr r6,[r6]
;	ldr sp,[r6] 				; 取出更高优先级任务的堆顶指针到 SP
;	b POP_ALL 					; 根据设定的栈结构顺序出栈
