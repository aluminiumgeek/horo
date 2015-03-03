# -*- coding: utf-8 -*-
# test_module.py (c) Mikhail Mezyakov <mihail265@gmail.com>
# Released under the GNU GPL v.3
#
# Import module from 'modules' directory and test it in a command line
# First argument must be a module name; others - module parameters
# If --help/-h option is specified after module name, print docstring

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

import importlib

test_channel = '#test'
test_user = 'user'

if __name__ == '__main__':
    module_name = sys.argv[1]
    del sys.argv[1]
    del sys.argv[0]

    module = importlib.import_module('modules.{0}'.format(module_name))

    if sys.argv and sys.argv[0] in ('--help', '-h'):
        result = module.horo.__doc__
    else:
        result = module.horo(test_channel, test_user, sys.argv)
        if hasattr(module.horo, 'test'):
            assert result == module.horo.test, 'Something went wrong inside the module:\n{} != {}'.format(result, module.horo.test)

    print >>sys.stdout, result
