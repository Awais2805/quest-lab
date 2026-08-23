#include "quest.h"
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>

/* Each rank prints in turn, separated by barriers, so the output doesn't
   interleave into noise. */
static void dump(Qureg q, const char *label) {
    if (q.rank == 0) printf("\n===== %s =====\n", label);
    for (int r = 0; r < q.numNodes; r++) {
        syncQuESTEnv();
        if (q.rank != r) continue;

        /* QuEST splits on the HIGH-order bits, so rank r owns a contiguous
           block of global indices starting at r * numAmpsPerNode. */
        qindex first = (qindex) q.rank * q.numAmpsPerNode;
        printf("rank %2d owns global [%lld .. %lld]  (%lld amps, %.2f MiB)\n",
               q.rank, (long long) first,
               (long long) (first + q.numAmpsPerNode - 1),
               (long long) q.numAmpsPerNode,
               q.numAmpsPerNode * (double) sizeof(qcomp) / 1048576.0);

        for (qindex i = 0; i < q.numAmpsPerNode; i++) {
            double re = creal(q.cpuAmps[i]), im = cimag(q.cpuAmps[i]);
            if (re*re + im*im > 1e-12)          /* only nonzero amplitudes */
                printf("    local[%lld] -> global[%lld] = % .4f %+.4fi\n",
                       (long long) i, (long long) (first + i), re, im);
        }
        fflush(stdout);
    }
    syncQuESTEnv();
}

int main(int argc, char **argv) {
    initQuESTEnv();
    int n = (argc > 1) ? atoi(argv[1]) : 6;

    Qureg q = createForcedQureg(n);
    if (q.rank == 0) {
        printf("qubits=%d  numNodes=%d  logNumNodes=%d\n",
               q.numQubits, q.numNodes, q.logNumNodes);
        printf("numAmps=%lld  numAmpsPerNode=%lld  bytes/amp=%zu\n",
               (long long) q.numAmps, (long long) q.numAmpsPerNode, sizeof(qcomp));
        printf("distributed qubit indices: %d..%d (the top %d)\n",
               n - q.logNumNodes, n - 1, q.logNumNodes);
    }

    /* A single amplitude of 1 at global index 0 - easy to watch it spread. */
    initClassicalState(q, 0);
    dump(q, "initClassicalState(0): one amplitude, on one node");

    applyHadamard(q, 0);
    dump(q, "H on qubit 0: partner index differs in bit 0 - SAME node");

    applyHadamard(q, n - 1);
    dump(q, "H on top qubit: partner differs in the top bit - DIFFERENT node");

    destroyQureg(q);
    finalizeQuESTEnv();
    return 0;
}
