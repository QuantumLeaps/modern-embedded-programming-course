[![QP Zephyr Module](./qp-zephyr.jpg)](https://www.state-machine.com/qpc/zephyr.html)

# About the QPC Zephyr Module
This directory defines the
[QP/C Real-Time Embedded Framework](https://github.com/QuantumLeaps/qpc)
as a [Zephyr module](https://docs.zephyrproject.org/latest/develop/modules.html).

# How to use
Example of use is provided in the related repository
[qpc-zephyr-app](https://github.com/QuantumLeaps/qpc-zephyr-app)

To create your own QP-Zephyr project, you can clone that repository
and customize it to your needs:

```bash
git clone https://github.com/QuantumLeaps/qpc-zephyr-app <my-project> --recurse-submodules --depth 1
```
where `<my-project>` is the name of your project.


## Configuring the QPC Zephyr Module
The `Kconfig` file provides configuration `CONFIG_QPC` to activate the QPC module
in Zephyr. To do so, you need to add the following line to your `prj.conf`:

```ini
CONFIG_QPC=y
```

## Option for Activating QSPY Software Tracing
The QP/C Zephyr Module supports the
[QSPY Software Tracing](https://www.state-machine.com/qtools/qpspy.html)
option and will add the appropriate macros and files to build the "QSPY"
configuration.

If you wish to enable "QSPY" you can provide the option "QSPY"
in the command-line for the build. For example:

```bash
west build -b nucleo_h743zi -- -DQSPY=ON
```
