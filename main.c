#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define R_TABLE_SIZE 86090
#define S_TABLE_SIZE 198103
#define BLOCK_SIZE 32


struct Tuple{
    int dataPresent;
    int key;
    int key1;
};

struct Block{
    int dataPresent;
    struct Tuple tuples[BLOCK_SIZE];
};

struct BlockInfo{
    int dataPresent;
    int curr_S_block;
    int one_failure_reward;
    int precision_based_reward;
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
        blocks[i].dataPresent = 1;
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

// void getBlockInfo()
void explore(struct BlockInfo results[], int r_blocks_to_explore, int s_blocks_to_explore, struct Block r_table[], struct Block s_table[], int k_failure, float accuracy_rate, int limit) {

    int br, bs, r, s, done;

    for (br=0; br < r_blocks_to_explore; br++) {
        struct Tuple r_tuples[] = r_table[br].tuples;
        for(bs=0; bs < s_blocks_to_explore; bs++) {
            struct Tuple s_tuples[] = s_table[bs].tuples;
            
            for (r = 0; r < BLOCK_SIZE; r++) {
                for (s = 0; s < BLOCK_SIZE; s++) {
                    if (r_tuples[r].dataPresent == 0 || s_tuples[s].dataPresent == 0) {
                        done = 1;
                        break;
                    }
                    if (r_tuples[r].key == s_tuples[s].key) {
                        
                    }
                
                }
            }
            

        }

    }

}


int main(void) {   

    char *delim = "|";
    char *r_table_path = "./data/s1/customer_cleaned.tbl";
    char *s_table_path = "./data/s1/order.tbl";

    int r_blocks = get_block_count(R_TABLE_SIZE);
    int s_blocks = get_block_count(S_TABLE_SIZE);
    struct Block r_table[r_blocks];
    struct Block s_table[s_blocks];

    loadData(r_table, r_table_path, delim, 0, r_blocks);
    loadData(s_table, s_table_path, delim, 1, s_blocks);

    // Hyperparameters:
    //     s_exploration: Ratio of S to try each block of R with(Check for further optimization)
    //     k_failure: K number of blocks to try S with
    //     r_exploration: Ratio of R to perform exploration on
    //     accuracy_rate: percentage of reward/join to see of the S Block
    //     limit: Total values of results to return

    float s_exploration = 0.25;
    float accuracy_rate = 0.1;
    float r_exploration = 0.1;
    int k_failure = 5;
    int limit = 1000;
    int r_blocks_to_explore = r_exploration * R_TABLE_SIZE;
    int s_blocks_to_explore = s_exploration * S_TABLE_SIZE;

    struct Block results[limit];
    
    // struct BlockInfo blockInfo[];
    explore(results, r_blocks_to_explore, s_blocks_to_explore, r_table, s_table, k_failure, accuracy_rate, limit);
    




	return 0;
}