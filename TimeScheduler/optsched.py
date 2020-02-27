import numpy as np
from scipy.optimize import minimize

from scipy.optimize import Bounds
bounds = Bounds([15, 1, 30, 1], [25, 3, 50, 4])

def sched(x):
    return -x[0]
    

def cons_f(x):
    return [x[0]**2 + x[1], x[0]**2 - x[1]]
def cons_J(x):
    return [[2*x[0], 1], [2*x[0], -1]]
def cons_H(x, v):
    return v[0]*np.array([[2, 0], [0, 0]]) + v[1]*np.array([[2, 0], [0, 0]])

from scipy.optimize import NonlinearConstraint
nonlinear_constraint = NonlinearConstraint(cons_f, -np.inf, 1, jac=cons_J, hess=cons_H)

from scipy.optimize import SR1
x0 = np.array([0.1, 0.2, 0.3, 0.4])
res = minimize(sched, x0, method='trust-constr', jac="2-point", hess=SR1(), constraints=[nonlinear_constraint], options={'verbose': 1}, bounds=bounds)

print (res.x)

def calc(minutes: int) -> (int, int):
    worku = 86
    breaku = 40

    return divmod((minutes-worku), (worku + breaku))# -*- coding: utf-8 -*-

