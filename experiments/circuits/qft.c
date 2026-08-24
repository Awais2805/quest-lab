/* 
Quantum Fourier Transform (QFT) circuit implementation using QuEST library. 
*/

#include <stdio.h>
#include "quest.h"
#include <math.h>

const char* INIT_METHOD[] = {"zero", "plus", "classical (requires qubit index)", "pure random"};
const char* QFT_METHOD[] = {"full", "partial (requires qubit indices)"};

void prepareInitState(Qureg psi, int initChoice) {
    // qubit index for classical state initialization
    int stateIndex;

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
            printf("Initializing to classical state \n");
            printf("Enter the qubit index (0 to %d): ", (1 << psi.numQubits) - 1);
            scanf("%d", &stateIndex);
            initClassicalState(psi, stateIndex);
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

void applyQFT(Qureg psi, int qftChoice){
    
    int numTargetQubits;

    switch (qftChoice){

        case 0:
            printf("Applying full QFT on all qubits.\n");
            applyFullQuantumFourierTransform(psi);
            break;

        case 1: {
            printf("Enter number of target qubits for partial QFT: ");
            scanf("%d", &numTargetQubits);
            int targetQubits[numTargetQubits];
            printf("Enter the indices of the target qubits (0 to %d):\n", psi.numQubits - 1);
            for (int i = 0; i < numTargetQubits; i++) {
                scanf("%d", &targetQubits[i]);
            }
            printf("Applying partial QFT on specified qubits.\n");
            applyPartialQuantumFourierTransform(psi, targetQubits, numTargetQubits); 
            break;
        }

        default:
            printf("Invalid choice. Applying full QFT on all qubits.\n");
            applyFullQuantumFourierTransform(psi);
    
    }
    

}


int main(int argc, char **argv) {

    int initChoice;
    int qftChoice;

    // initialize QuEST environment (required before any other QuEST calls)
    initQuESTEnv();
    // report the QuEST environment configuration at runtime
    reportQuESTEnv();
    // get and set the QuEST environment configuration
    QuESTEnv env = getQuESTEnv();

    // initialise qubit registers for n qubits
    printf("Enter the number of qubits: ");
    int n = scanf("%d", &n);
    Qureg psi = createQureg(n);

    // prompt user to confirm initial state method
    printf("Choose an initial state method:\n");
    for (int i = 0; i < sizeof(INIT_METHOD) / sizeof(INIT_METHOD[0]); i++) {
        printf("%d: %s\n", i, INIT_METHOD[i]);
    }
    scanf("%d", &initChoice);
    prepareInitState(psi, initChoice);


    // report the quantum register post state initialization 
    reportQureg(psi);
    reportQuregParams(psi); 


    // prompt user to confirm QFT method
    printf("Choose a QFT method (default is full):\n");
    for (int i = 0; i < sizeof(QFT_METHOD) / sizeof(QFT_METHOD[0]); i++) {
        printf("%d: %s\n", i, QFT_METHOD[i]);
    }
    scanf("%d", &qftChoice);
    applyQFT(psi, qftChoice);






}