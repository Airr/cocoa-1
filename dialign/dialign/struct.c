//
//  struct.c
//  dialign
//
//  Created by Sang Chul Choi on 8/14/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "struct.h"

void alder_align_print_scr_matrix (struct scr_matrix *o)
{
//    int length;   // number of amino acids
//    int max_score; // maximum among all entries in the data array
//    int *char2num; // resolves the character of an amino acid to its number
//    int *num2char; // resolves the number of an amino acid to its character
//    int *data;     // contains the matrix indexed by the number of the particular amino acid
//    int *dist;   // number of pairs of amino acids (i,j) having score at equal to              // the  value of the index
//    long double **raw_dist;
//    double avg_sim_score;
    
    fprintf(stdout,
            "----------\n"
            "scr_matrix\n"
            "----------\n"
            "length: %d\n"
            "max_score: %d\n"
            "avg_sim_score: %f\n"
            "char2num and num2char: ",
            o->length, o->max_score, o->avg_sim_score);
    for (int i = 0; i < o->length; i++) {
        fprintf(stdout, "[%d] %c ", i, o->num2char[i]);
    }
    fprintf(stdout, "\n");
//    fprintf(stdout, "data: ");
//    for (int i = 0; i < o->length; i++) {
//        fprintf(stdout, "[%d] %d ", i, o->data[i]);
//    }
//    fprintf(stdout, "\n");
//    fprintf(stdout, "dist: ");
//    for (int i = 0; i < o->length; i++) {
//        fprintf(stdout, "[%d] %d ", i, o->dist[i]);
//    }
//    fprintf(stdout, "\n");
//    for (int i = 0; i < 4; i++) {
//        for (int j = 0; j < 4; j++) {
//            fprintf(stdout, "%Lf ", o->raw_dist[i][j]);
//        }
//        fprintf(stdout, "\n");
//    }
}

void alder_align_print_seq_col (struct seq_col *o)
{
//    struct seq
//    {
//        char *data;  // sequence data
//        char *name;  // name/description of the sequence
//        int num;     // number of the sequence
//        int length;  // length of sequence
//        int max_seen;
//        char *dna_num; // Numbers fo retranslation from Protein to DNA
//        int orf_frame; // reading frame of the longest orf
//        char crick_strand; // orf translation or sequence translation on crickstrand
//    };
//    
//    
//    /**
//     * sequence collection
//     */
//    struct seq_col
//    {
//        struct seq *seqs; // array of the sequences
//        int avg_length;   // average length of sequences
//        int length;       // number of sequences
//    };
    printf("-------\n"
           "seq_col\n"
           "-------\n"
           "length: %d\n"
           "avg_length: %d\n"
           , o->length, o->avg_length);
    for (int i = 0; i < o->length; i++) {
        printf("seq[%d] (num: %d) (length: %d) (max_seen: %d) - %s"
               "  %s\n"
               , i, o->seqs[i].num, o->seqs[i].length, o->seqs[i].max_seen,
               o->seqs[i].name, o->seqs[i].data);
    }
}

void alder_align_print_diag_col (struct diag_col *o)
{
//    struct diag_col {
//        int seq_amount;            // number of sequences involved
//        struct simple_diag_col** diag_matrix; // diag_matrix[i +seq_amount *j] contains
//        // all diags found involving the sequences i and j
//        
//        double total_weight;       // total weight
//        double average_weight;     // average_weight
//        struct diag** diags; // all diags unordered
//        unsigned int diag_amount; // number of diags found
//        struct gt_node *gt_root;
//    };
    printf("--------\n"
           "diag_col\n"
           "--------\n"
           "seq_amount: %d\n"
           "total_weight: %f\n"
           "average_weight: %f\n"
           "diag_amount: %ud\n"
           , o->seq_amount, o->total_weight, o->average_weight,
           o->diag_amount);
}


void alder_align_print_simple_diag_col (struct simple_diag_col *o)
{
//
//struct simple_diag_col {
//  unsigned int length;       // number of diags
//  double total_weight;       // total weight
//  double weight_fac;         // weight factor
//  struct diag** data;        // the array of diags
//};
    
    printf("simple_diag_col\n"
           "length: %ud\n"
           "total_weight: %lf\n"
           "weight_frac: %lf\n"
           , o->length, o->total_weight, o->weight_fac);
}

void alder_align_print_diag (struct diag *o)
{
//struct diag {
//  struct seq_part seq_p1; // first sequence part
//  struct seq_part seq_p2; // seconde sequence part
//  unsigned int length;     // length of the diag
//  long score;     // score of the diag
//  long orig_score;     // orig score of the diag
//
//  struct diag *pred_diag;  // predecessor diag for dynamic programming
//  struct diag *col_pred_diag;  // col predecessor diag for dynamic programming
//  int pool_pos;            // position in diag pool
//
//  char meetsThreshold;	   // whether diag meets threshold
//
//  // for vertex cover
//  int degree;
//  int max_degree;
//  struct diag **neighbours;
//
//  char anchor;              // if this is an anchor diag
//  char marked;              // marking flag for arbitrary use
//  char multi_dg;            // is >0 if this is a multi dg
//  struct diag **multi_cont; // the contained dgs of this is a multi dg
//  int multi_length;         // size of multi_cont
//
//  //  char onlyOverThres;
//
//  double weight;           // weight of the diag = -log(prob)
//  double weight_sum;       // weight sum for dialign
//  double weight_fac;       // weight factor
//  double ov_weight;       // overlap weight
//  double total_weight;       // total_weight = weight+o_weight
//};
    printf("diag:\n"
           "-----\n"
           "length: %d\n"
           "score: %ld\n"
           "orig_score: %ld\n"
           "pool_pos: %d\n"
           "meetsThreshold: %c\n"
           "degree: %d\n"
           "max_degree: %d\n"
           "anchor: %c\n"
           "marked: %c\n"
           "multi_dg: %c\n"
           "multi_length: %d\n"
           , o->length, o->score, o->orig_score, o->pool_pos,
           o->meetsThreshold,
           o->degree,
           o->max_degree,
           o->anchor,
           o->marked,
           o->multi_dg,
           o->multi_length);
    alder_align_print_seq_part(&o->seq_p1);
    alder_align_print_seq_part(&o->seq_p2);
}



void alder_align_print_seq (struct seq *o)
{
    
//struct seq 
//{
//  char *data;  // sequence data  
//  char *name;  // name/description of the sequence
//  int num;     // number of the sequence
//  int length;  // length of sequence
//  int max_seen;
//  char *dna_num; // Numbers fo retranslation from Protein to DNA
//  int orf_frame; // reading frame of the longest orf
//  char crick_strand; // orf translation or sequence translation on crickstrand
//};
    printf("seq (num: %d) (length: %d) (max_seen: %d) - %s"
           "  %s\n"
           , o->num, o->length, o->max_seen,
           o->name, o->data);
}

void alder_align_print_seq_part (struct seq_part *o)
{
//struct seq_part {
//  int num;         // a number that indicates a position in an array
//  struct seq* sq;  // the pointer to the sequence
//  int startpos;   // startpos in the sequence
//  //int leftmargin;
//  //int rightmargin;
//};

    printf("seq_part:"
           "--------\n"
           "num: %d\n"
           "startpos: %d\n"
           , o->num, o->startpos);
    alder_align_print_seq (o->sq);
}


void alder_align_print_prob_dist (struct prob_dist *o)
{
//struct prob_dist {
//  struct scr_matrix *smatrix; // pointer to the associated score matrix
//  long double **data;  // distribution of scores dist[i][j] contains the     
//                       // probability of a diags of length i having score >=j
//  double **log_data; // distribution of scores dist[i][j] contains the     
//                  // -log(probability of a diags of length i having score >=j)
//  //  long double *expect;      // score expectancy for each diaglen
//  unsigned int max_dlen;     // maximum diaglength in dist
//};
    
}

void alder_align_print_algn_pos (struct algn_pos *o)
{
//struct algn_pos {
//  char state;         // orphane: not aligned to any pos,
//
//  struct diag_cont *dg_cont; // diags that are aligned with that position
//
//  int row;
//  int col;
//  int* predF;    // predecessor frontier, only filled if non-orphane
//  int* succF;    // successor frontier, only filled if non-orphane
//
//  char *proceed;    // for output
//  int predFPos;  // in case of orphane, where to find the predF or succF
//  int succFPos;
//
//  int *eqcAlgnPos; // equivalence class minimum alignment position (for output)
//  struct algn_pos *eqcParent; // equivalence class parent
//  int eqcRank; // equivalence class rank (>= maximum number of children)
//};
    
    printf("algn_pos\n"
           "--------\n"
           "state: %c\n"
           "row: %d\n"
           "col: %d\n"
           "predFPos: %d\n"
           "succFPos: %d\n"
           "eqcRank: %d\n"
           , o->state,
           o->row,
           o->col,
           o->predFPos,
           o->succFPos,
           o->eqcRank);
}

void alder_align_print_alignment (struct alignment *o)
{
//struct alignment {
//  char *seq_is_orphane;      // boolean array indicating for each sequence
//                             // whether it is orphane or not
//  int max_pos;               // the greatest position in the alignment (including all -'s)
//                             // if <0: the alignment has not yet been prepared
//  struct seq_col *scol;      // all the sequences involved
//  struct algn_pos **algn;    // the alignment
//  double total_weight;       // the total weight of the alignment
//  struct alignment *next;    // pointer to next alignment in the sorted linked list
//};

    printf("alignment\n"
           "---------\n"
           "seq_is_orphane[array]: %d\n"
           "max_pos: %d\n"
           "total_wieght: %lf\n"
           , o->seq_is_orphane[0],
           o->max_pos,
           o->total_weight);
}
