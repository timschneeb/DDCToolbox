## Gradient based optimization
### Features

Reverse mode automatic differentiation

- Based on KANN library
- Extra functions other than "stock" KANN
- Differentiable tensor-scalar operation for Add, Sub, Mul, Div

#### Is the library reliable?
Not quite, but I'm working on it to make it as robust as other AD library

#### Q/A

- Can I add nonlinear/linear constraints other than just bound constraints?

No, not really, but not impossible, modify your cost function is the only way at the moment