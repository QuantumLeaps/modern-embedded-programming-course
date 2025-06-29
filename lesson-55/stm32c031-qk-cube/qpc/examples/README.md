## Brought to you by:
[![Quantum Leaps](https://www.state-machine.com/attachments/logo_ql_400.png)](https://www.state-machine.com)
<hr>

# About qpc-examples
This repository contains the application examples for the QP/C
Real-Time Embedded Framework (see https://github.com/QuantumLeaps/qpc ).

This repository can be used in several ways:
- [Standalone examples](#standalone-examples-in-the-cmsis-packs-folder)
  that don't require the [qpc framework repo](https://github.com/QuantumLeaps/qpc).
  (These examples rely on the qpc framework included from the
  [CMIS-packs](https://github.com/QuantumLeaps/cmsis-packs))

- [As a sub-module to the qpc repo](#as-a-sub-module-to-the-qpc-repository)

- [Examples for the QView monitoring system](#examples-for-qview-monitoring-system)

- [Examples for the QUTest testing system](#examples-for-qtest-testing-system)


## Standalone (examples in the `cmsis-packs` folder)
```
+---cmsis-packs    <=== standalone examples
|   +---dpp_nucleo-c031c6-qk-gcc
|   +---dpp_nucleo-c031c6-qk-keil
|   +---dpp_nucleo-c031c6-qk-stm32cube
|   \---dpp_nucleo-u545re-qk-stm32cube
|
```

## As a sub-module to the qpc repository
```
+---arm-cm
|   +---blinky_ek-tm4c123gxl
|   +---blinky_nucleo-c031c6
|   +---dpp_efm32-slstk3401a
|   |   +---qk
|   |   |   +---armclang
|   |   |   +---gnu
|   |   |   \---iar
|   |   +---qv
|   |   |   +---armclang
|   |   |   +---gnu
|   |   |   \---iar
|   |   +---qview
|   |   |   \---img
|   |   \---qxk
|   |       +---armclang
|   |       +---gnu
|   |       \---iar
|   +---dpp_ek-tm4c123gxl
|   +---dpp_ek-tm4c123gxl_mpu
|   +---dpp_mbed-lpc1768
|   +---dpp_nucleo-c031c6
|   +---dpp_nucleo-c031c6_mpu
|   +---dpp_nucleo-h743zi
|   +---dpp_nucleo-l053r8
|   +---dpp_nucleo-l152re
|   +---dpp_nucleo-l552ze
|   +---dpp_nucleo-u545re
|   +---dpp_stm32f4-discovery
|   +---game_efm32-slstk3401a
|   +---low-power_ek-tm4c123gxl
|   \---real-time_nucleo-l053r8
|
+---arm-cr
|   +---blinky_launchxl2-tms57012
|   \---dpp_launchxl2-tms57012
|
+---embos
|   \---arm-cm
|       \---dpp_nucleo-h743zi
+---emwin
|   +---demo_no_wm
|   \---demo_with_wm
|
+---freertos
|   \---arm-cm
|       +---dpp_ek-tm4c123gxl
|       \---dpp_nucleo-h743zi
|
+---lwip
|   \---arm-cm
|       \---lwip_ek-lm3s6965
|
+---msp430
|   +---blinky_msp-exp430f5529lp
|   +---blinky_msp-exp430g2
|   \---dpp_msp-exp430f5529lp
|
+---posix-win32   <== examples for the host computer
|   +---blinky
|   +---calc
|   +---calc2
|   +---calc_sub
|   +---comp
|   +---defer
|   +---dpp
|   +---dpp-posix
|   +---dpp_comp
|   +---history_qhsm
|   +---history_qmsm
|   +---qhsmtst
|   +---qmsmtst
|   +---reminder
|   \---reminder2
|
+---threadx
|   \---arm-cm
|       +---dpp_ek-tm4c123gxl
|       \---dpp_stm32f429-discovery
+---uc-os2
|   \---arm-cm
|       \---dpp_ek-tm4c123gxl
+---zephyr
|   +---blinky
|   \---dpp
|
+---qutest    <== examples for QUTest testing harness
|   +---blinky
|   +---dpp
|   +---dpp-comp
|   +---evt_par
|   +---qhsmtst
|   +---qmsmtst
|   +---self_test
|   +---start_seq
|   +---unity_basic
|   +---unity_ledbar
|   +---unity_ledbar2
|   +---unity_mock
|   \---unity_strlen
|       +---qutest
|       +---src
|       \---test
|
+---qview-dpp   <== examples for QView monitoring (DPP example)
|   +---img
|       dpp.py
|       dpp1.py
|       qview-dpp.lnk
|       qview-dpp.lnk
|
+---qwin-gui   <== examples for the QWin GUI
|   +---dpp-gui
|   \---game-gui
|
```

## Examples for QTest Testing System
The "qutest" Python package  (included in the
[QTools collection](https://github.com/QuantumLeaps/qtools))
is a powerful [testing system](https://www.state-machine.com/qtools/qutest.html).
It has been specifically designed for deeply embedded systems, but also supports
unit and integration testing of embedded code on host computers ("dual targeting").
QUTest is the fundamental tooling for Test-Driven Development (TDD) and Behavior-
Driven Development (BDD) of QP/C/C++ applications , which is a highly recommended
best-practice.

The following screen shot show a typical QUTest run with the QSPY ouptut
in the background.

![](qutest/img/qutest-dpp.webp)


## Examples for QView Monitoring System
The "qview" Python package (included in the
[QTools collection](https://github.com/QuantumLeaps/qtools))
is a powerful
[Visualization and Monitoring system](https://www.state-machine.com/qtools/qview.html),
which allows embedded developers to create virtual Graphical User Interfaces
in Python to monitor and control their embedded devices from a host (desktop)
computer. The interfaces created by QView can visualize the data produced
by [QP/Spy software tracing system](https://www.state-machine.com/qtools/qpspy.html)
and can also interact with the embedded target by sending various commands.

> **NOTE** You need to **install** the `qview` package in your Python setup.<br>
`pip install qview`

The python scripts `qview-dpp/dpp.py` and `qview-dpp/dpp1.py` will allow
monitoring of all DPP examples running on any supported board (or on the
host computer). Here is how you can launch the QView monitoring for DPP:

```
pythonw dpp.py

pythonw dpp1.py
```

The following screen shot shows QView for the DPP (Dining Philosophers Problem)
application with the QSPY ouptut in the background.

![](qview-dpp/img/qview-dpp.webp)

