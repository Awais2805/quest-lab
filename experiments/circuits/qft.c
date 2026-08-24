/* 
Quantum Fourier Transform (QFT) circuit implementation using QuEST library. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quest.h"
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef LOG_DIR
#define LOG_DIR "../experiments/logs"
#endif


//declare global constants for initial state and QFT methods
const char* INIT_METHOD[] = {"zero", "plus", "classical (requires qubit index)", "pure random"};
const char* INIT_METHOD_ARG[] = {"zero", "plus", "classical", "randompure"};
const char* QFT_METHOD[] = {"full", "partial (requires qubit indices)"};
const char* QFT_METHOD_ARG[] = {"full", "partial"};

/*
    Matches a CLI arg against a list of valid options.
    Returns the matching index, or -1 if no match is found.
*/
int matchArg(const char *arg, const char **options, int numOptions) {
    for (int i = 0; i < numOptions; i++) {
        if (strcmp(arg, options[i]) == 0) return i;
    }
    return -1;
}

/*
    Function to select the correct initial state based on user input.
    Qubit index is only required for classical state initialization.

*/
void prepareInitState(Qureg psi, int initChoice, char **argv) {
    // qubit index for classical state initialization
    int qubitIndex;

    switch (initChoice) {
        case 0:
            printf("Initializing to zero state.\n");
            initZeroState(psi);
            break;
        case 1:
            printf("Initializing to plus state.\n");
            initPlusState(psi);
            break;
        case 2:
            qubitIndex = atoi(argv[3]);
            printf("Initializing to classical state, qubit index %d.\n", qubitIndex);
            initClassicalState(psi, qubitIndex);
            break;
        case 3:
            printf("Initializing to pure random state.\n");
            initRandomPureState(psi);
            break;
        default:
            printf("Invalid choice. Initializing to zero state.\n");
            initZeroState(psi);
    }

}


/*
    Function to apply QuEST's QFT implementation based on user method input.
    For a partial QFT, target qubit count and indices are read in as cli args.
*/
void applyQFT(Qureg psi, int qftChoice, int argc, char **argv){
    
    int numTargetQubits;
    (void)argc; 

    switch (qftChoice){

        case 0:
            printf("Applying full QFT on all qubits.\n");
            applyFullQuantumFourierTransform(psi);
            break;

        case 1: {
            numTargetQubits = atoi(argv[5]);
            int targetQubits[numTargetQubits];
            for (int i = 0; i < numTargetQubits; i++) {
                targetQubits[i] = atoi(argv[6 + i]);
            }
            printf("Applying partial QFT on %d target qubit(s).\n", numTargetQubits);
            applyQuantumFourierTransform(psi, targetQubits, numTargetQubits); 
            break;
        }

        default:
            printf("Invalid choice. Applying full QFT on all qubits.\n");
            applyFullQuantumFourierTransform(psi);
    
    }

}


int main(int argc, char **argv) {

    // CLI args: <numQubits> <initMethod> <classicalQubitIndex> <qftMethod> <numTargetQubits> [targetQubit indices...]
    // classicalQubitIndex and numTargetQubits are positional placeholders when unused (pass 0)
    if (argc < 6) {
        printf("Usage: %s <numQubits> <initMethod> <classicalQubitIndex> <qftMethod> <numTargetQubits> [targetQubit indices...]\n", argv[0]);
        printf("initMethod options:\n");
        for (int i = 0; i < (int)(sizeof(INIT_METHOD_ARG) / sizeof(INIT_METHOD_ARG[0])); i++)
            printf("  %-10s : %s\n", INIT_METHOD_ARG[i], INIT_METHOD[i]);
        printf("qftMethod options:\n");
        for (int i = 0; i < (int)(sizeof(QFT_METHOD_ARG) / sizeof(QFT_METHOD_ARG[0])); i++)
            printf("  %-10s : %s\n", QFT_METHOD_ARG[i], QFT_METHOD[i]);
        return 1;
    }

    int n = atoi(argv[1]);

    int initChoice = matchArg(argv[2], INIT_METHOD_ARG, sizeof(INIT_METHOD_ARG) / sizeof(INIT_METHOD_ARG[0]));
    if (initChoice < 0) {
        printf("Unknown initMethod '%s'. Valid: zero, plus, classical, randompure\n", argv[2]);
        return 1;
    }

    int qftChoice = matchArg(argv[4], QFT_METHOD_ARG, sizeof(QFT_METHOD_ARG) / sizeof(QFT_METHOD_ARG[0]));
    if (qftChoice < 0) {
        printf("Unknown qftMethod '%s'. Valid: full, partial\n", argv[4]);
        return 1;
    }

    int numTargetQubits = atoi(argv[5]);
    if (qftChoice == 1 && argc < 6 + numTargetQubits) {
        printf("Not enough target qubit indices supplied on the command line.\n");
        return 1;
    }

    // initialize QuEST environment (required before any other QuEST calls)
    initQuESTEnv();
    QuESTEnv env = getQuESTEnv();

    // create a timestamped log file for each run
    time_t now = time(NULL);
    char ts[16];
    strftime(ts, sizeof(ts), "%Y%m%d_%H%M", gmtime(&now));
    
    if (mkdir(LOG_DIR, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Failed to create log directory %s: %s\n", LOG_DIR, strerror(errno));
        return 1;
    }
    
    char logPath[256];
    snprintf(logPath, sizeof(logPath), "%s/qft_rank%d_%s.log", LOG_DIR, env.rank, ts);
    if (freopen(logPath, "w", stdout) == NULL) {
        fprintf(stderr, "Failed to open log file %s\n", logPath);
        return 1;
    }

    // report the QuEST environment configuration at runtime
    reportQuESTEnv();

    // initialise qubit registers for n qubits
    printf("Using number of qubits: %d\n", n);
    Qureg psi = createQureg(n);

    prepareInitState(psi, initChoice, argv);


    // report the quantum register post state initialization 
    printf("Quantum register & parameters post state initialization:\n");
    reportQureg(psi);
    reportQuregParams(psi); 


    applyQFT(psi, qftChoice, argc, argv);

    // report the quantum register post QFT application
    printf("Quantum register post QFT application:\n");
    reportQureg(psi);

    
    // clean up and free resources
    destroyQureg(psi);
    finalizeQuESTEnv();







}