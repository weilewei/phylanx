# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import re
import ast
import inspect
import phylanx
from .physl import PhySL
from .oscop import OpenSCoP
from phylanx import execution_tree
from phylanx.ast import generate_ast as generate_phylanx_ast
from phylanx.exceptions import InvalidDecoratorArgumentError


def Phylanx(__phylanx_arg=None, **kwargs):
    class __PhylanxDecorator(object):
        targets = {'PhySL': PhySL, 'OpenSCoP': OpenSCoP}

        def __init__(self, f):
            """
            :function:f the decorated funtion.
            """

            self.set_target(kwargs.get('target'))
            kwargs['fglobals'] = f.__globals__

            # get the source of the function and remover the decorator.
            src = inspect.getsource(f)
            src = re.sub(r'^\s*@\w+.*\n', '', src)

            # Generate the Python AST
            tree = ast.parse(src)
            actual_lineno = inspect.getsourcelines(f)[-1]
            ast.increment_lineno(tree, actual_lineno)
            assert len(tree.body) == 1
            self.transformation = self.targets[self.target](f, tree, kwargs)
            self.__src__ = self.transformation.__src__

        def set_target(self, target):
            """Set the target backend. By default it is set to PhySL."""
            if target:
                self.target = target
                if self.target not in __PhylanxDecorator.targets:
                    raise NotImplementedError(
                        "Unknown target: %s." % self.target)
            else:
                self.target = 'PhySL'

        def __call__(self, *args):
            if self.target == 'OpenSCoP':
                raise NotImplementedError(
                    "OpenSCoP kernels are not yet callable.")
            return self.transformation.call(args)

        def generate_ast(self):
            return generate_phylanx_ast(self.__src__)

    if callable(__phylanx_arg):
        return __PhylanxDecorator(__phylanx_arg)
    elif __phylanx_arg is not None:
        raise InvalidDecoratorArgumentError
    else:
        return __PhylanxDecorator
