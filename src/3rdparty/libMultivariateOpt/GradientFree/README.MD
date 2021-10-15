## Gradient free optimization
### Features

Differential evolution

- Bezier Search
- Almost parameter free
- Allow bound constraints, bound clipping or renew solutions

Nelder-Mead Simplex search with bound constraints

Flower pollination search

CHIO search

#### Q/A

- Can I add nonlinear/linear constraints other than just bound constraints?

Those are problem dependent, you have to modify your cost function in some ways.

- Why are the optimizer function argument could be that long?

We allow user to choose probability density functions(PDF), different PDF affects the random number distribution of the optmizer solution generator/modifier.
Last argument is callback function pointer, it allows programmer display the process of optimization to GUI, if there is no GUI, pass NULL, if there is a GUI, implement your own callback function.
