#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>


#define BLOCK_SIZE 32

struct Tuple{
    int dataPresent;
    int key;
    int key1;
};

struct Block{
    struct Tuple tuples[BLOCK_SIZE];
};

struct Result{
    int count;
    struct Tuple tuples[LIMIT];
};

struct BlockInfo{
    int dataPresent;
    int currSBlock;
    int kFailureReward;
    int accuracyBasedRewardBlock;
    int currReward;
    int tuplesTried;
};

int get_block_count(int size) {
    int no_of_blocks = size / BLOCK_SIZE;
    int rem = size % BLOCK_SIZE;

    if (rem > 0) {
        no_of_blocks++;
    }
    return no_of_blocks;
}

void loadData(struct Block blocks[], char *path, char *delim, int isOuterTable, int no_of_blocks) {

    char *item;
    int i;
    int j;
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(path, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    
    int done = 0;
    for(i=0; i < no_of_blocks; i++) {
        for(j = 0; j < BLOCK_SIZE; j++) {
            if ((read = getline(&line, &len, fp)) != -1) {
                item = strtok(line, delim);
                blocks[i].tuples[j].key = atoi(item);
                blocks[i].tuples[j].dataPresent = 1;
                if (isOuterTable == 1) {
                    item = strtok(NULL, delim);
                    blocks[i].tuples[j].key1 = atoi(item);
                } else {
                    blocks[i].tuples[j].key1 = -1;
                }
            } else {
                done = 1;
                break;
            }
        }

        if (done == 1) {
            break;
        }
    }
    
    fclose(fp);
    if (line)
        free(line);
 }

 void setPrecisionBasedReward(struct BlockInfo *blockInfo, float accuracyRate, int sBlocksToExplore, int *rewardDistribution, float sExploration) {
    // Get the total reward over the entire s table from the reward of sExploration
    //Another option is to check accuracyRate for current sExploration of the STable
    printf("\nblockInfo->currReward : %d", blockInfo->currReward);
    // printf("\n Array Value : %d", *rewardDistribution);
    int accuracyBasedReward = accuracyRate * blockInfo->currReward;
    printf("\naccuracyBasedReward : %d", accuracyBasedReward);

    int i, count = 0;

    for (i = 0; i < sBlocksToExplore; i++) {
        // printf("\nrewardDistribution[%d] : %d", i, *rewardDistribution);

        if (*rewardDistribution >= accuracyBasedReward) {
            blockInfo->accuracyBasedRewardBlock = i;
            printf("\naccuracyBasedRewardBlock : %d", i);
            return;
        }
        rewardDistribution++;
    }
 }

 void exploratory_join(struct BlockInfo *blockInfo, struct Result *result, struct Block rBlock, struct Block sTable[], int sBlocksToExplore, int kFailure, float accuracyRate, int limit, float sExploration) {
    int bs, r, s, sTupleCount = 0;
    int reward = 0;

    blockInfo->dataPresent = 1;
    int rewardDistribution[sBlocksToExplore];
    for(bs=0; bs < sBlocksToExplore; bs++) {

        if (bs == kFailure) {
            blockInfo->kFailureReward = reward;     
        }

        for (r = 0; r < BLOCK_SIZE; r++) {
            for (s = 0; s < BLOCK_SIZE; s++) {
                if (rBlock.tuples[r].dataPresent == 0 || sTable[bs].tuples[s].dataPresent == 0) {
                    goto done;
                }
                if (rBlock.tuples[r].key == sTable[bs].tuples[s].key1) {
                    result->tuples[result->count].key = sTable[bs].tuples[s].key;
                    result->tuples[result->count].key1 = sTable[bs].tuples[s].key1;
                    result->count += 1; 

                    reward++;
                }
                sTupleCount++;
            }   
        }
        // printf("\nReward In : %d", reward);
        rewardDistribution[bs] = reward;
        // printf("\nReward In : %d", rewardDistribution[bs]);

    }
    done:
    // printf("\nReward : %d", reward);
    // printf("\nReward Distribution : %d", rewardDistribution);
    
    blockInfo->tuplesTried = sTupleCount; 
    blockInfo->currReward = reward; 
    blockInfo->currSBlock = sBlocksToExplore + 1;
    setPrecisionBasedReward(blockInfo, accuracyRate, sBlocksToExplore, rewardDistribution, sExploration);
 }

int checkIfLearningIsPossible(struct BlockInfo blockInfo[], int rBlocksToExplore) {
    int total = 0;
    int i;
    float entropy = 0;
    // printf("\n checkIfLearningIsPossible");

    for(i = 0; i < rBlocksToExplore; i++) {
        // printf("\nblockInfo[i].currReward %d", blockInfo[i].currReward);
        if (blockInfo[i].currReward == 0) {
            blockInfo[i].currReward = 1;
        }
        total += blockInfo[i].currReward;
    }
    // printf("\n Total : %d", total);
    
    for(i = 0; i < rBlocksToExplore; i++) {
        float prob = (float)blockInfo[i].currReward / (float)total;
        // printf("\nLog : %.3f", prob);

        entropy += prob * log(prob);
        // printf("\nEntropy : %.3f", entropy);

    }

    entropy = -entropy;

    //TODO Need to verify this
    float maxEntropyValue = log(rBlocksToExplore);
    printf("\nMax Entropy Value : %4f", maxEntropyValue);
    printf("\nEntropy : %.3f", entropy);

    if (entropy > maxEntropyValue * 0.9 || maxEntropyValue *entropy < 0.75) {
        return 0;
    } else {
        return 1; 
    }
}


// void getBlockInfo()
int explore(struct Result *result, struct BlockInfo blockInfo[], int rBlocksToExplore, int sBlocksToExplore, struct Block rTable[], struct Block sTable[], int kFailure, float accuracyRate, int limit, float sExploration) {

    int br, bs, r, s, done;

    for (br=0; br < rBlocksToExplore; br++) {
        printf("\nR Block : %d", br);

        exploratory_join(&blockInfo[br], result, rTable[br], sTable, sBlocksToExplore, kFailure, accuracyRate, limit, sExploration);
    }
    printf("\nDone Exploring");
    
    int doLearning = checkIfLearningIsPossible(blockInfo, rBlocksToExplore);
    return doLearning;
}



int main(void) {   

    char *delim = "|";
    char *rTable_path = "./data/s0/customer_cleaned.tbl";
    char *sTable_path = "./data/s1/order.tbl";
    int RTable_SIZE = 86090;
    int STable_SIZE = 198103;
    int rBlocks = get_block_count(RTable_SIZE);
    int sBlocks = get_block_count(STable_SIZE);
    struct Block rTable[rBlocks];
    struct Block sTable[sBlocks];

    loadData(rTable, rTable_path, delim, 0, rBlocks);
    loadData(sTable, sTable_path, delim, 1, sBlocks);
    

    // Hyperparameters:
    //     s_exploration: Ratio of S to try each block of R with(Check for further optimization)
    //     kFailure: K number of blocks to try S with
    //     r_exploration: Ratio of R to perform exploration on
    //     accuracyRate: percentage of reward/join to see of the S Block
    //     limit: Total values of results to return

    float sExploration = 0.25;
    float accuracyRate = 0.1;
    float rExploration = 0.1;
    int kFailure = 5;
    int limit = 1000;

    int rBlocksToExplore = rExploration * rBlocks;
    int sBlocksToExplore = sExploration * sBlocks;
    printf("\n R Blocks to Explore : %d", rBlocksToExplore);
    printf("\n S Blocks to Explore : %d", sBlocksToExplore);

    struct Result result;
    result.count = 0;
    struct BlockInfo blockInfo[rBlocksToExplore];
    
    // struct BlockInfo blockInfo[];
    int doLearning = explore(&result, blockInfo, rBlocksToExplore, sBlocksToExplore, rTable, sTable, kFailure, accuracyRate, limit, sExploration);
    
    if (doLearning == 1) {
        int total = 0;
        int i;
        for (i = 0; i < rBlocksToExplore; i++) {
            total += blockInfo[i].accuracyBasedRewardBlock;
        }

        int accuracyBasedK = total / rBlocksToExplore;
        printf("\naccuracyBasedK : %d", accuracyBasedK);
        
    }
    printf("\ndoLearning : %d", doLearning);
	return 0;
}