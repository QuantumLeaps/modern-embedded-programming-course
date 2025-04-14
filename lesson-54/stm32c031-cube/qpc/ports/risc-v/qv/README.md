# QP/C Port to RISC-V
This document describes the QP/C framework port to RISC-V with the cooperative QV kernel. The port is located in the `qpc/ports/risc-v/qv/gnu` directory.

```
qpc/                  - QP/C installation directory
+-ports/              - QP/C ports directory
| +-risc-v/           - QP/C port to RISC-V
| | +-qv/             - QP/C port to RISV-V with the QV kernel
| | | +-gnu/          - QP/C port to RISV-V with the QV kernel and GNU compiler
| | | | +-qep_port.h  - QEP/C (hierarchical event processor) port
| | | | +-qf_port.h   - QF/C  (event-driven framework) port
| | | | +-qs_port.h   - QS/C  (QP/Spy software tracing) port
| | | | +-qv_port.h   - QV/C  (cooperative QV kernel) port
```
> NOTE:
The `qpc/ports/risc-v/qv/qv/gnu` directory needs to be provided to the compiler's include path.

## Cooperative QV Kernel for RISC-V
The most important aspects of the QP/C port to RISC-V with the cooperative QV kernel are provided as macro definitions in the files `qf_port.h` and `qv_port.h` in the `qpc/ports/risc-v/qv/qv/gnu` directory. The port strives to be generic in that it's designed to support all currently available RISC-V (RV32) chips.

```
file: qf_port.h
~ ~ ~

/* The maximum number of active objects in the application, see NOTE1 */
#define QF_MAX_ACTIVE          32U

/* interrupt disabling policy, see NOTE2 */
#define QF_INT_DISABLE()       __asm__ volatile("csrc mstatus,8")
#define QF_INT_ENABLE()        __asm__ volatile("csrs mstatus,8")

/* QF critical section entry/exit, see NOTE3 */
#define QF_CRIT_STAT_TYPE      unsigned int
#define QF_CRIT_ENTRY(status_) do { \
    __asm__ volatile("csrr %0, mstatus" : "=r"(status_)); \
    QF_INT_DISABLE(); \
} while (false)
#define QF_CRIT_EXIT(status_)  \
    __asm__ volatile ("csrw mstatus, %0" :: "rK"(status_))

/* support for fast LOG2, see NOTE4 */
#define QF_LOG2(n_) ((uint_fast8_t)(32U - __builtin_clz((n_))))

/* NOP instruction for RISC-V, NOTE5 */
#define QF_CRIT_EXIT_NOP()     __asm__ volatile ("nop")


~ ~ `
```

**[1]** the maximum number of active objects `QF_MAX_ACTIVE` can be increased up to 64, if necessary. Here it is set to a lower level of 32 to slightly improve performance and save some RAM.

**[2]** the port disables interrupts by clearing `mstatus` bit-3 (MIE == Machine Interrupt Enable) and enables interrupts setting `mstatus` bit-3 (MIE).

**[3]** the port uses the "save and restore interrupt status" policy. This policy is appropriate for CPUs that disable interrupts inside ISRs, which is the case with RISC-V. Also, this policy allows *nesting* of critical sections.

**[4]** The port takes advantage of the built-in function `__builtin_clz()` for fast computation of log-base-2, which is used in the QV scheduler.

**[5]** The port uses `NOP` instruction to prevent potential merging of back-to-back critical sections inside the QP/C implementation. This precaution is to reduce interrupt latency.

```
file: qv_port.h
~ ~ ~

/*! macro to put the CPU to sleep inside QV_onIdle() callback */
/* The following QV_CPU_SLEEP() executes the WFI instruction with
* interrupts still DISABLED. Interrupts are enabled only after
* the RISC-V CPU wakes up.
* NOTE: this approach needs to be tested for each specific RISC-V SoC.
*/
#define QV_CPU_SLEEP() do { \
    __asm volatile("wfi");  \
    QF_INT_ENABLE();        \
} while (false)

~ ~ `
```

**[6]** The QV port can detect the idle condition of the system, which can be utilized to put the system (CPU and peripherals) in a low-power sleep mode. However, the specifics of the QV kernel require the CPU to transition to the sleep mode with interrupts **DISABLED**.

> **NOTE:**
According to the "The RISC-V Instruction Set Manual", the `WFI` instruction can be executed when interrupts are **disabled**.
