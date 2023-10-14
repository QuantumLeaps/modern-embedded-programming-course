; startup code for TM4C MCU

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     0x20004000  ; Top of Stack
                DCD     0x9         ; Reset Handler

__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

                END
