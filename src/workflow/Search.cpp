#include <iostream>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <list>

#include "WorkflowFunctions.h"

extern "C" {
#include "ffindex.h"
#include "ffutil.h"
}

void printUsage(){

    std::string usage("\nCalculates the clustering of the sequences in the input database.\n");
    usage.append("Written by Maria Hauser (mhauser@genzentrum.lmu.de))\n\n");
    usage.append("USAGE: mmseqs_clustering ffindexQueryDBBase ffindexTargetDBBase ffindexOutDBBase tmpDir [opts]\n"
            "-m              \t[file]\tAmino acid substitution matrix file.\n"
            "--max-seqs      \tMaximum result sequences per query (default=300)\n"
            "--max-seq-len   \t[int]\tMaximum sequence length (default=50000).\n"
            "-s              \t[float]\tTarget sensitivity in the range [2:9] (default=4).\n"
            );
    std::cout << usage;
}

void parseArgs(int argc, const char** argv, std::string* ffindexQueryDBBase, std::string* ffindexTargetDBBase, std::string* ffindexOutDBBase, std::string* tmpDir, std::string* scoringMatrixFile, size_t* maxSeqLen, float* sens, size_t* maxResListLen){
    if (argc < 4){
        printUsage();
        exit(EXIT_FAILURE);
    }

    ffindexQueryDBBase->assign(argv[1]);
    ffindexTargetDBBase->assign(argv[2]);
    ffindexOutDBBase->assign(argv[3]);
    tmpDir->assign(argv[4]);

    int i = 5;
    while (i < argc){

        if (strcmp(argv[i], "-m") == 0){
            if (++i < argc){
                scoringMatrixFile->assign(argv[i]);
                i++;
            }
            else {
                printUsage();
                std::cerr << "No value provided for " << argv[i] << "\n";
                exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(argv[i], "--max-seq-len") == 0){
            if (++i < argc){
                *maxSeqLen = atoi(argv[i]);
                i++;
            }
            else {
                printUsage();
                std::cerr << "No value provided for " << argv[i] << "\n";
                exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(argv[i], "-s") == 0){
            if (++i < argc){
                *sens = atof(argv[i]);
                if (*sens < 2.0 || *sens > 9.0){
                    Debug(Debug::ERROR) << "Please choose sensitivity in the range [2:9].\n";
                    exit(EXIT_FAILURE);
                }
                i++;
            }
        }
        else if (strcmp(argv[i], "--max-seqs") == 0){
            if (++i < argc){
                *maxResListLen = atoi(argv[i]);
                i++;
            }
            else {
                printUsage();
                Debug(Debug::ERROR) << "No value provided for " << argv[i-1] << "\n";
                exit(EXIT_FAILURE);
            }
        }
        else {
            printUsage();
            std::cerr << "Wrong argument: " << argv[i] << "\n";
            exit(EXIT_FAILURE);
        }
    }
    if (strcmp (scoringMatrixFile->c_str(), "") == 0){
        printUsage();
        std::cerr << "\nPlease provide a scoring matrix file. You can find scoring matrix files in $INSTALLDIR/data/.\n";
        exit(EXIT_FAILURE);
    }
}

void runSearch(float sensitivity, size_t maxSeqLen, int seqType,
        int kmerSize, int alphabetSize, size_t maxResListLen, int split, int skip, bool aaBiasCorrection,
        double evalThr, double covThr,
        std::string queryDB, std::string targetDB, std::string outDB, std::string scoringMatrixFile, std::string tmpDir){

    std::string queryDBIndex = queryDB + ".index";
    std::string targetDBIndex = targetDB + ".index";
    float zscoreThr = getZscoreForSensitivity(sensitivity);

    std::string alnDB = runStep(queryDB, queryDBIndex, targetDB, targetDBIndex, tmpDir, scoringMatrixFile, maxSeqLen, seqType, kmerSize, alphabetSize, maxResListLen, split, skip, aaBiasCorrection, zscoreThr, sensitivity, evalThr, covThr, INT_MAX, 1, 0, true);

    std::string alnDBIndex = alnDB + ".index";
    std::string outDBIndex = outDB + ".index";

    // copy the clustering databases to the right location
    copy(alnDBIndex, outDBIndex);
    copy(alnDB, outDB);
}

int main (int argc, const char * argv[]){

    // general parameters
    size_t maxSeqLen = 50000;
    int seqType = Sequence::AMINO_ACIDS;
    float targetSens = 4.0;
    size_t maxResListLen = 300;

    // parameter for the prefiltering
    int kmerSize = 6;
    int alphabetSize = 21;
    int split = 0;
    int skip = 0;
    bool aaBiasCorrection = true;

    // parameters for the alignment
    double evalThr = 0.001;
    double covThr = 0.8;

    std::string queryDB = "";
    std::string targetDB = "";
    std::string outDB = "";
    std::string scoringMatrixFile = "";
    std::string tmpDir = "";

    parseArgs(argc, argv, &queryDB, &targetDB, &outDB, &tmpDir, &scoringMatrixFile, &maxSeqLen, &targetSens, &maxResListLen);

    runSearch(targetSens, maxSeqLen, seqType,
            kmerSize, alphabetSize, maxResListLen, split, skip, aaBiasCorrection,
            evalThr, covThr,
            queryDB, targetDB, outDB, scoringMatrixFile, tmpDir);

}
