:: Batch file for building the Python package, checking it,
:: and uploading it to PyPi.
::
:: usage:
:: make
::
@setlocal

:: set the project
@set PROJ=spexygen

:: adjust the Python location for your system
@set PYTHON=python

:: cleanup any previous builds...
@rmdir /S /Q build
@rmdir /S /Q dist
@rmdir /S /Q %PROJ%.egg-info

:: execute the build...
%PYTHON% setup.py sdist bdist_wheel

:: check the build...
twine check dist/*

:: upload to PyPi -- skip for now
twine upload dist/* --verbose

:: copy/rename the "wheel" to the current dir
@cp dist/%PROJ%-*.tar.gz %PROJ%.tar.gz

:: cleanup after the build...
@rm -rf build
@rm -rf dist
@rm -rf %PROJ%.egg-info

@endlocal
