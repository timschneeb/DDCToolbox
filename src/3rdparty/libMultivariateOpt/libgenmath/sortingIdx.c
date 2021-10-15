static void merge(unsigned int idx_data[], double x_data[], int offset, int np, int nq, unsigned int iwork_data[], double xwork_data[])
{
	int exitg1;
	int iout;
	int j;
	int n_tmp;
	int p;
	int q;
	if (nq != 0) {
		n_tmp = np + nq;
		for (j = 0; j < n_tmp; j++) {
			iout = offset + j;
			iwork_data[j] = idx_data[iout];
			xwork_data[j] = x_data[iout];
		}

		p = 0;
		q = np;
		iout = offset - 1;
		do {
			exitg1 = 0;
			iout++;
			if (xwork_data[p] <= xwork_data[q]) {
				idx_data[iout] = iwork_data[p];
				x_data[iout] = xwork_data[p];
				if (p + 1 < np) {
					p++;
				}
				else {
					exitg1 = 1;
				}
			}
			else {
				idx_data[iout] = iwork_data[q];
				x_data[iout] = xwork_data[q];
				if (q + 1 < n_tmp) {
					q++;
				}
				else {
					q = iout - p;
					for (j = p + 1; j <= np; j++) {
						iout = q + j;
						idx_data[iout] = iwork_data[j - 1];
						x_data[iout] = xwork_data[j - 1];
					}

					exitg1 = 1;
				}
			}
		} while (exitg1 == 0);
	}
}
void merge_block(unsigned int idx_data[], double x_data[], int n, unsigned int iwork_data[], double xwork_data[])
{
	int bLen;
	int nPairs;
	int nTail;
	int tailOffset;
	nPairs = n >> 2;
	bLen = 1 << 2;
	while (nPairs > 1) {
		if ((nPairs & 1) != 0) {
			nPairs--;
			tailOffset = bLen * nPairs;
			nTail = n - tailOffset;
			if (nTail > bLen)
				merge(idx_data, x_data, tailOffset, bLen, nTail - bLen, iwork_data, xwork_data);
		}

		tailOffset = bLen << 1;
		nPairs >>= 1;
		for (nTail = 0; nTail < nPairs; nTail++)
			merge(idx_data, x_data, nTail * tailOffset, bLen, bLen, iwork_data, xwork_data);

		bLen = tailOffset;
	}

	if (n > bLen) {
		merge(idx_data, x_data, 0, bLen, n - bLen, iwork_data, xwork_data);
	}
}
void sort(double x_data[], const unsigned int xSize, unsigned int idx_data[])
{
	if (xSize == 0)
		return;
	if (xSize == 1)
	{
		idx_data[0] = 0;
		return;
	}
	double vwork_data[3777];
	double xwork_data[3777];
	unsigned int iidx_data[3777];
	unsigned int iwork_data[3777];
	double x4[4];
	unsigned int idx4[4];
	signed char perm[4];
	double d;
	double d1;
	int dim;
	int i3;
	int i4;
	int ib;
	unsigned int k;

	for (k = 0; k < xSize; k++)
		vwork_data[k] = x_data[k];

	for (dim = 0; dim < 4; dim++) {
		x4[dim] = 0.0;
		idx4[dim] = 0;
	}

	ib = 0;
	for (k = 0; k < xSize; k++) {
		ib++;
		idx4[ib - 1] = (unsigned int)(k + 1);
		x4[ib - 1] = vwork_data[k];
		if (ib == 4) {
			if (x4[0] <= x4[1]) {
				dim = 1;
				ib = 2;
			}
			else {
				dim = 2;
				ib = 1;
			}

			if (x4[2] <= x4[3]) {
				i3 = 3;
				i4 = 4;
			}
			else {
				i3 = 4;
				i4 = 3;
			}

			d = x4[dim - 1];
			d1 = x4[i3 - 1];
			if (d <= d1) {
				d = x4[ib - 1];
				if (d <= d1) {
					perm[0] = (signed char)dim;
					perm[1] = (signed char)ib;
					perm[2] = (signed char)i3;
					perm[3] = (signed char)i4;
				}
				else if (d <= x4[i4 - 1]) {
					perm[0] = (signed char)dim;
					perm[1] = (signed char)i3;
					perm[2] = (signed char)ib;
					perm[3] = (signed char)i4;
				}
				else {
					perm[0] = (signed char)dim;
					perm[1] = (signed char)i3;
					perm[2] = (signed char)i4;
					perm[3] = (signed char)ib;
				}
			}
			else {
				d1 = x4[i4 - 1];
				if (d <= d1) {
					if (x4[ib - 1] <= d1) {
						perm[0] = (signed char)i3;
						perm[1] = (signed char)dim;
						perm[2] = (signed char)ib;
						perm[3] = (signed char)i4;
					}
					else {
						perm[0] = (signed char)i3;
						perm[1] = (signed char)dim;
						perm[2] = (signed char)i4;
						perm[3] = (signed char)ib;
					}
				}
				else {
					perm[0] = (signed char)i3;
					perm[1] = (signed char)i4;
					perm[2] = (signed char)dim;
					perm[3] = (signed char)ib;
				}
			}

			iidx_data[k - 3] = idx4[perm[0] - 1];
			iidx_data[k - 2] = idx4[perm[1] - 1];
			iidx_data[k - 1] = idx4[perm[2] - 1];
			iidx_data[k] = idx4[perm[3] - 1];
			vwork_data[k - 3] = x4[perm[0] - 1];
			vwork_data[k - 2] = x4[perm[1] - 1];
			vwork_data[k - 1] = x4[perm[2] - 1];
			vwork_data[k] = x4[perm[3] - 1];
			ib = 0;
		}
	}

	if (ib > 0) {
		for (dim = 0; dim < 4; dim++) {
			perm[dim] = 0;
		}

		if (ib == 1) {
			perm[0] = 1;
		}
		else if (ib == 2) {
			if (x4[0] <= x4[1]) {
				perm[0] = 1;
				perm[1] = 2;
			}
			else {
				perm[0] = 2;
				perm[1] = 1;
			}
		}
		else if (x4[0] <= x4[1]) {
			if (x4[1] <= x4[2]) {
				perm[0] = 1;
				perm[1] = 2;
				perm[2] = 3;
			}
			else if (x4[0] <= x4[2]) {
				perm[0] = 1;
				perm[1] = 3;
				perm[2] = 2;
			}
			else {
				perm[0] = 3;
				perm[1] = 1;
				perm[2] = 2;
			}
		}
		else if (x4[0] <= x4[2]) {
			perm[0] = 2;
			perm[1] = 1;
			perm[2] = 3;
		}
		else if (x4[1] <= x4[2]) {
			perm[0] = 2;
			perm[1] = 3;
			perm[2] = 1;
		}
		else {
			perm[0] = 3;
			perm[1] = 2;
			perm[2] = 1;
		}

		for (k = 0; k < ib; k++) {
			i3 = perm[k] - 1;
			iidx_data[(xSize - ib) + k] = idx4[i3];
			vwork_data[(xSize - ib) + k] = x4[i3];
		}
	}

	merge_block(iidx_data, vwork_data, xSize, iwork_data, xwork_data);

	for (k = 0; k < xSize; k++) {
		x_data[k] = vwork_data[k];
		idx_data[k] = iidx_data[k] - 1;
	}
}