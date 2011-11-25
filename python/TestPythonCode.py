#!/usr/bin/env python

"""Copyright (C) University of Oxford, 2005-2011

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
Tester for Chaste Python code.  Runs unittest tests and makes the output
follow the same format as the cxxtest shipped with Chaste.

The first argument should be the path to a Python source file.  If this
file defines a MakeTestSuite method, this will be called to obtain a test
suite to run.  Otherwise, we scan the file for TestCase subclasses, and
run them all.

The optional second argument can restrict which test cases to run, as for
unittest.main.  For example, it can be 'MyTestCase' or
'MyTestCase.TestMethod'.

If no arguments are supplied it will run the example tests defined in this
module, class TestTest.
"""

import imp
import os
import sys
import time
import unittest

class TestTest(unittest.TestCase):
    """A simple test case for testing the framework."""
    def TestOk(self):
        self.failUnless(True)
    
    def TestFail(self):
        self.failIf(True)
    
    def TestError(self):
        self.assertEqual(1, 1/0)

#def MakeTestSuite():
#    """An example of how to create a suite of tests."""
#    return unittest.TestSuite(map(TestTest, ['TestOk', 'TestFail', 'TestError']))


class ChasteTestResult(unittest.TestResult):
    """A test result class that can print cxxtest-formatted text results to a stream.

    Used by _ChasteTestRunner.
    """
    separator1 = '=' * 70
    separator2 = '-' * 70

    def __init__(self, stream, descriptions=True):
        unittest.TestResult.__init__(self)
        self.stream = stream
        self.descriptions = descriptions

    def getDescription(self, test):
        if self.descriptions:
            return test.shortDescription() or str(test)
        else:
            return str(test)

    def startTest(self, test):
        unittest.TestResult.startTest(self, test)
        self.stream.write("Entering " + self.getDescription(test) + "\n")

    def addSuccess(self, test):
        unittest.TestResult.addSuccess(self, test)
        self.stream.write("Passed\n")

    def addError(self, test, err):
        unittest.TestResult.addError(self, test, err)
        self.stream.write(self.errors[-1][1])
        self.stream.write("Failed\n")

    def addFailure(self, test, err):
        unittest.TestResult.addFailure(self, test, err)
        self.stream.write(self.failures[-1][1])
        self.stream.write("Failed\n")

class ChasteTestRunner:
    """A test runner class that displays results in Chaste's cxxtest format."""
    def __init__(self, stream=sys.stdout, descriptions=True):
        self.stream = stream
        self.descriptions = descriptions

    def _makeResult(self):
        return ChasteTestResult(self.stream, descriptions=self.descriptions)

    def run(self, test):
        "Run the given test case or test suite."
        result = self._makeResult()
        start_time = time.time()
        test(result)
        stop_time = time.time()
        time_taken = stop_time - start_time
        num_run = result.testsRun
        self.stream.write("\nRan %d test%s in %.3fs\n\n" %
                            (num_run, num_run != 1 and "s" or "", time_taken))
        if not result.wasSuccessful():
            failed, errored = map(len, (result.failures, result.errors))
            num_bad = failed + errored
            self.stream.write("Failed %d of %d tests\n" % (num_bad, num_run))
        else:
            self.stream.write("OK!\n")
        return result
    
class ChasteTestLoader(unittest.TestLoader):
    """Allow test methods to start with either Test or test."""
    def getTestCaseNames(self, testCaseClass):
        """Return a sorted sequence of method names found within testCaseClass."""
        def isTestMethod(attrname, testCaseClass=testCaseClass):
            return (callable(getattr(testCaseClass, attrname)) and
                    (attrname.startswith('Test') or attrname.startswith('test')))
        test_names = filter(isTestMethod, dir(testCaseClass))
        for base_class in testCaseClass.__bases__:
            for test_name in self.getTestCaseNames(base_class):
                if test_name not in test_names:  # handle overridden methods
                    test_names.append(test_name)
        if self.sortTestMethodsUsing:
            test_names.sort(self.sortTestMethodsUsing)
        return test_names


if __name__ == '__main__':
    if len(sys.argv) > 1:
        filepath = sys.argv[1]
        if os.path.isfile(filepath):
            # Load Python file and run its tests
            dirpath = os.path.dirname(filepath)
            base, ext = os.path.splitext(os.path.basename(filepath))
            if ext != '.py':
                raise ValueError(filepath + ' is not a Python source file')
            (file, pathname, desc) = imp.find_module(base, [dirpath])
            try:
                module = imp.load_module(base, file, pathname, desc)
            finally:
                file.close()
            if hasattr(module, 'MakeTestSuite') and callable(module.MakeTestSuite):
                suite = module.MakeTestSuite()
                result = ChasteTestRunner().run(suite)
                sys.exit(not result.wasSuccessful())
            else:
                unittest.main(module=module, argv=[sys.argv[0]] + sys.argv[2:],
                              testRunner=ChasteTestRunner(), testLoader=ChasteTestLoader())
        else:
            raise ValueError(filepath + ' is not a file')
    else:
        # Default test of this file
        unittest.main(testRunner=ChasteTestRunner(), testLoader=ChasteTestLoader())