# Logic Analyzer Connection (CHx) to NUCLEO-C031C6 board:

              CN7                                       CN10

           PD0    PD1                                PD3    NC
           PD2    PD2                                PB8    NC
           VDD    E5V                                PB0    NC
           PA14   GND -- GND                         AVDD   5V-USB-CHG
           NC     NC                                 GND    PA3
           NC     IOREF                       CH8 -- PA5    PC15
           PA13   NRST                        CH3--  PA6    PB12
           NC     3V3                         CH2 -- PA7    PB12
           PC6    +5V                                PB0    PB2
           GND    GND                         CH7 -- PC7    GND
           NC     GND                                PA9    PF3
    CH1 -- PC13   VIN                                PA15   PA8
           PC14   NC                                 PB5    PB15
           NC     PA0 -- CH6                         PB4    PB14
           PF0    PA1 -- CH5                         PB10   PB13
           PF1    PA4 -- CH4                         PB3    AGND
           NC     PB1                                PA10   PB0
           PB11   PA11/PB9                           PB6    NC
           PA2    PA12/PB8                           PB7    NC

# Example Logic Analyzer Traces

<p align="center">
<img src="../traces/qv.png"/><br>
<b>Logic analyzer trace after pressing the button (non-preemptive QV kernel)</b>
</p>
<p align="center">
<img src="../traces/qk.png"/><br>
<b>Logic analyzer trace after pressing the button (preemptive QK kernel)</b>
</p>
<p align="center">
<img src="../traces/freertos.png"/><br>
<b>Logic analyzer trace after pressing the button (preemptive FreeRTOS kernel)</b>
</p>
