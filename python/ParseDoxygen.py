#!/usr/bin/env python


"""Copyright (C) University of Oxford, 2005-2009

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Chaste is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Chaste is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details. The offer of Chaste under the terms of the
License is subject to the License being interpreted in accordance with
English Law and subject to any action against the University of Oxford
being under the jurisdiction of the English Courts.

You should have received a copy of the GNU Lesser General Public License
along with Chaste. If not, see <http://www.gnu.org/licenses/>.
"""

"""
Parse the output of running Doxygen to find files that aren't properly documented.

The script takes arguments:
 <log_file>       Doxygen's normal output log.
 <error_log_file> Doxygen's error log output.
 <output_dir>     The directory in which to generate summary files and
                  an index page.
"""

import os
import re
import sys

if len(sys.argv) != 4:
    print "Syntax error."
    print "Usage:",sys.argv[0],"<log file> <error log file> <test output dir>"
    sys.exit(1)

log_file_name = sys.argv[1]
error_log_file_name = sys.argv[2]
output_dir = sys.argv[3]


def munge_name(file_name, status):
    """Return the file name to use for processed results.
    
    file_name is the name as it appears in the Doxygen logs.  This will be
    an absolute path.  We strip off the path to the Chaste root (i.e. make
    the name relative) and convert os.path.sep to '-'.
    
    The status code is then added, and our output_dir added on the front.
    """
    munged_file_name = file_name.replace(os.path.sep, '-')
    root_dir = os.getcwd().replace(os.path.sep, '-') + '-'
    if munged_file_name.startswith(root_dir):
        munged_file_name = munged_file_name[len(root_dir):]
    return os.path.join(output_dir, munged_file_name + '.' + status + '.0')

# Remove any old output files/test results from output_dir
for filename in os.listdir(output_dir):
    os.remove(os.path.join(output_dir, filename))

# Get a list of source files
source_files = set()
for line in open(log_file_name, 'U'):
    if line.startswith('Parsing file '):
        source_files.add(line[13:-4]) # NB: line ends with a newline

# Now work out which have problems
error_re = re.compile(r'^(.+):(\d+):')
file_name = None
problem_files = {}
counts = {}
for line in open(error_log_file_name):
    m = error_re.match(line)
    if m:
        file_name = m.group(1)
        if file_name[0] == '<':
            file_name = 'unknown-' + file_name[1:-1]
        if not file_name in problem_files:
            problem_files[file_name] = []
            counts[file_name] = 1
        else:
            counts[file_name] += 1
    if file_name:
        problem_files[file_name].append(line)

# Write output files for those with errors
for problem_file, lines in problem_files.iteritems():
    status = '%d_%d' % (counts[problem_file], counts[problem_file])
    output_file = open(munge_name(problem_file, status), 'w')
    output_file.write(''.join(lines))
    output_file.close()

# Now write 'OK' files for those without any errors
ok_files = source_files - set(problem_files.iterkeys())
for file_name in ok_files:
    output_file = open(munge_name(file_name, 'OK'), 'w')
    output_file.close()

# And generate a summary page
os.system('python python/DisplayTests.py '+output_dir+' DoxygenCoverage')
