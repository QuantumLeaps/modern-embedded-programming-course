# Sub-directories in this lesson:
```
|
+---qpc             - QP/C framework with event-processor port
|
......................projects for this lesson.............................
|
\---qhsmtst-console
    |
    \---build
            qhsmtst.exe - Windows executable to experiment with QHsmTst
        ...
        Makefile    - Makefile to build QHsmTst example with GCC (MigGW)
        ...
        qhsmtst.sln - Visual Studo solutin to build QHsmTst
```

# Example run
```
C:\modern-embedded-programming-course\lesson-42\qhsmtst-console>build\qhsmtst.exe
QHsmTst example, built on Nov  7 2024 at 11:16:13,
QP: 7.3.0.
Enter x or X to quit...
top-INIT;s-ENTRY;s2-ENTRY;s2-INIT;s21-ENTRY;s211-ENTRY;
>>>a
s21-A;s211-EXIT;s21-EXIT;s21-ENTRY;s21-INIT;s211-ENTRY;
>>>b
s21-B;s211-EXIT;s211-ENTRY;
>>>c
s2-C;s211-EXIT;s21-EXIT;s2-EXIT;s1-ENTRY;s1-INIT;s11-ENTRY;
>>>x

Bye, Bye!

```