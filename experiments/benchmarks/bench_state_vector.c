#include "quest.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double) ts.tv_sec + 1e-9 * (double) ts.tv_nsec;
}

int main(int argc, char **argv) {
    initQuESTEnv();
    reportQuESTEnv();

    int numQubits = (argc > 1) ? atoi(argv[1]) : 30;
    int reps      = (argc > 2) ? atoi(argv[2]) : 5;
    if (numQubits < 4) numQubits = 4;
    if (reps < 1) reps = 1;

    Qureg qureg = createForcedQureg(numQubits);
    reportQuregParams(qureg);
    initRandomPureState(qureg);

    applyHadamard(qureg, 0);   // warm up

    syncQuESTEnv();
    double t0 = now_seconds();
    for (int r = 0; r < reps; r++)
        applyHadamard(qureg, 0);
    syncQuESTEnv();
    double localGate = (now_seconds() - t0) / reps;

    int topQubit = numQubits - 1;
    syncQuESTEnv();
    t0 = now_seconds();
    for (int r = 0; r < reps; r++)
        applyHadamard(qureg, topQubit);
    syncQuESTEnv();
    double distributedGate = (now_seconds() - t0) / reps;

    reportScalar("qubits", (qreal) numQubits);
    reportScalar("seconds per gate, qubit 0 (node-local)", (qreal) localGate);
    reportScalar("seconds per gate, top qubit (cross-node)", (qreal) distributedGate);
    reportScalar("cross-node penalty (x)", (qreal) (distributedGate / localGate));
    reportScalar("total probability", calcTotalProb(qureg));

    destroyQureg(qureg);
    finalizeQuESTEnv();
    return 0;
}
