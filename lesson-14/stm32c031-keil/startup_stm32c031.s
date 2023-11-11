; startup code for STM32C031 MCU

; NOTE: this is an INCOMPLETE startup code that does not work yet.
; It does not yet implement the Reset_Handler.
;
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     0x20004000  ; Top of Stack
                DCD     0x9         ; Reset Handler <== NOT implemented yet

__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

                END
