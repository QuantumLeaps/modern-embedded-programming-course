# DPP example for Zephyr

<p align="center">
<img src="qp-zephyr.webp"/>
</p>

# Overview
Dining Philosophers Problem (DPP) example with multiple Active Objects,
each with its own state machine.

# Setting Up Zephyr Environment (Linux)
Open a terminal in the directory of this example (`qpc/examples/zephry/dpp`).

Depending on your Zephyr installation, you might need to shell the scripts
to configure Zephyr environment:

```
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
```

# Setting Up Zephyr Environment (Windows)
Open a command prompt in the directory of this example (`qpc\examples\zephry\dpp`).

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
Philo[0]->thinking
Philo[1]->thinking
Philo[2]->thinking
Philo[3]->thinking
Philo[4]->thinking
QF_onStartup
Philo[2]->hungry
Philo[2]->eating
Philo[4]->hungry
Philo[4]->eating
Philo[1]->hungry
Philo[0]->hungry
Philo[3]->hungry
```

# Activating QSPY Software Tracing
The QP/C Zephyr Module supports the
[QSPY Software Tracing](https://www.state-machine.com/qtools/qpspy.html)
option and will add the appropriate macros and files to build the "QSPY"
configuration.

If you wish to enable "QSPY" you can provide the option "QSPY"
in the command-line for the build. For example:

```bash
west build -b <board> -- -DQSPY=ON
```
