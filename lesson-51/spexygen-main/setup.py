#!/usr/bin/env python
'''
Setup script.
To build _spexygen_ install:
[sudo] python setup.py sdist bdist_wheel
'''

from setuptools import setup

setup(
    name="spexygen",
    version="2.2.2",
    author="Quantum Leaps",
    author_email="info@state-machine.com",
    description="Traceable specifications based on doxygen",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    url="https://github.com/QuantumLeaps/spexygen",
    license="MIT",
    platforms="any",
    py_modules=["spexygen"],
    entry_points={"console_scripts": ["spexygen = spexygen:main"]},
    classifiers=["Development Status :: 5 - Production/Stable",
                 "Intended Audience :: Developers",
                 "Topic :: Software Development :: Documentation",
                 "License :: OSI Approved :: MIT License",
                 "Operating System :: Microsoft :: Windows",
                 "Operating System :: POSIX :: Linux",
                 "Operating System :: MacOS :: MacOS X",
                 "Programming Language :: Python",
                 "Programming Language :: Python :: 3"],
)
