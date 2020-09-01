Executing smc.jar
-------------------

Run SMC with Java as follows.

$ java -jar Smc.jar <options> <.sm source files>

SMC's command line options are:

    -suffix <suffix>
             : Use this suffix on the output file.
    -g, -g0, -g1
             : Add debugging to generated code. -g0 reports state
               changes and transitions. -g1 reports entry and
               exit actions. -g is the same as -g0.
    -nostreams
             : Do not use C++ iostream
               (use with -c++ only).
    -version : Print SMC version to standard out and exit.
    -verbose : Output compiler messages (SMC is silent by default).
    -vverbose: Output more compiler messages.
    -help    : Print this message to standard out and exit.
    -sync    : Synchronize generated Java code
               (use with -java, -groovy, -vb and -csharp only).
    -noex    : Do not generate C++ exception throws
               (use with -c++ only).
    -nocatch : Do not generate try/catch/rethrow code
               (not recommended).
    -serial  : Generate serialization code.
    -return  : Smc.main() does not exit but simply returns.
               Use this option with ANT.
    -reflect : Generate reflection code.
               (use with -java, -tcl, -vb, -csharp
               -groovy, -lua, -perl, -python and -ruby only).
    -cast <cast type>
             : use this C++ cast type
               (use with -c++ only).
    -headerd <directory>
             : Place generated header files in directory.
               (use with -c, -c++, -objc only).
    -d <directory>
             : Place generated files in directory.
    -glevel <0 - 2>
             : Graph detail level from 0 (least) to 2 (greatest)
               (use with -graph only).
    -c       : Generate C code.
    -c++     : Generate C++ code.
    -csharp  : Generate C# code.
    -graph   : Generate a Graphviz DOT file.
    -groovy  : Generate Groovy code.
    -java    : Generate Java code.
    -lua     : Generate Lua code.
    -objc    : Generate Objective-C code.
    -perl    : Generate Perl code.
    -php     : Generate PHP code.
    -python  : Generate Python code.
    -ruby    : Generate Ruby code.
    -scala   : Generate Scala code.
    -table   : Generate an HTML table.
    -tcl     : Generate [incr Tcl] code.
    -vb      : Generate VB.Net code.

    NOTE: Source files *must* end in ".sm" or ".SM".
          Exactly one of the following must be specified:
          + -c
          + -c++
          + -csharp
          + -graph
          + -groovy
          + -java
          + -lua
          + -objc
          + -perl
          + -php
          + -python
          + -ruby
          + -scala
          + -table
          + -tcl
          + -vb


