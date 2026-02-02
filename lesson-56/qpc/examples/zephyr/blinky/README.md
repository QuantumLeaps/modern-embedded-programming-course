# Blinky example for Zephyr

<p align="center">
<img src="qp-zephyr.webp"/>
</p>

# Overview
Blinks an on-board LED once per second. The blinking is performed
by an Active Object (Blinky) with a state machine.


# Setting Up Zephyr Environment (Linux)
Open a terminal in the directory of this example (`qpc/examples/zephry/blinky`).

Depending on your Zephyr installation, you might need to shell the scripts
to configure Zephyr environment:

```
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
```

# Setting Up Zephyr Environment (Windows)
Open a command prompt in the directory of this example (`qpc\examples\zephry\blinky`).

Depending on your Zephyr installation, you might need to execute the batch
scripts to configure Zephyr environment:

```
%HOMEPATH%\zephyrproject\.venv\Scripts\activate
%HOMEPATH%\zephyrproject\zephyr\zephyr-env.cmd
```

# Building
To build the example on any host OS, type:
```
west build -b <board>
```
where `<board>` is one of the
[boards supported by Zephyr](https://docs.zephyrproject.org/latest/boards/index.html).
The `dpp` example has been tested with the following boards:

```
nucleo_c031c6
nucleo_f103rb
nucleo_l152re
nucleo_h743zi
```

To flash the board, type:
```
west flash
```

# Sample Output
The application also produces ASCII output to the serial terminal
(if suppoted by the board):

```
*** Booting Zephyr OS build v4.3.0-3316-ge0c706a7f5f7 ***
BSP_ledOff
QF_onStartup
BSP_ledOn
BSP_ledOff
BSP_ledOn
BSP_ledOff
```
