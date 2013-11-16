# test_module.py (c) Mikhail Mezyakov <mihail265@gmail.com>
# Released under the GNU GPL v.3
#
# Import module from 'modules' directory and test it in a command line
# First argument must be a module name; others - module parameters

import sys
import importlib

test_channel = '#test'
test_user = 'user'

if __name__ == '__main__':
    module_name = sys.argv[1]
    del sys.argv[1]
    del sys.argv[0]

    module = importlib.import_module('modules.{0}'.format(module_name))
    
    print >>sys.stdout, module.horo(test_channel, test_user, sys.argv)
