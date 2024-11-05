#!/usr/bin/env python

#=============================================================================
# _Spexygen_ - Traceable Specifications Based on doxygen
# Copyright (C) 2024 Quantum Leaps, LLC <www.state-machine.com>
#
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Contact information:
# <www.state-machine.com>
# <info@state-machine.com>
#=============================================================================

'''
Spexygen is a Doxygen extension for creating traceable technical
specifications, such as:
- traceable requirement specifications
- traceable source code
- traceable tests
- traceable specifications of other kind
'''

import sys
import os
import json

class Spexygen:
    '''
    Spexygen class that encapsulates tracing and code generation
    '''

    # public class constants
    VERSION = 222

    UID_DOC  = 1
    UID_CODE = 2
    LEVELS   = ('', '  ', '    ', '      ', '        ')
    _debug   = False

    def __init__(self):
        # private members
        self._uid = None           # current UID
        self._kind = 0             # current UID-kind
        self._fname = ''           # current file name (for generating output)
        self._lnum = 0             # current linie number (for generating output)
        self._file = None          # current file (for generating output)
        self._prefix = ''          # prefix for each generated line
        self._bw_trace = ''        # bw-trace for current uid requested
        self._uid_trace_dict = {}  # UID dictionary collected during tracing
        self._uid_brief_dict = {}  # UID-brief dictionary collected during tracing
        self._uid_traced_list = [] # UID list built during generation

    @staticmethod
    def debug(*args, **kwargs):
        '''debug output (if requesed in command-line parameters)
        '''
        if Spexygen._debug:
            print(*args, **kwargs)


    def on_file_pattern(self, fname):
        '''return True if file-name is acceptable for Spexygen
        '''
        return fname.endswith('.dox') or \
               fname.endswith('.h') or fname.endswith('.c') or \
               fname.endswith('.hpp') or fname.endswith('.cpp') or \
               fname.endswith('.py') or fname.endswith('.lnt')

    def on_gen_fw_trace(self, uid, level):
        '''recursively generate the forward trace for a "uid"
        '''
        self._uid_traced_list.append(uid)
        Spexygen.debug("  level=", level, "uid=", uid,
                       "self._uid_traced_list:", self._uid_traced_list)
        for uid in self._uid_trace_dict.get(uid):
            if uid not in self._uid_traced_list:
                self._file.write("%s%s- @tr{%s}: %s\n"
                    %(self._prefix, Spexygen.LEVELS[level],
                    uid, self._uid_brief_dict[uid]))
                if level < len(Spexygen.LEVELS) - 2:
                    if uid in self._uid_trace_dict:
                        self.on_gen_fw_trace(uid, level+1) # recursive!
                else:
                    self._file.write("%s%s- ...\n"
                        %(self._prefix, Spexygen.LEVELS[level+1]))
                    print(f"  {self._fname}:{self._lnum} too many"\
                          f' forward trace levels for "{uid}"')

    def uid_begin(self, line):
        '''set the current uid 'self._uid' and 'self._kind'
        if UID found in the given 'line'
        '''
        if self._uid != '':
            print("Looking for new UID while previous is still active"\
                  f" {self._uid}")
        kind = Spexygen.UID_DOC
        if (i := line.find('@uid{')) >= 0:
            l = 5
        elif (i := line.find('@code_uid{')) >= 0:
            kind = Spexygen.UID_CODE
            l = 10
        elif (i := line.find('@code_alias{')) >= 0:
            kind = Spexygen.UID_CODE
            l = 12
        else:
            return

        j = line.find(',', i + l)
        if j < 0:
            print("Error: missing ',' in UID definition",
                    "line", self._lnum, ":", i)
            return

        k = line.find('}', j + 1)
        if k < 0:
            print("Error: missing '}' in UID definition",
                    "line", self._lnum, ":", i)
            return

        self._kind = kind
        self._uid = line[i + l:j].strip()
        brief = line[j + 1:k].strip()
        self._bw_trace = ''
        if self._uid not in self._uid_brief_dict:
            self._uid_brief_dict[self._uid] = brief
        Spexygen.debug("  uid:", self._uid, brief)

    def uid_end(self, line):
        '''return True if UID-end was found in the "line"
        '''
        if self._uid == '':
            print("Looking for UID-end while no UID active")
            return False

        if self._kind == Spexygen.UID_DOC:
            if line.find('@enduid') >= 0:
                Spexygen.debug("  end:", self._uid)
                self._uid = ''
                self._bw_trace = ''
                return True
        elif self._kind == Spexygen.UID_CODE:
            if line.find('@endcode_uid') >= 0:
                Spexygen.debug("  end:", self._uid)
                self._uid = ''
                self._bw_trace = ''
                return True
        else:
            print(f"Unknown current UID kind={self._kind}")

        return False

    def uid_bw_trace(self, line):
        '''return True if bw-trace placeholder found in "line"
        '''
        if self._uid == '':
            return False
        l = 0
        i = -1
        if self._kind == Spexygen.UID_DOC:
            if (i := line.find('@uid_bw_trace')) >= 0:
                l = 13
        elif self._kind == Spexygen.UID_CODE:
            if (i := line.find('@code_bw_trace')) >= 0:
                l = 14
        else:
            print(f"Unknown current UID kind={self._kind}")
            return False

        if i < 0:
            return False

        if line.find('{', i+l) == i+l: # parameter present?
            j = line.find('}', i+l+1)
            if j >= 0:
                self._bw_trace = line[i+l+1:j]
            else:
                print("Error: missing '}' for '@uid_bw_trace{' in line",
                        self._lnum, ":", i+l+1)
                return False
        else:
            self._bw_trace = 'empty'
        return True

    def uid_tr(self, line):
        '''return list of backward traces found in a given "line"
        '''
        tr_list = []
        i = 0
        while (i := line.find('@tr{', i)) >= 0:
            j = line.find('}', i + 4)
            if j >= 0:
                tr = line[i + 4:j]
                tr_list.append(tr)
                i = j
            else:
                print("Error: missing '}' for '@tr{' in line",
                      self._lnum, ":", i)
                break
        return tr_list

    def trace(self, fname):
        '''trace a given file and harvest the traces into the
        dictionaries: self._uid_brief_dict and self._uid_trace_dict
        '''
        try:
            f = open(fname, encoding="utf-8")
        except OSError:
            print("File not found", fname)
            return
        with f:
            lines = f.readlines()

        print("Tracing:", fname)
        self._uid = ''
        self._lnum = 0
        for line in lines:
            self._lnum += 1
            if self._uid == '':
                self.uid_begin(line)
            else:
                if self.uid_end(line):
                    pass
                elif self.uid_bw_trace(line):
                    pass
                elif self._bw_trace != '':
                    tr_list = self.uid_tr(line)
                    for tr in tr_list:
                        if not self._uid_trace_dict.get(tr):
                            self._uid_trace_dict[tr] = []
                        if self._uid not in self._uid_trace_dict[tr]:
                            self._uid_trace_dict[tr].append(self._uid)
                            Spexygen.debug(tr, '<-', self._uid)

    def gen_bw_trace(self, line):
        '''find a bw-trace placeholder
        as long as bw-trace found
        return True if bw-trace placeholder or "@tr{}" found
        '''
        if self.uid_bw_trace(line):
            self._file.write(line)
            return True

        if self._bw_trace != '':
            if (i := line.find('@tr{')) >= 0:
                j = line.find('}', i + 4)
                tr = ''
                if j >= 0:
                    tr = line[i + 4:j]
                else:
                    print("Error: missing '}' for '@tr{' in line",
                          self._lnum, ":", i)
                    self._file.write(line)
                    return True
                if tr in self._uid_brief_dict:
                    if self._bw_trace == 'brief':
                        self._file.write(line[:j+1])
                        self._file.write(f": {self._uid_brief_dict[tr]}")
                        self._file.write(line[j+1:])
                    else:
                        self._file.write(line)
                else:
                    print(f'  {self._fname}:{self._lnum} '\
                          f'"{tr}" undefined in backward trace'\
                          f' for UID: "{self._uid}"')
                    self._file.write(line)
                return True
        return False


    def gen_fw_trace(self, line):
        '''find a fw-trace placeholder and generate fw-trace
        return True if the placeholder found
        '''
        if (i := line.find('@uid_fw_trace')) < 0:
            i = line.find('@code_fw_trace')
        if i < 0:
            return False # placeholder not found

        self._prefix = line[:i]
        self._file.write(line)
        if self._uid in self._uid_trace_dict:
            self._uid_traced_list = []
            self.on_gen_fw_trace(self._uid, 0)
        else:
            print(f'  {self._fname}:{self._lnum} empty forward trace'\
                  f' for UID: "{self._uid}"')

        return True

    def gen(self, gendir, fname):
        '''generate a given file, replacing the detected placeholdrs
        with information from the dictionaries:
        self._uid_brief_dict and self._uid_trace_dict
        '''
        try:
            f = open(fname, encoding="utf-8")
        except OSError:
            print("File not found", fname)
            return
        with f:
            lines = f.readlines()

        self._fname = os.path.basename(fname)
        fname = gendir + '/' + self._fname
        try:
            self._file = open(fname, 'w', encoding="utf-8")
        except OSError:
            print("File cannot be written", fname)
            return
        with self._file:
            print("Generating:", fname)
            self._lnum = 0
            self._uid = ''
            self._bw_trace = ''
            for line in lines:
                self._lnum += 1
                if self._uid == '': # no current item
                    self._file.write(line)
                    self.uid_begin(line)
                else:              # have current item
                    if self.uid_end(line):
                        self._file.write(line)
                    elif self.gen_fw_trace(line):
                        pass
                    elif self.gen_bw_trace(line):
                        pass
                    else:
                        self._file.write(line)

    def main(self):
        '''main entry point to Spexygen
        process command-line arguments and run Spexygen
        '''
        print(f"Spexygen: traceable technical documentation system "\
            f"{self.VERSION//100}.{(self.VERSION//10) % 10}."\
            f"{self.VERSION % 10}")
        print("Copyright (c) 2005-2024 Quantum Leaps, www.state-machine.com\n")

        # defaults
        sfname = 'spex.json'
        gendir = 'spex'
        spexyinp = 'Spexyinput'

        # process command-line arguments...
        argv = sys.argv
        argc = len(argv)
        arg  = 1 # skip the 'Spexygen' argument

        if '-h' in argv or '--help' in argv or '?' in argv \
            or '--version' in argv:
            print("Usage: python [-d] spexygen.py [spexyfile]",
                  "\n(deafult spexyfile: 'spexy.json')")
            sys.exit(0)

        if '-d' in argv:
            Spexygen._debug = True
            argv.remove('-d')
            argc -= 1

        if arg < argc:
            # is the next argument a test script?
            sfname = argv[arg]

        # parse the provided spexyfile as json data
        try:
            sfile = open(sfname, encoding="utf-8")
        except OSError:
            print("spexyfile not found:", sfname)
            sys.exit(-1)
        with sfile:
            try:
                spex = json.load(sfile)
            except ValueError as e:
                print("Error in spexyfile:", sfname)
                print(e)
                sys.exit(-1)

        # perform tracing
        key = 'trace'
        if key not in spex:
            key = 'gen' # no 'trace' section means use 'gen' section
        if key not in spex:
            print("Spexygen: nothing to do")
            return
        for path in spex[key]:
            if os.path.isfile(path):
                if self.on_file_pattern(path):
                    self.trace(path)
            elif os.path.isdir(path):
                for fname in os.listdir(path):
                    if self.on_file_pattern(fname):
                        self.trace(path + '/' + fname)
            else:
                print("not exist:", path)
        Spexygen.debug("\n_uid_trace_dict", self._uid_trace_dict)
        Spexygen.debug("\n_uid_brief_dict", self._uid_brief_dict)

        key = 'gen-dir'
        if key in spex:
            gendir = spex[key]
        if gendir == '':
            print("No code generation (empty gen directory)")
            return

        key = 'gen-inc'
        if key in spex:
            spexyinp = spex[key]

        key = 'gen'
        geninc = None
        if key not in spex:
            print("Spexygen: nothing to generate")
            return

        if not os.path.exists(gendir):
            os.mkdir(gendir)
        if spexyinp != '':
            try:
                geninc = open(gendir + '/' + spexyinp, 'w',
                              encoding="utf-8")
            except OSError:
                print("spexyinp cannot be written:", spexyinp)
                sys.exit(-1)
        if geninc:
            geninc.write('INPUT +=')

        # generate documentation
        for path in spex[key]:
            if os.path.isfile(path):
                if self.on_file_pattern(path):
                    self.gen(gendir, path)
                    if geninc:
                        geninc.write(' \\\n' + gendir + '/'
                                     + os.path.basename(path))
            elif os.path.isdir(path):
                for fname in os.listdir(path):
                    if self.on_file_pattern(fname):
                        self.gen(gendir, path + '/' + fname)
                        if geninc:
                            geninc.write(' \\\n' + gendir + '/'
                                        + os.path.basename(fname))
            else:
                print("not exist:", path)

        if geninc:
            geninc.close()

#=============================================================================
if __name__ == "__main__":
    spx = Spexygen()
    spx.main()
