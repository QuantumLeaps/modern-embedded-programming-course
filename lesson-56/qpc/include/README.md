# Available QP/C-Extras
The open source GPL distribution of QP/C can be augmented by the
[QP/C Extras][Extras], which provide more advanced QP/C features, such as:

- QS software tracing component (QP Spy);
- QXK real-time kernel component;
- Static-analysis configuration and automation scripts for the PC-Lint-Plus static analysis tool;
- Test suite (for the QUTest trace-based test harness?) that demonstrates
  100% lines of code and 100% MC/DC code coverage for QP/C.

# Licensing and Getting QP/C Extras
The QP/C Extras are [licensed commercially][Lic] and available to the commercial
licensees with the active Support Term.

> **NOTE:**
If you already are a commercial licensee, please [contact Quantum Leaps technical support][Sup]
to obtain the [QP/C Extras][Extras] matching the indicated public QP/C version.

> **NOTE:**
The [QP/C Extras][Extras] are also available for evaluation ([upon request][Req]).

# QP/C Extras Directories & Files
QP/C Extras consist of the following directories and files:

```
qpc/             // qpc installation directory
|
+---include/
|       qs.h     // QS target-resident software tracing public header file
|       qs_pkg.h // QS target-resident software tracing package-scope header
|       qxk.h    // QXK dual-mode real-time kernel header file
|
+---src/
|   |
|   +---qs/      // QS target-resident software tracing component implementation
|   |       qs.c
|   |       qs_64bit.c
|   |       qs_fp.c
|   |       qs_rx.c
|   |       qutest.c
|   |
|   \---qxk/      // QXK dual-mode component implementation
|           qxk.c
|           qxk_mutex.c
|           qxk_sema.c
|           qxk_xthr.c
|
+---ports/
|   +---arm-cm/
|   |   +---qxk/
|   |   |   +---armclang/
|   |   |   |       qxk_port.c
|   |   |   |       qp_port.h
|   |   |   |       qs_port.h
|   |   |   |
|   |   |   +---gnu/
|   |   |   |       qxk_port.c
|   |   |   |       qp_port.h
|   |   |   |       qs_port.h
|   |   |   |
|   |   |   \---iar/
|   |   |           qxk_port.c
|   |   |           qp_port.h
|   |   |           qs_port.h
|   |   |
|   |   \---qutest/
|   |           qp_port.h
|   |           qs_port.h
|   |
|   +---posix-qutest/
|   |       qp_port.h
|   |       qs_port.h
|   |       qutest_port.c
|   |       safe_std.h
|   |
|   \---win32-qutest/
|           qp_port.h
|           qs_port.h
|           qutest_port.c
|           safe_std.h
|
+---static-analysis/
|   |
|   \---pclp
|       |   au-barr.lnt      // BARR-C:2018 stylistic guidelines for C code
|       |   au-ds.lnt        // Dan Saks' recommendations for placing 'const' and 'volatile'
|       |   au-misra5.lnt    // PCLP MISRA-C:2025 (MC5) confiuration for rules checking
|       |   co-gnu_arm.bat   // script to generate configuration for GNU-ARM toolset
|       |   co-gnu_arm.h     // generated PCLP header file for GNU-ARM (14.3 at this time)
|       |   co-gnu_arm.lnt   // generated PCLP configuration file for GNU-ARM (14.3 at this time)
|       |   co-mingw32.bat   // script to generate configuration for MinGW32 toolset
|       |   co-mingw32.h     // generated PCLP header file for MinGW32 (14.2 at this time)
|       |   co-mingw32.lnt   // generated PCLP configuration for MinGW32 (14.2 at this time)
|       |   make.bat         // script to run the PCLP analysis
|       |   metrics_file.py  // Python script to generate file metrics report from PCLP data
|       |   metrics_func.py  // Python script to generate function metrics report from PCLP data
|       |   options.lnt      // PCLP configuration with Deviation Records in QP source code
|       |   qpcpp.lnt        // PCLP configuration with Deviation Permits in QP applications
|       |   qp_config.h      // QP compile-time configuration used for "linting" QP source code
|       |   std.lnt          // PCLP standard configuration used for "linting" QP source code
|       |
|       +---qk/
|       |       qp_port.hpp  // "QK port" for "linting" QK kernel code
|       |       qs_port.hpp  // "QS port" for "linting" QK kernel code
|       |
|       +---qv/
|       |       qp_port.hpp  // "QV port" for "linting" QV kernel code
|       |       qs_port.hpp  // "QS port" for "linting" QV kernel code
|       |
|       \---qxk/
|               qp_port.hpp   // "QXK port" for "linting" QXK kernel code
|               qs_port.hpp   // "QS port" for "linting" QXK kernel code
|
\---tests/
    |
    |   // unit tests (TUN) (host and embedded target)
    +---TUN_QP_qep_hsm/       // unit tests (TUN) for the qep_hsm module
    |   +---src/              // code for exercising the module under test
    |   |       tstsm.c       // QM-generated state machine implementation
    |   |       tstsm.h       // QM-generated state machine interface
    |   |       tstsm.qm      // QM model to generate code
    |   |       tstsm_sf.c    // QM-generated state machines for Safety Functions
    |   |       tstsm_sf.h    // QM-generated state machines interface
    |   \---test/
    |   |       Makefile          // makefile to build & run tests on the host
    |   |       nucleo-u545re.mak // makefile to build & run tests on NUCLEO-U545
    |   |       test_fixture.c    // QUTest fixture
    |   |       TUN_QP_qep_hsm.py    // QUTest script for main functionality
    |   |       TUN_QP_qep_hsm_sf.py // QUTest script for Safety Functions
    |   |
    +---TUN_QP_qep_msm/       // unit tests (TUN) for the qep_msm module
    |      ...
    +---TUN_QP_qf_act/        // unit tests (TUN) for the qf_act module
    |      ...
    +---TUN_QP_qf_actq/       // unit tests (TUN) for the qf_act module
    |      ...
    +---TUN_QP_qf_defer/      // unit tests (TUN) for the qf_act module
    |      ...
    +---TUN_QP_qf_dyn/        // unit tests (TUN) for the qf_act module
    |      ...
    +---TUN_QP_qf_mem/        // unit tests (TUN) for the qf_act module
    |      ...
    +---TUN_QP_qf_ps/         // unit tests (TUN) for the qf_act module
    |      ...
    +---TUN_QP_qf_qact32/     // unit tests (TUN) for the qf_act module
    |      ...
    +---TUN_QP_qf_qact64/     // unit tests (TUN) for the qf_act module
    |      ...
    +---TUN_QP_qf_qeq/        // unit tests (TUN) for the qf_act module
    |      ...
    +---TUN_QP_qf_qmact/      // unit tests (TUN) for the qf_act module
    |      ...
    +---TUN_QP_qf_time/       // unit tests (TUN) for the qf_act module
    |      ...
    +---TUN_QP_qutest/        // unit tests (TUN) for the qf_act module
    |      ...
    |
    |   // integration tests (TIN) (embedded target only)
    +---TIN_QP_mem/           // integration tests (TIN) for memory protection
    |   \---test_mpu/
    +---TIN_QP_qk/            // integration tests (TIN) for QK kernel
    |   \---test_sched/
    +---TIN_QP_qv/            // integration tests (TIN) for QV kernel
    |   \---test_sched/
    +---TIN_QP_qxk/           // integration tests (TIN) for QXK kernel
        +---test_mutex/
        \---test_sched/
```

   [Extras]: <https://www.state-machine.com/qpc/gs_extras.html>
   [Lic]:    <https://www.state-machine.com/licensing>
   [Req]:    <https://www.state-machine.com/licensing#RequestForm>
   [Sup]:    <mailto:support@state-machine.com>
