#include "CurveFittingWorker.h"

#include "utils/CurveFittingUtils.h"

#undef __SSE__  // Workaround: FPA crash caused by SSE instructions on Linux
#include <kann.h>


typedef struct
{
    double fs;
    double *phi;
    double *weights;
    unsigned int gridSize;
    double *target;
    unsigned int numBands;
    double *tmp;
    double last_mse;
    void* user_data;
} optUserdata;

CurveFittingWorker::CurveFittingWorker(const CurveFittingOptions& _options, QObject* parent) : QObject(parent), options(_options)
{
    freq =_options.frequencyData();
    target =_options.targetData();
    rng_seed = _options.seed();
    rng_density_func = _options.probabilityDensityFunc();
    array_size = _options.dataCount();
    algorithm_type = _options.algorithmType();
    force_to_oct_grid_conversion = _options.forceLogOctGrid();
    avg_bw = _options.averageBandwidth();

    stage1Epk = _options.iterations();
    stage2Epk = _options.iterationsSecondary();
    stage3Epk = _options.iterationsTertiary();

    lr1 = _options.getLearnRate1();
    lr1DecayRate = _options.getLearnDecayRate1();
    lr2 = _options.getLearnRate2();
    lr2DecayRate = _options.getLearnDecayRate2();
}

CurveFittingWorker::~CurveFittingWorker()
{
    free(targetList);
    free(flt_freqList);
    free(flt_fc);
    free(idx);
    free(dFreqDiscontin);
    free(dif);
    free(flt_peak_g);
    free(initialQ);
    free(phi);
    free(initialAns);
    free(low);
    free(up);
    free(tmpDat);
}

void CurveFittingWorker::optimizationHistoryCallback(void *hostData, unsigned int n, double *currentResult, double *currentFval)
{
    Q_UNUSED(n);
    Q_UNUSED(currentResult);

    optUserdata *userdata = (optUserdata*)hostData;
    CurveFittingWorker *worker = (CurveFittingWorker*)userdata->user_data;

    double *fc = currentResult;
    double *Q = currentResult + userdata->numBands;
    double *gain = currentResult + userdata->numBands * 2;
    double b0, b1, b2, a1, a2;
    memset(userdata->tmp, 0, userdata->gridSize * sizeof(double));
    for (unsigned int i = 0; i < userdata->numBands; i++)
    {
        validatePeaking(gain[i], fc[i], Q[i], userdata->fs, &b0, &b1, &b2, &a1, &a2);
        validateMagCal(b0, b1, b2, a1, a2, userdata->phi, userdata->gridSize, userdata->fs, userdata->tmp);
    }

    // Determine if result changed using fuzzy compare
    if(!isApproximatelyEqual<double>(userdata->last_mse, *currentFval))
    {
        emit worker->mseReceived(*currentFval);
        emit worker->graphReceived(std::vector<double>(userdata->tmp, userdata->tmp + userdata->gridSize));
    }

    userdata->last_mse = *currentFval;
}

double CurveFittingWorker::peakingCostFunctionMap(double *x, void *usd)
{
    optUserdata *userdata = (optUserdata*)usd;
    double *fc = x;
    double *Q = x + userdata->numBands;
    double *gain = x + userdata->numBands * 2;
    double b0, b1, b2, a1, a2;
    memset(userdata->tmp, 0, userdata->gridSize * sizeof(double));

    for (unsigned int i = 0; i < userdata->numBands; i++)
    {
        validatePeaking(gain[i], fc[i], Q[i], userdata->fs, &b0, &b1, &b2, &a1, &a2);
        validateMagCal(b0, b1, b2, a1, a2, userdata->phi, userdata->gridSize, userdata->fs, userdata->tmp);
    }

    double meanAcc = 0.0;
    for (unsigned int i = 0; i < userdata->gridSize; i++)
    {
        double error = userdata->tmp[i] - userdata->target[i];
        meanAcc += error * error;
    }
    meanAcc = meanAcc / (double)userdata->gridSize;
    return meanAcc;
}

void CurveFittingWorker::preprocess(double *&flt_freqList, double *&target, uint& array_size, int fs, bool force_to_oct_grid_conversion, double avg_bw, bool* is_nonuniform, bool invert){
    uint i;

    if(invert){
        for(uint i = 0; i < array_size; i++)
            target[i] = -target[i];
    }

    // Detect X axis linearity
    // Assume frequency axis is sorted
    double first = flt_freqList[0];
    double last = flt_freqList[array_size - 1];
    double stepLinspace = (last - first) / (double)(array_size - 1);
    double residue = 0.0;
    for (i = 0; i < array_size; i++)
    {
        double vv = fabs((first + i * stepLinspace) - flt_freqList[i]);
        residue += vv;
    }
    residue = residue / (double)array_size;
    char gdType;
    if (residue > 10.0) // Allow margin of error, in fact, non-zero residue hint the grid is nonuniform
    {
        gdType = 1;
    }
    else
    {
        gdType = 0;
    }

    if(is_nonuniform != nullptr){
        *is_nonuniform = gdType == 1;
    }

    // Convert uniform grid to log grid is recommended
    // Nonuniform grid doesn't necessary to be a log grid, but we can force to make it log
    if (force_to_oct_grid_conversion)
    {
        unsigned int oGridSize;
        double *out1;
        double *out2;
        smoothSpectral(gdType, flt_freqList, target, array_size, avg_bw, fs, &oGridSize, &out1, &out2);
        free(flt_freqList);
        free(target);
        flt_freqList = out1;
        target = out2;
    }
}

double buildDAGNodesDifferentiate(pcg32x2_random_t *PRNG, double *gbest, double *target, double *phi, double *weights, int n_bands, int n_out, double fs, int lockFc, unsigned int iterations, double learningRate, double lrDecayRate, double *low, double *up, void(*optStatus)(void*, unsigned int, double*, double*), void *userdataPtr)
{
    optUserdata *userdata = (optUserdata*)userdataPtr;
    int i, j;
    kad_trap_fe();
    kann_srand(1337);
    int axis = 2; // n_bands * n_out matrix is all we need
    kad_node_t *fsNode = kad_feed(axis, 1, 1);
    fsNode->ext_flag |= KANN_F_IN;
    // n_bands filters, n_out frequency points
    kad_node_t *one = kann_new_leaf(KAD_CONST, 1.0, axis, 1, 1);
    kad_node_t *two = kann_new_leaf(KAD_CONST, 2.0, axis, 1, 1);
    kad_node_t *four = kann_new_leaf(KAD_CONST, 4.0, axis, 1, 1);
    kad_node_t *tenlogTen = kann_new_leaf(KAD_CONST, 10.0 * (1.0 / log(10.0)), axis, 1, 1);
    kad_node_t *minustwo = kann_new_leaf(KAD_CONST, -2.0, axis, 1, 1);
    kad_node_t *twopiArrayConst = kann_new_leaf(KAD_CONST, 2.0 * M_PI, axis, n_bands, 1);
    kad_node_t *fc = kann_new_leaf(lockFc ? KAD_CONST : KAD_VAR, 0.0, axis, n_bands, 1);
    for (i = 0; i < n_bands; i++)
        fc->x[i] = gbest[i];
    kad_node_t *Q = kann_new_leaf(KAD_VAR, 0.0, axis, n_bands, 1);
    for (i = 0; i < n_bands; i++)
        Q->x[i] = gbest[i + n_bands];
    kad_node_t *gain = kann_new_leaf(KAD_VAR, 0.0, axis, n_bands, 1);
    for (i = 0; i < n_bands; i++)
        gain->x[i] = gbest[i + n_bands + n_bands];
    kad_node_t *pregainConst = kann_new_leaf(KAD_CONST, (double)(1.0 / 40.0), axis, n_bands, 1);
    /*kad_node_t *tenConst = kann_new_leaf(KAD_CONST, 10.0f, axis, n_bands, 1);
    kad_node_t *A = kad_pow(tenConst, kad_mul(gain, pregainConst));
    kad_node_t *t2 = kad_div(kad_pow(tenConst, fc), fsNode); // (10.0 .^ fc) ./ fs*/
    kad_node_t *A = kad_pow10x(kad_mul(gain, pregainConst));
    kad_node_t *t2 = kad_div(kad_pow10x(fc), fsNode); // (10.0 .^ fc) ./ fsNode
    kad_node_t *w0 = kad_mul(twopiArrayConst, t2);
    kad_node_t *sn = kad_sin(w0);
    kad_node_t *cn = kad_cos(w0);
    kad_node_t *alpha = kad_div(sn, kad_mul(two, Q));
    kad_node_t *a0_pk = kad_add(one, kad_div(alpha, A));
    kad_node_t *a1 = kad_div(kad_mul(minustwo, cn), a0_pk);
    kad_node_t *a2 = kad_div(kad_sub(one, kad_div(alpha, A)), a0_pk);
    kad_node_t *b0 = kad_div(kad_add(one, kad_mul(alpha, A)), a0_pk);
    kad_node_t *b1 = kad_div(kad_mul(minustwo, cn), a0_pk);
    kad_node_t *b2 = kad_div(kad_sub(one, kad_mul(alpha, A)), a0_pk);

    kad_node_t *term1 = kad_square(kad_add(kad_add(b0, b1), b2));
    kad_node_t *term3 = kad_mul(b1, kad_add(b0, b2));
    kad_node_t *term4 = kad_mul(four, kad_mul(b0, b2));
    kad_node_t *term34 = kad_add(term3, term4);
    kad_node_t *term5 = kad_square(kad_add(one, kad_add(a1, a2)));
    kad_node_t *term7 = kad_mul(a1, kad_add(one, a2));
    kad_node_t *term8 = kad_mul(four, a2);
    kad_node_t *term78 = kad_add(term7, term8);
    kad_node_t *b0_b2 = kad_mul(b0, b2);
    kad_node_t *phi_1d = kann_new_leaf(KAD_CONST, 0.0, axis, 1, n_out);
    for (j = 0; j < n_out; j++)
        phi_1d->x[j] = phi[j];
    kad_node_t **prem1 = (kad_node_t**)malloc(n_bands * sizeof(kad_node_t*));
    for (i = 0; i < n_bands; i++)
    {
        kad_node_t *selectedSlice1 = kad_slice(b0_b2, 0, i, i + 1);
        kad_node_t *term2 = kad_mul(phi_1d, selectedSlice1);
        kad_node_t *selectedSlice2 = kad_slice(term34, 0, i, i + 1);
        kad_node_t *term234phi = kad_mul(kad_sub(term2, selectedSlice2), phi_1d);
        kad_node_t *selectedSlice3 = kad_slice(term1, 0, i, i + 1);
        kad_node_t *term1234phi = kad_log(kad_add(term234phi, selectedSlice3));

        kad_node_t *selectedSlice5 = kad_slice(a2, 0, i, i + 1);
        kad_node_t *term6 = kad_mul(phi_1d, selectedSlice5);
        kad_node_t *selectedSlice6 = kad_slice(term78, 0, i, i + 1);
        kad_node_t *term678phi = kad_mul(kad_sub(term6, selectedSlice6), phi_1d);
        kad_node_t *selectedSlice7 = kad_slice(term5, 0, i, i + 1);
        kad_node_t *term5678phi = kad_log(kad_add(term678phi, selectedSlice7));
        prem1[i] = kad_sub(term1234phi, term5678phi);
    }
    kad_node_t *termX = kad_mul(tenlogTen, kad_concat_array(0, n_bands, prem1));
    free(prem1);
    kad_node_t *weightVector = kann_new_leaf(KAD_CONST, 0.0, axis, 1, n_out);
    for (j = 0; j < n_out; j++)
        weightVector->x[j] = weights[j];
    kad_node_t *eq_op = kad_mul(kad_reduce_sum(termX, 0), weightVector);
    //kad_node_t *eq_op = kad_reduce_sum(termX, 0);
    kad_node_t *loss = kann_layer_mse_raw(eq_op, n_out); // Getting out the summed matrix
    //kad_node_t *loss = kann_layer_mse_raw(termX, n_bands * n_out); // Getting out the internal matrix
    kann_t *network = kann_new(loss, 0);
    // Print matrix
    /*
    const double *res = kann_apply1(network, &fs);
    fp = fopen("dbg.txt", "wb");
    for (i = 0; i < n_bands; i++)
    {
        for (j = 0; j < n_out; j++)
            fprintf(fp, "%1.7f, ", res[i * n_out + j]);
        fprintf(fp, "\n");
    }*/
    /*// Print vector
    fp = fopen("dbg.txt", "wb");
    for (i = 0; i < n_out; i++)
        fprintf(fp, "%1.8f\n", res[i]);*/
    // Parameters
    double *pointerToGainGradient = gain->g;
    double *pointerToQGradient = Q->g;
    double *pointerTofcGradient = fc->g;
    double *pointerToGainVector = gain->x;
    double *pointerToQVector = Q->x;
    double *pointerTofcVector = fc->x;
    int n_var = kann_size_var(network);
    int n_const = kann_size_const(network);
    double lrD = learningRate;
    grad_optimizer state;
    kann_InitRMSprop(&state, n_var, &lrD, 0.9f, 1e-6f);
    double *_x = &fs;
    double *_y = target;
    kann_feed_bind(network, KANN_F_IN, 0, &_x);
    kann_feed_bind(network, KANN_F_TRUTH, 0, &_y);
    kann_switch(network, 1);
    kann_set_batch_size(network, 1);
    memset(network->g, 0, n_var * sizeof(double));
    double *lowestCost = (double*)malloc((lockFc ? (n_var + n_bands) : n_var) * sizeof(double));
    char displayMessage = 0;
    unsigned int portionOfEpoch = iterations / 1000;
    unsigned int cnt = 0;
    double train_cost = DBL_MAX;
    double previousCost;
    for (unsigned int i = 0; i < iterations; ++i)
    {
        previousCost = train_cost;
        train_cost = kann_cost(network, 0, 1);
        if (!cnt)
            lrD *= lrDecayRate;
        cnt++;
        if (cnt >= portionOfEpoch)
            cnt = 0;
        for (j = 0; j < n_bands; j++)
        {
            if (pointerTofcGradient)
            {
                if (isnan(pointerTofcGradient[j]) || isinf(pointerTofcGradient[j]))
                    pointerTofcGradient[j] = 0;
            }
            if (isnan(pointerToQGradient[j]) || isinf(pointerToQGradient[j]))
                pointerToQGradient[j] = 0;
            if (isnan(pointerToGainGradient[j]) || isinf(pointerToGainGradient[j]))
                pointerToGainGradient[j] = 0;
        }
        kann_RMSprop(&state, network->g, network->x);
        if (low && up)
        {
            for (j = 0; j < n_bands; j++)
            {
                if (pointerTofcVector[j] < low[j])
                    pointerTofcVector[j] = c_rand(PRNG) * (up[j] - low[j]) + low[j];
                if (pointerTofcVector[j] > up[j])
                    pointerTofcVector[j] = c_rand(PRNG) * (up[j] - low[j]) + low[j];
                if (pointerToQVector[j] < low[j + n_bands])
                    pointerToQVector[j] = low[j + n_bands];
                if (pointerToQVector[j] > up[j + n_bands])
                    pointerToQVector[j] = up[j + n_bands];
            }
        }
        if (train_cost < previousCost)
        {
            for (j = 0; j < n_bands; j++)
            {
                lowestCost[j] = fc->x[j];
                lowestCost[j + n_bands] = Q->x[j];
                lowestCost[j + n_bands + n_bands] = gain->x[j];
            }
            if (optStatus)
                optStatus(userdataPtr, n_out, lowestCost, &train_cost);
        }
        if (displayMessage)
            printf("epoch: %d; training cost: %g\n", i + 1, train_cost);
    }
    freeRMSProp(&state);
    memcpy(gbest, lowestCost, (lockFc ? (n_var + n_bands) : n_var) * sizeof(double));
    free(lowestCost);
    kann_switch(network, 0);
    kann_delete(network);
    return train_cost;
}

void CurveFittingWorker::run()
{
    pcg32x2_random_t PRNG;
    pcg32x2_srandom_r(&PRNG, rng_seed, rng_seed >> 2,
                      rng_seed >> 4, rng_seed >> 6);
    unsigned int finess = 2057;
    double px[7] = { -0.3, 0.0, 0.1, 0.3, 0.5, 0.9, 1.0 };
    double prob[7] = { 10.0, 15, 8, 7, 6, 3, 0.2 };
    double *cdf = (double*)malloc(finess * sizeof(double));
    double *pxi = (double*)malloc(finess * sizeof(double));
    arbitraryPDF(px, prob, 7, cdf, pxi, finess);

    unsigned int i, j;
    unsigned int K = options.populationK();
    unsigned int N = options.populationN();
    double fs = 44100.0;

    flt_freqList = (double*)malloc(array_size * sizeof(double));
    memcpy(flt_freqList, freq, array_size * sizeof(double));
    targetList = (double*)malloc(array_size * sizeof(double));
    memcpy(targetList, target, array_size * sizeof(double));

    preprocess(flt_freqList, targetList, array_size, fs, force_to_oct_grid_conversion, avg_bw, nullptr, options.invertGain());

    // Bound constraints
    double lowFc = 10; // Hz
    double upFc = fs / 2 - 1; // Hz
    double lowQ = 0.01; // 0.01 - 1000, higher == shaper the filter
    double upQ = 512; // 0.01 - 1000, higher == shaper the filter
    double lowGain = targetList[0]; // dB
    double upGain = targetList[0]; // dB

    for (i = 1; i < array_size; i++)
    {
        if (targetList[i] < lowGain)
            lowGain = targetList[i];
        if (targetList[i] > upGain)
            upGain = targetList[i];
    }
    lowGain -= 32.0;
    upGain += 32.0;

    // Parameter estimation
    unsigned int numMaximas, numMinimas;
    unsigned int* maximaIndex = peakfinder_wrapper(targetList, array_size, 0.1, 1, &numMaximas);
    unsigned int* minimaIndex = peakfinder_wrapper(targetList, array_size, 0.1, 0, &numMinimas);
    unsigned int numBands = numMaximas + numMinimas;

    // Model complexity code start
    float modelComplexity = options.modelComplexity(); // 10% - 120%
    unsigned int oldBandNum = numBands;
    numBands = roundf(numBands * modelComplexity / 100.0f);
    double *flt_fc = (double*)malloc(numBands * sizeof(double));
    for (i = 0; i < numMaximas; i++)
        flt_fc[i] = flt_freqList[maximaIndex[i]];
    for (i = numMaximas; i < ((numBands < oldBandNum) ? numBands : oldBandNum); i++)
        flt_fc[i] = flt_freqList[minimaIndex[i - numMaximas]];
    if (numBands > oldBandNum)
    {
        for (i = oldBandNum; i < numBands; i++)
            flt_fc[i] = c_rand(&PRNG) * (upFc - lowFc) + lowFc;
    }
    free(maximaIndex);
    free(minimaIndex);

    double *weights = (double*)malloc(array_size * sizeof(double));
    for (i = 0; i < array_size; i++)
    {
        if (fabs(target[i]) > 1.0)
            weights[i] = 1.0;
        else
            weights[i] = 0.2;
    }
    if (numBands > oldBandNum)
    {
        for (i = oldBandNum; i < numBands; i++)
            flt_fc[i] = c_rand(&PRNG) * (upFc - lowFc) + lowFc;
    }

    // Model complexity code end
    unsigned int *idx = (unsigned int*)malloc(numBands * sizeof(unsigned int));
    sort(flt_fc, numBands, idx);

    // Initial value must not get out-of-bound, more importantly.
    // If value go too extreme, NaN will happens when frequency place on either too close to DC and touching/beyond Nyquist
    for (i = 0; i < numBands; i++)
    {
        if (flt_fc[i] < lowFc)
            flt_fc[i] = lowFc;
        if (flt_fc[i] > upFc)
            flt_fc[i] = upFc;
    }
    // Naive way to prevent nearby filters go too close, because they could contribute nothing
    double smallestJump = 0.0;
    double lowestFreq2Gen = 200.0;
    double highestFreq2Gen = 14000.0;
    dFreqDiscontin = (double*)malloc(numBands * sizeof(double));
    dif = (double*)malloc(numBands * sizeof(double));
    unsigned int cnt = 0;
    while (smallestJump <= 20.0)
    {
        derivative(flt_fc, numBands, 1, dFreqDiscontin, dif);
        unsigned int smIdx;
        smallestJump = minArray(dFreqDiscontin, numBands, &smIdx);
        double newFreq = c_rand(&PRNG) * (highestFreq2Gen - lowestFreq2Gen) + lowestFreq2Gen;
        flt_fc[smIdx] = newFreq;
        sort(flt_fc, numBands, idx);

        cnt++;
        if (cnt > 50)
            break;
    }
    free(idx);
    flt_peak_g = (double*)malloc(numBands * sizeof(double));
    for (i = 0; i < numBands; i++)
        flt_peak_g[i] = npointWndFunction(flt_fc[i], flt_freqList, targetList, array_size);
    initialQ = (double*)malloc(numBands * sizeof(double));
    for (i = 0; i < numBands; i++)
    {
        initialQ[i] = fabs(randn_pcg32x2(&PRNG) * (5 - 0.7) + 0.7);
        flt_fc[i] = log10(flt_fc[i]);
    }

    phi = (double*)malloc(array_size * sizeof(double));
    for (i = 0; i < array_size; i++)
    {
        double term1 = sin(M_PI * flt_freqList[i] / fs);
        phi[i] = 4.0 * term1 * term1;
    }

    unsigned int dim = numBands * 3;   
    low = (double*)malloc(dim * sizeof(double));
    up = (double*)malloc(dim * sizeof(double));
    for (j = 0; j < numBands; j++)
    {
        low[j] = log10(lowFc); low[numBands + j] = lowQ; low[numBands * 2 + j] = lowGain;
        up[j] = log10(upFc); up[numBands + j] = upQ; up[numBands * 2 + j] = upGain;
    }

    double *gbest = (double*)malloc(numBands * 3 * sizeof(double));
    idx = (unsigned int*)malloc(numBands * sizeof(unsigned int));
    double *sortedOptVector = (double*)malloc(numBands * 3 * sizeof(double));

    // Local minima avoidance
    double initialLowGain = -1.5;
    double initialUpGain = 1.5;
    double initialLowQ = -0.5;
    double initialUpQ = 0.5;
    double initialLowFc = -log10(2);
    double initialUpFc = log10(2);
    initialAns = (double*)malloc(K * N * dim * sizeof(double));
    for (i = 0; i < K * N; i++)
    {
        for (j = 0; j < numBands; j++)
        {
            initialAns[i * dim + j] = flt_fc[j] + c_rand(&PRNG) * (initialUpFc - initialLowFc) + initialLowFc;
            initialAns[i * dim + numBands + j] = initialQ[j] + c_rand(&PRNG) * (initialUpQ - initialLowQ) + initialLowQ;
            initialAns[i * dim + numBands * 2 + j] = flt_peak_g[j] + c_rand(&PRNG) * (initialUpGain - initialLowGain) + initialLowGain;
        }
    }
    for (i = 0; i < numBands; i++)
    {
        gbest[i] = flt_fc[i];
        gbest[i + numBands] = initialQ[i];
        gbest[i + numBands + numBands] = flt_peak_g[i];
    }

    // Cost function data setup
    tmpDat = (double*)malloc(array_size * sizeof(double));
    optUserdata userdat;
    userdat.fs = fs;
    userdat.numBands = numBands;
    userdat.phi = phi;
    userdat.target = targetList;
    userdat.tmp = tmpDat;
    userdat.last_mse = -1;
    userdat.gridSize = array_size;
    userdat.user_data = this;
    userdat.weights = weights;
    void *userdataPtr = (void*)&userdat;

    // Select probability distribution function
    double(*pdf1)(pcg32x2_random_t*) = randn_pcg32x2;
    switch(rng_density_func){
    case CurveFittingOptions::PDF_RANDN_PCG32X2:
        pdf1 = randn_pcg32x2;
        break;
    case CurveFittingOptions::PDF_RAND_TRI_PCG32X2:
        pdf1 = rand_tri_pcg32x2;
        break;
    case CurveFittingOptions::PDF_RAND_HANN:
        pdf1 = rand_hann;
        break;
    }

    // Create optimization history callback
    void *hist_userdata = (void*)userdataPtr;
    void(*optStatus)(void*, unsigned int, double*, double*) = optimizationHistoryCallback;

    // Calculate
    switch(algorithm_type){
    // DE is relatively robust, but require a lot iteration to converge, we then improve DE result using fminsearch
    // fminsearchbnd can be a standalone algorithm, but would high dimension or even some simple curve
    // but overall fminsearchbnd converge faster than other 2 algorithms in current library for current fitting purpose
    case CurveFittingOptions::AT_DIFF_EVOLUTION: {
        emit stageChanged(1, algorithm_type);
        double gmin = differentialEvolution(peakingCostFunctionMap, userdataPtr, initialAns, K, N, options.deProbiBound(), dim, low, up, stage1Epk, gbest, &PRNG, pdf1, optStatus, hist_userdata);
        qDebug("CurveFittingThread: gmin=%lf", gmin);
        break;
    }
    case CurveFittingOptions::AT_CHIO: {
        emit stageChanged(1, algorithm_type);
        double fval = CHIO(peakingCostFunctionMap, userdataPtr, initialAns, K * N, options.chioMaxSolSurviveEpoch(), options.chioC0(), options.chioSpreadingRate(), dim, low, up, stage1Epk, gbest, &PRNG, optStatus, hist_userdata);
        qDebug("CurveFittingThread: fval=%lf", fval);
        break;
    }
    case CurveFittingOptions::AT_SGD: {
        emit stageChanged(1, CurveFittingOptions::AT_SGD);
        double cost1 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, numBands, array_size, fs, 1, stage1Epk, lr1, lr1DecayRate, low, up, optStatus, userdataPtr);
        qDebug("CurveFittingThread: SGD stage 1 cost: %1.8lf", cost1);
        emit stageChanged(2, CurveFittingOptions::AT_SGD);
        double cost2 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, numBands, array_size, fs, 0, stage2Epk, lr2, lr2DecayRate, low, up, optStatus, userdataPtr);
        qDebug("CurveFittingThread: SGD stage 2 cost: %1.8lf", cost2);
        break;
    }
    case CurveFittingOptions::AT_HYBRID_SGD_DE: {
        emit stageChanged(1, CurveFittingOptions::AT_SGD);
        double cost1 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, numBands, array_size, fs, 1, stage1Epk, lr1, lr1DecayRate, low, up, optStatus, userdataPtr);
        qDebug("CurveFittingThread: SGD stage 1 cost: %1.8lf", cost1);
        emit stageChanged(2, CurveFittingOptions::AT_SGD);
        double cost2 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, numBands, array_size, fs, 0, stage2Epk, lr2, lr2DecayRate, low, up, optStatus, userdataPtr);
        qDebug("CurveFittingThread: SGD stage 2 cost: %1.8lf", cost2);
        for (j = 0; j < numBands; j++)
        {
            initialAns[j] = gbest[j];
            initialAns[numBands + j] = gbest[j + numBands];
            initialAns[numBands * 2 + j] = gbest[j + numBands * 2];
        }
        for (i = 1; i < K * N; i++)
        {
            for (j = 0; j < numBands; j++)
            {
                initialAns[i * dim + j] = gbest[j] + c_rand(&PRNG) * (initialUpFc - initialLowFc) + initialLowFc;
                initialAns[i * dim + numBands + j] = gbest[j + numBands] + c_rand(&PRNG) * (initialUpQ - initialLowQ) + initialLowQ;
                initialAns[i * dim + numBands * 2 + j] = gbest[j + numBands * 2] + c_rand(&PRNG) * (initialUpGain - initialLowGain) + initialLowGain;
            }
        }
        emit stageChanged(3, CurveFittingOptions::AT_DIFF_EVOLUTION);
        double deFval = differentialEvolution(peakingCostFunctionMap, userdataPtr, initialAns, K, N, 1.0, dim, low, up, stage3Epk, gbest, &PRNG, pdf1, optStatus, userdataPtr);
        qDebug("CurveFittingThread: DE cost: %1.8lf", deFval);
        break;
    }
    case CurveFittingOptions::AT_HYBRID_SGD_CHIO: {
        emit stageChanged(1, CurveFittingOptions::AT_SGD);
        double cost1 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, numBands, array_size, fs, 1, stage1Epk, lr1, lr1DecayRate, low, up, optStatus, userdataPtr);
        qDebug("CurveFittingThread: SGD stage 1 cost: %1.8lf", cost1);
        emit stageChanged(2, CurveFittingOptions::AT_SGD);
        double cost2 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, numBands, array_size, fs, 0, stage2Epk, lr2, lr2DecayRate, low, up, optStatus, userdataPtr);
        qDebug("CurveFittingThread: SGD stage 2 cost: %1.8lf", cost2);
        for (j = 0; j < numBands; j++)
        {
            initialAns[j] = gbest[j];
            initialAns[numBands + j] = gbest[j + numBands];
            initialAns[numBands * 2 + j] = gbest[j + numBands * 2];
        }
        for (i = 1; i < K * N; i++)
        {
            for (j = 0; j < numBands; j++)
            {
                initialAns[i * dim + j] = gbest[j] + c_rand(&PRNG) * (initialUpFc - initialLowFc) + initialLowFc;
                initialAns[i * dim + numBands + j] = gbest[j + numBands] + c_rand(&PRNG) * (initialUpQ - initialLowQ) + initialLowQ;
                initialAns[i * dim + numBands * 2 + j] = gbest[j + numBands * 2] + c_rand(&PRNG) * (initialUpGain - initialLowGain) + initialLowGain;
            }
        }
        emit stageChanged(3, CurveFittingOptions::AT_CHIO);
        double fval = CHIO(peakingCostFunctionMap, userdataPtr, initialAns, K * N, options.chioMaxSolSurviveEpoch(), options.chioC0(), options.chioSpreadingRate(), dim, low, up, stage3Epk, gbest, &PRNG, optStatus, userdataPtr);
        qDebug("CurveFittingThread: CHIO cost: %1.8lf", fval);
        break;
    }
    }

    sort(gbest, numBands, idx);
    for (i = 0; i < numBands; i++)
    {
        sortedOptVector[i] = gbest[i];
        sortedOptVector[numBands + i] = gbest[numBands + idx[i]];
        sortedOptVector[numBands + numBands + i] = gbest[numBands + numBands + idx[i]];
    }
    free(gbest);
    free(idx);

    // Serialize results
    double *fc = sortedOptVector;
    double *q = sortedOptVector + numBands;
    double *gain = sortedOptVector + numBands * 2;
    for(uint i = 0; i < numBands; i++)
    {
        double trueFreq = pow(10.0, fc[i]);
        double omega = 2.0 * M_PI * trueFreq / fs;
        double bw = (asinh(1.0 / (2.0 * q[i])) * sin(omega)) / (log(2.0) / 2.0 * omega);
        results.append(DeflatedBiquad(FilterType::PEAKING, trueFreq, bw, gain[i]));
    }
    free(sortedOptVector);

    emit finished();
}

QVector<DeflatedBiquad> CurveFittingWorker::getResults() const
{
    return results;
}
