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

    syncQuESTEnv();
    double t0 = now_seconds();
    Qureg qureg = createForcedQureg(numQubits);
    syncQuESTEnv();
    double allocTime = now_seconds() - t0;
    reportQuregParams(qureg);

    /* initPlusState writes a nonzero amplitude to every element, forcing every
       page to be committed. initZeroState could leave most of the array mapped
       to the shared zero page and understate true memory pressure. */
    syncQuESTEnv();
    t0 = now_seconds();
    initPlusState(qureg);
    syncQuESTEnv();
    double initTime = now_seconds() - t0;

    reportScalar("qubits", (qreal) numQubits);
    reportScalar("allocation seconds", (qreal) allocTime);
    reportScalar("initPlusState seconds", (qreal) initTime);

    /* One Hadamard per qubit index. The distributed qubits announce themselves
       in the timings, so nothing here has to assume which ones they are. */
    for (int q = 0; q < numQubits; q++) {
        syncQuESTEnv();
        t0 = now_seconds();
        applyHadamard(qureg, q);
        syncQuESTEnv();
        double dt = now_seconds() - t0;

        char label[64];
        snprintf(label, sizeof label, "H on qubit %2d (s)", q);
        reportScalar(label, (qreal) dt);
    }

    /* A global reduction across every amplitude on every node - the cheapest
       way to catch silent corruption, which is exactly what memory pressure
       and sustained network traffic are liable to produce. H applied to every
       qubit of |+>^n returns |0>^n, so this must still be 1. */
    syncQuESTEnv();
    t0 = now_seconds();
    qreal prob = calcTotalProb(qureg);
    syncQuESTEnv();
    reportScalar("calcTotalProb seconds", (qreal) (now_seconds() - t0));
    reportScalar("total probability (want 1)", prob);

    

    destroyQureg(qureg);
    finalizeQuESTEnv();
    return 0;
}
