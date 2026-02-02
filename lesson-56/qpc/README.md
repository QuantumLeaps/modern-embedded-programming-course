> NOTE: This is __NOT__ the full distribution of the QP/C framework.
This version contains only the files needed for this particular
lesson in the "Modern Embedded Systems Programming" course. Please
see [Quantum Leaps downloads][Down] for the complete version.

![QP Framework](https://www.state-machine.com/img/qp_banner.jpg)

# What's New?

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/QuantumLeaps/qpc)](https://github.com/QuantumLeaps/qpc/releases/latest)

View QP/C Revision History at: https://www.state-machine.com/qpc/history.html

> **NOTE:** If you're interested in the latest QP/C version from GitHub,
it is recommended that you clone this repo like that:

```
git clone https://github.com/QuantumLeaps/qpc --recurse-submodules --depth 1
```

However, the easiest and most recommended way of
[getting started with QP/C](#getting-started-with-qpc) is to download
the QP-bundle, as [described below](#getting-started-with-qpc).


# About QP/C Real-Time Event Framework
QP/C real-time event framework (RTEF) is a lightweight implementation of
the asynchronous, event-driven [Active Object (a.k.a. Actor) model of computation][AOmod]
specifically designed for real-time embedded systems, such as microcontrollers (MCUs).
QP/C is both a software infrastructure for building applications consisting
of Active Objects (Actors) and a runtime environment for executing the Active
Objects in a deterministic, real-time fashion. Additionally, QP/C Framework
supports Hierarchical State Machines with which to specify the behavior of
Active Objects [UML 2.5], [Sutter:10], [ROOM:94]. The QP/C Framework can be
viewed as a modern, asynchronous, and truly event-driven real-time operating
system (RTOS).

## QP Framework Family
QP/C framework is part of the larger QP family, consisting of the following
QP editions (please also see [__QP Frameworks Feature Comparison__](https://www.state-machine.com/products/qp#Features)
):

|QP Edition |  Language   |      API         | Safety Functions   |Certification Artifacts| Licensing
|:----------|:-----------:|:-----------------|:-------------------|:----------------|:---------
| QP/C      | C (C11)     |same as<br>SafeQP/C  |Selected Assertions |Req/Arch/Design  | [open-source & commercial][Lic]
| QP/C++    | C++ (C++17) |same as<br>SafeQP/C++|Selected Assertions |Req/Arch/Design  | [open-source & commercial][Lic]
| SafeQP/C  | C (C11)     |same as<br>QP/C      |All Safety Functions|[SafeQP/C<br>Certification Kit][CERT-KIT]| [commercial][Com]
| SafeQP/C++| C++ (C++17) |same as<br>QP/C++    |All Safety Functions|[SafeQP/C++<br>Certification Kit][CERT-KIT]| [commercial][Com]

The **SafeQP/C** and **SafeQP/C++** frameworks were originally derived from QP/C and QP/C++,
respectively, but were extensively reengineered for the safety market using compliant
Software Safety Lifecycle (SSL). In this process, the QP framework functional model has been
subjected to a full Hazard and Risk Analysis, which identified all areas of weakness within
the functional model and API. These findings led to the creation of Safety Requirements and risk
mitigation by Safety Functions, which were subsequently implemented, verified, and validated.
The SafeQP frameworks are accompanied by the "SafeQP Certification Kits", which provide
developers with ready-to-use artifacts, enabling them to save time, mitigate risk, and reduce
costs during application certification for safety-critical devices in the industrial, medical,
aerospace, and automotive industries. Please [contact Quantum Leaps][Cont]
for more information about the SafeQP frameworks and the "Certification Kits".

> **NOTE:** The SafeQP/C edition remains fully API- and functionally compatible with the
corresponding standard QP/C framework. This ensures existing QP/C Applications can transition
seamlessly to SafeQP/C without requiring any modifications. SafeQP/C edition retains QP/C
Frameworks' hallmark features: a small memory footprint, excellent efficiency, and hard
real-time performance.


# Getting Started with QP/C
The most recommended way to get started with QP/C is by
[__downloading the QP-bundle__](https://www.state-machine.com/#Downloads),
which includes QP/C as well as the [QM modeling tool][QM] and the
[QTools collection][QTools]. The main advantage of obtaining QP/C bundled
together is that you get all components, tools, and _examples_ all ready to go.

> **NOTE:**
Perhaps the most important fact to remember is that in embedded systems,
nothing works until everything works. This means that you should always start
with a _working system_ and gradually evolve it, changing one thing at a time
and making sure that it keeps working every step of the way.

The provided [__QP/C example projects__](examples), such as the super-simple
[Blinky](examples/arm-cm/blinky_nucleo-u545re/), or a bit more advanced
[Dining Philosophers Problem (DPP)](examples/arm-cm/dpp_nucleo-u545re/),
allow you to get started with a __working project__ rather than starting from
scratch. You should also always try one of the unmodified examples on one
of the very inexpensive evaluation boards (such as
[STM32 NUCLEO-U545RE](examples/arm-cm/dpp_nucleo-u545re/stm32-nucleo-u545re.webp))
that it was designed for, _before_ attempting to immediately adapt the projects
to your specific hardware. Only once an example project is built and runs on
the evaluation board, can you use it as a starting point for your specific hardware
and software development.


## Getting Started Resources
- [Video: "Getting Started with QP Real-Time Event Frameworks"][Video]
provides instructions on how to download, install, and get started with QP.

- [AppNote: "Getting Started with QP Real-Time Event Frameworks"][AN]
contains also a tutorial, in which you build a simple "Blinky" application.

- ["QP/C Tutorial"][Tut]
describes a series of progressively advanced QP/C example applications.

## QP/C Extras
The open source GPL distribution of QP/C can be augmented by the
["QP/C Extras"][QPX/C], which provide more advanced QP/C features, such as:
- [QS software tracing][QS] component (QP Spy)
- [QXK real-time kernel][QXK] component
- Static-analysis configuration and automation scripts for PC-Lint-Plus (MISRA-C:2025 compliance)
- Test suite (based on the [QUTest trace-based test harness][QUTest])
that demonstrates 100% lines of code and 100% MC/DC code coverage for QP/C.

> **NOTE:** The ["QP/C Extras"][QPX/C] are [licensed commercially][Lic] only
and available to the commercial licensees with the active Support Term. Please contact
[Quantum Leaps technical support][Sup] to get the matching ["QP/C Extras"][QPX/C]
for the public QP/C version.

> **NOTE:** The ["QP/C Extras"][QPX/C] are also __available for evaluation__
([upon request][ReqForm]).


# Licensing
The QP/C real-time event framework is licensed under the
[dual licensing model](https://www.state-machine.com/licensing), with
the following licensing options:

1. [Open-source licensing](https://www.state-machine.com/licensing#Open) under the
[GNU General Public License (GPLv3)](https://www.gnu.org/licenses/gpl-3.0.en.html).

> NOTE: The GPL requires that all modifications to the original code
as well as your application code (Derivative Works as defined in the
Copyright Law) must also be released under the terms of the GPL
open source license.

2. [Closed-source licensing](https://www.state-machine.com/licensing#Closed) under one of
[Quantum Leaps commercial licenses](https://www.state-machine.com/licensing#Commercial),
which are specifically designed for users interested in retaining the
proprietary status of their code.

> NOTE: If your company has a policy forbidding open source in your product,
all QP frameworks can be licensed commercially, in which case you don't use
any open source license and you do not violate your policy.

> NOTE: The **SafeQP** frameworks and the accompanying "Certification Kits"
are licensed commercially only.


# Documentation
The online HTML documentation for the **latest** version of QP/C is located
at: https://www.state-machine.com/qpc

The offline HTML documentation for **this** particular version of QP/C
is located in the sub-folder [html](html) (included in the [QP/C releases][QP-Rel]).
To view the offline documentation, open the file [html/index.html](html/index.html)
in your web browser.


# Support & Contact Information
- [Free Support Forum](https://sourceforge.net/p/qpc/discussion/668726)
- [Bug Reports](https://sourceforge.net/p/qpc/bugs/)
- [Feature Requests](https://sourceforge.net/p/qpc/feature-requests/)
- [Quantum Leaps website](https://www.state-machine.com)
- [Quantum Leaps licensing](https://www.state-machine.com/licensing)
- [info@state-machine.com](mailto:info@state-machine.com)


# How to Help this Project?
If you like this project, please give it a star (in the upper-right corner of your browser window):

<p align="center">
<img src="https://www.state-machine.com/img/github-star.jpg"/><br>
</p>

   [Down]:   <https://www.state-machine.com#Downloads>
   [Cont]:   <https://www.state-machine.com/contact>
   [RTEF]:   <https://www.state-machine.com/rtef>
   [QP]:     <https://www.state-machine.com/products/qp>
   [QP/C]:   <https://github.com/QuantumLeaps/qpc>
   [QP/C++]: <https://github.com/QuantumLeaps/qpcpp>
   [QPX/C]:  <https://www.state-machine.com/qpc/gs_extras.html>
   [QPX/C++]:  <https://www.state-machine.com/qpcpp/gs_extras.html>
   [CERT-KIT]: <https://www.state-machine.com/products/qp#CERT-KIT>
   [QM]:     <https://github.com/QuantumLeaps/qm>
   [QTools]: <https://github.com/QuantumLeaps/qtools>
   [QUTest]: <https://www.state-machine.com/qtools/qutest.html>
   [Lic]:    <https://www.state-machine.com/licensing>
   [Com]:    <https://www.state-machine.com/licensing#Commercial>
   [ReqForm]: <https://www.state-machine.com/licensing#RequestForm>
   [Cust]:   <https://www.state-machine.com/customers>
   [Sup]:    <mailto:support@state-machine.com>
   [AN]:     <https://www.state-machine.com/doc/AN_Getting_Started_with_QP.pdf>
   [Video]:  <https://youtu.be/O7ER6_VqIH0>
   [QS]:     <https://www.state-machine.com/qpc/srs-qp_qs.html>
   [QV]:     <https://www.state-machine.com/qpc/srs-qp_qv.html>
   [QK]:     <https://www.state-machine.com/qpc/srs-qp_qk.html>
   [QXK]:    <https://www.state-machine.com/qpc/srs-qp_qxk.html>
   [SRS]:    <https://www.state-machine.com/qpc/srs-qp.html>
   [SAS]:    <https://www.state-machine.com/qpc/sas-qp.html>
   [SDS]:    <https://www.state-machine.com/qpc/sds-qp.html>
   [Active]: <https://www.state-machine.com/qpc/srs-qp_ao.html>
   [AOmod]:  <https://www.state-machine.com/qpc/srs-qp_ao.html#srs-qp_ao-model>
   [Event]:  <https://www.state-machine.com/qpc/srs-qp_evt.html>
   [HSM]:    <https://www.state-machine.com/qpc/srs-qp_sm.html>
   [QP-Rel]: <https://github.com/QuantumLeaps/qpc/releases>
   [Tut]:    <https://www.state-machine.com/qpc/gs_tut.html>
