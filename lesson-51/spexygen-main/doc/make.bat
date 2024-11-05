@echo off
:: ===========================================================================
:: Product: batch script for generating Spexygen documentation
::
::                   Q u a n t u m  L e a P s
::                   ------------------------
::                   Modern Embedded Software
::
:: Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
::
:: SPDX-License-Identifier: MIT
::
:: Permission is hereby granted, free of charge, to any person obtaining a copy of
:: this software and associated documentation files (the "Software"), to deal in
:: the Software without restriction, including without limitation the rights to
:: use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
:: the Software, and to permit persons to whom the Software is furnished to do so,
:: subject to the following conditions:
::
:: The above copyright notice and this permission notice shall be included in all
:: copies or substantial portions of the Software.
::
:: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
:: IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
:: FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
:: COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
:: IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
:: CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
::
:: <www.state-machine.com>
:: <info@state-machine.com>
:: ===========================================================================
@setlocal

@echo usage:
@echo make
@echo make -PDF

:: tools (adjust to your system)----------------------------------------------
:: Doxygen/Spexygen tools
@set DOXYGEN=doxygen
@set SPEXYGEN=..

@echo Generate Spexygen tracing ----------------------------------------------
rmdir /S /Q  .\spex
python %SPEXYGEN%/spexygen.py spex.json

::============================================================================
@if "%1"=="-PDF" goto PDF

@echo Generate HTML Documentation --------------------------------------------
@set HTML_OUT=html

@echo.
@echo cleanup
rmdir /S /Q %HTML_OUT%

@echo generating HTML...
%DOXYGEN% Doxyfile

@echo Adding custom files...
copy %SPEXYGEN%\spexygen-awesome\jquery.js %HTML_OUT%
@copy DOC_MAN_SPX.pdf %HTML_OUT%

::qclean %HTML_OUT%
goto END

:PDF
@echo Generate PDF Documentation --------------------------------------------
@set LATEX_OUT=latex

@echo.
@echo cleanup
rmdir /S /Q  %LATEX_OUT%

@echo generating LATEX...
%DOXYGEN% Doxyfile-PDF

:: Generate LaTex/PDF Documentation...
@echo generating PDF...
@cd %LATEX_OUT%
@call make.bat
@copy refman.pdf ..\DOC_MAN_SPX.pdf
@cd ..
rmdir /S /Q  %LATEX_OUT%

:END
@echo Final cleanup ----------------------------------------------------------
rmdir /S /Q  .\spex

@endlocal
