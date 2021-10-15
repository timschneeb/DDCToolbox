#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vld.h>
#include <kann.h>
void kann_gradient_optimizer(kann_t *ann, double lr, double lrDecayRate, int max_epoch, double *_x, double *_y)
{
	int n_in = kann_dim_in(ann);
	int n_out = kann_dim_out(ann);
	if (n_in < 0 || n_out < 0)
		return;
	int n_var = kann_size_var(ann);
	int n_const = kann_size_const(ann);
	double lrD = lr;
	grad_optimizer state;
	kann_InitRMSprop(&state, n_var, &lrD, 0.9f, 1e-6f);
	kann_feed_bind(ann, KANN_F_IN, 0, &_x);
	kann_feed_bind(ann, KANN_F_TRUTH, 0, &_y);
	kann_switch(ann, 1);
	kann_set_batch_size(ann, 1);
	memset(ann->g, 0, n_var * sizeof(double));
	char displayMessage = 1;
	unsigned int portionOfEpoch = max_epoch / 1000;
	unsigned int cnt = 0;
	int updateEpoch;
	for (int i = 0; i < max_epoch; ++i)
	{
		double train_cost = kann_cost(ann, 0, 1);
		if (!cnt)
		{
			updateEpoch = 1;
			lrD *= lrDecayRate;
		}
		else
			updateEpoch = 0;
		cnt++;
		if (cnt >= portionOfEpoch)
			cnt = 0;
		kann_RMSprop(&state, ann->g, ann->x, updateEpoch);
		if (displayMessage)
			printf("epoch: %d; training cost: %g\n", i + 1, train_cost);
	}
	freeRMSProp(&state);
	kann_switch(ann, 0);
}
#define TEST_SUBJECT (1)
int main()
{
	srand(21);
	int i, j;
#if TEST_SUBJECT == 0
	// y = log(pow(n1 + x, 0.4 + n2) + n3)
	int n_out = 1;
	kad_trap_fe();
	kann_srand(1337);
	int axis = 2;
	kad_node_t *input = kad_feed(axis, 1, 1);
	input->ext_flag |= KANN_F_IN;
	kad_node_t *NOne = kann_new_leaf(KAD_VAR, 1.0, axis, 1, 1);
	kad_node_t *zeroPt4 = kann_new_leaf(KAD_CONST, 0.4, axis, 1, 1);
	kad_node_t *NTwo = kann_new_leaf(KAD_VAR, 2.2, axis, 1, 1);
	kad_node_t *A = kad_pow(kad_add(NOne, input), kad_add(zeroPt4, NTwo));
	kad_node_t *NThree = kann_new_leaf(KAD_VAR, 0.02, axis, 1, 1);
	kad_node_t *sn = kad_log(kad_add(A, NThree));
	kad_node_t *loss = kann_layer_mse_raw(sn, n_out);
	kann_t *network = kann_new(loss, 0);
	double x = 0.8;
	double y = 0.33;
	kann_gradient_optimizer(network, 0.01, 1.0, 100, &x, &y, 0, 0);
	kann_switch(network, 0);
	const double *res = kann_apply1(network, &x);
	kann_delete(network);
#elif TEST_SUBJECT == 1
	// y = pow(n1 + x, 3)
	int n_out = 1;
	kad_trap_fe();
	kann_srand(1337);
	int axis = 2;
	kad_node_t *input = kad_feed(axis, 1, 1);
	input->ext_flag |= KANN_F_IN;
	kad_node_t *NOne = kann_new_leaf(KAD_VAR, 1.0, axis, 1, 1);
	kad_node_t *three = kann_new_leaf(KAD_CONST, 1.0, axis, 1, 1);
	three->x[0] = 3.0;
	kad_node_t *sn = kad_pow(kad_add(NOne, input), three);
	kad_node_t *loss = kann_layer_mse_raw(sn, n_out);
	kann_t *network = kann_new(loss, 0);
	double x = 0.8;
	double y = 0.33;
	kann_gradient_optimizer(network, 0.1, 1.0, 100, &x, &y, 0, 0);
	kann_switch(network, 0);
	const double *res = kann_apply1(network, &x);
	kann_delete(network);
#elif TEST_SUBJECT == 2
	// y = hypot(n1 + x, n2 + 2)
	int n_out = 1;
	kad_trap_fe();
	kann_srand(1337);
	int axis = 2;
	kad_node_t *input = kad_feed(axis, 1, 1);
	input->ext_flag |= KANN_F_IN;
	kad_node_t *NOne = kann_new_leaf(KAD_VAR, 1.0, axis, 1, 1);
	kad_node_t *Ntwo = kann_new_leaf(KAD_VAR, 1.0, axis, 1, 1);
	kad_node_t *ktwo = kann_new_leaf(KAD_CONST, 2.0, axis, 1, 1);
	kad_node_t *sn = kad_hypot(kad_add(NOne, input), kad_add(Ntwo, ktwo));
	kad_node_t *loss = kann_layer_mse_raw(sn, n_out);
	kann_t *network = kann_new(loss, 0);
	double x = 0.8;
	double y = 0.33;
	kann_gradient_optimizer(network, 0.08, 1.0, 80, &x, &y, 0, 0);
	kann_switch(network, 0);
	const double *res = kann_apply1(network, &x);
	kann_delete(network);
#elif TEST_SUBJECT == 3
	// y = atan2(n1 + x, n2 + 2)
	int n_out = 1;
	kad_trap_fe();
	kann_srand(1337);
	int axis = 2;
	kad_node_t *input = kad_feed(axis, 1, 2);
	input->ext_flag |= KANN_F_IN;
	kad_node_t *NOne = kann_new_leaf(KAD_VAR, 1.0, axis, 1, 2);
	kad_node_t *Ntwo = kann_new_leaf(KAD_VAR, 1.0, axis, 1, 2);
	for (int i = 0; i < 2; i++)
	{
		NOne->x[i] = rand() / 32768.0f;
		Ntwo->x[i] = rand() / 32768.0f;
	}
	kad_node_t *ktwo = kann_new_leaf(KAD_CONST, 2.0, axis, 1, 2);
	kad_node_t *sn = kad_atan2(kad_add(NOne, input), kad_add(Ntwo, ktwo));
	kad_node_t *fn = kad_reduce_mean(sn, 1);
	kad_node_t *loss = kann_layer_mse_raw(fn, n_out);
	kann_t *network = kann_new(loss, 0);
	double x[2] = { 0.8, 0.3 };
	double y = 0.33;
	kann_gradient_optimizer(network, 0.02, 1.0, 100, x, &y, 0, 0);
	kann_switch(network, 0);
	const double *res = kann_apply1(network, &x);
	kann_delete(network);
#elif TEST_SUBJECT == 4
	// y = fmod(n1 + x, n2 + 2)
	int n_out = 1;
	kad_trap_fe();
	kann_srand(1337);
	int axis = 2;
	kad_node_t *input = kad_feed(axis, 1, 1);
	input->ext_flag |= KANN_F_IN;
	kad_node_t *NOne = kann_new_leaf(KAD_VAR, 1.0, axis, 1, 1);
	kad_node_t *Ntwo = kann_new_leaf(KAD_VAR, 1.0, axis, 1, 1);
	kad_node_t *ktwo = kann_new_leaf(KAD_CONST, 2.0, axis, 1, 1);
	kad_node_t *sn = kad_mod(kad_add(NOne, input), kad_add(Ntwo, ktwo));
	kad_node_t *loss = kann_layer_mse_raw(sn, n_out);
	kann_t *network = kann_new(loss, 0);
	double x = 0.8;
	double y = 0.33;
	kann_gradient_optimizer(network, 0.04, 1.0, 100, &x, &y, 0, 0);
	kann_switch(network, 0);
	const double *res = kann_apply1(network, &x);
	kann_delete(network);
#elif TEST_SUBJECT == 5 // ? problematic when learning rate decay isn't possible?
	// y = expint(n1 + x)
	int n_out = 1;
	kad_trap_fe();
	kann_srand(1337);
	int axis = 2;
	kad_node_t *input = kad_feed(axis, 1, 1);
	input->ext_flag |= KANN_F_IN;
	kad_node_t *NOne = kann_new_leaf(KAD_VAR, 1.0, axis, 1, 1);
	kad_node_t *sn = kad_expint(kad_add(input, NOne));
	kad_node_t *loss = kann_layer_mse_raw(sn, n_out);
	kann_t *network = kann_new(loss, 0);
	double x = 0.8;
	double y = 7.33;
	kann_gradient_optimizer(network, 0.00008, 0.998, 100000, &x, &y, 0, 0);
	kann_switch(network, 0);
	const double *res = kann_apply1(network, &x);
	kann_delete(network);
#elif TEST_SUBJECT == 6
	// y = gamma(n1 + x), Differentiable factorial
	int n_out = 1;
	kad_trap_fe();
	kann_srand(1337);
	int axis = 2;
	kad_node_t *input = kad_feed(axis, 1, 1);
	input->ext_flag |= KANN_F_IN;
	kad_node_t *NOne = kann_new_leaf(KAD_VAR, 1.0, axis, 1, 1);
	kad_node_t *sn = kad_gamma(kad_add(input, NOne));
	kad_node_t *loss = kann_layer_mse_raw(sn, n_out);
	kann_t *network = kann_new(loss, 0);
	double x = 0.8;
	double y = 24.0;
	kann_gradient_optimizer(network, 0.005, 1.0, 650, &x, &y, 0, 0);
	kann_switch(network, 0);
	const double *res = kann_apply1(network, &x);
	kann_delete(network);
#elif TEST_SUBJECT == 7
	// y = gammaln(n1 + x)
	int n_out = 1;
	kad_trap_fe();
	kann_srand(1337);
	int axis = 2;
	kad_node_t *input = kad_feed(axis, 1, 1);
	input->ext_flag |= KANN_F_IN;
	kad_node_t *NOne = kann_new_leaf(KAD_VAR, 1.0, axis, 1, 1);
	kad_node_t *sn = kad_gammaln(kad_add(input, NOne));
	kad_node_t *loss = kann_layer_mse_raw(sn, n_out);
	kann_t *network = kann_new(loss, 0);
	double x = 0.8;
	double y = 8.0;
	kann_gradient_optimizer(network, 0.05, 1.0, 160, &x, &y, 0, 0);
	kann_switch(network, 0);
	const double *res = kann_apply1(network, &x);
	kann_delete(network);
#elif TEST_SUBJECT == 8
	// vector * / Scalar
	int n_out = 3;
	kad_trap_fe();
	kann_srand(1337);
	int axis = 2;
	kad_node_t *input = kad_feed(axis, 1, 1);
	input->ext_flag |= KANN_F_IN;
	kad_node_t *weights_and_bias = kann_new_leaf(KAD_VAR, 0.2, axis, 1, 1);
	kad_node_t *NOne = kann_new_leaf(KAD_CONST, 1.0, axis, 1, 3);
	NOne->x[0] = 2.0; NOne->x[1] = 3.0; NOne->x[2] = 4.0;
	kad_node_t *var1 = kad_add(input, weights_and_bias);
	//kad_node_t *sn = kad_mul(var1, NOne);
	//kad_node_t *sn = kad_div(var1, NOne);
	kad_node_t *sn = kad_sub(var1, NOne);
	//kad_node_t *sn = kad_mul(var1, kad_reciprocal(NOne));
	// x + [2, 3, 4] == [2, 3, 4] + x
	// x - [2, 3, 4] == -[2, 3, 4] + x
	// x .* [2, 3, 4] == [2, 3, 4] .* x
	// x ./ [2, 3, 4] == x * (1 ./ [2, 3, 4])
	kad_node_t *loss = kann_layer_mse_raw(sn, n_out);
	kann_t *network = kann_new(loss, 0);
	double x = 0.0;
	double y[3] = { 1.0, 2.0, 3.0 };
	kann_gradient_optimizer(network, 0.05, 1.0, 500, &x, y, 0, 0);
	kann_switch(network, 0);
	const double *res = kann_apply1(network, &x);
	kann_delete(network);
#else
#endif
	return 0;
}