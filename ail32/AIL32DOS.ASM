

                OPTION SCOPED           ;Enable local labels
                .386                    ;Enable 386 instruction set

                IFDEF ZORTECH

                .MODEL SMALL,C          ;Small 32-bit memory model and C calls
DGROUP          group _TEXT,_DATA       ;(Zortech's BLINK needs this)

                ELSE

                .MODEL FLAT,C

                ENDIF

                 ;
                ;Local data
                ;

                .DATA

HOOK_INT        equ 8                   ;timer tick interrupt

                INCLUDE 386.mac         ;DOS extender macros
                INCLUDE AIL32_INTERNAL.inc

BIOS_H          equ 16                  ;Handle to BIOS default timer
bios_cb         dd 0
bios_cb_cs      dw 0  
bios_cb_real    dd 0

                .TEXT

;*****************************************************************************
hook_timer_process PROC C \             ;Take over default BIOS INT 8 handler
                USES ebx esi edi

                pushfd
                cli

                mov eax,HOOK_INT         ;get current INT 8 vector and save it
                GET_VECT                 ;as reserved timer function (stopped)
                mov bios_cb,ebx      
                mov bios_cb_cs,dx
                mov bios_cb_real,ecx

                mov ebx,OFFSET bios_caller
                mov timer_callback[BIOS_H*4],ebx

                mov eax,HOOK_INT         ;replace default handler with API task
                mov edx,OFFSET API_timer ;manager
                mov bx,cs
                SET_VECT

                POP_F
                ret
hook_timer_process ENDP

;*****************************************************************************
unhook_timer_process PROC C \           ;Restore default BIOS INT 8 handler
                USES ebx esi edi

                pushfd
                cli

                mov current_timer,-1    ;disallow any further callbacks

                mov eax,HOOK_INT
                mov ebx,bios_cb_real
                SET_REAL_VECT

                mov eax,HOOK_INT
                mov edx,bios_cb
                movzx ebx,bios_cb_cs
                SET_PROT_VECT

                POP_F
                ret
unhook_timer_process ENDP
