# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx, PhylanxSession

PhylanxSession(1)


@Phylanx
def f():
    a = None
    return a


assert f.__src__ == \
    "define$12$0(f$12$0, block$12$0(define$13$4(a$13$4, nil$13$8), a$14$11))"


@Phylanx
def f():
    a = True
    return a


assert f.__src__ == \
    "define$22$0(f$22$0, block$22$0(define$23$4(a$23$4, true$23$8), a$24$11))"


@Phylanx
def f():
    a = False
    return a


assert f.__src__ == \
    "define$32$0(f$32$0, block$32$0(define$33$4(a$33$4, false$33$8), a$34$11))"
