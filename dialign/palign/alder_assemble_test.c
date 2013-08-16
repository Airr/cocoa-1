//
//  main.c
//  palign
//
//  Created by Sang Chul Choi on 8/14/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "alder_assemble.h"

struct parameters *para;

int alder_assemble_test(int argc, const char * argv[])
{
    init_parameters();
    set_parameters_dna();
    set_parameters("/Users/goshng/Library/Developer/Xcode/DerivedData/dialign-bugmultaffytaqcooumdypjlaeuu/Build/Products/Debug/conf", "/Users/goshng/Library/Developer/Xcode/DerivedData/dialign-bugmultaffytaqcooumdypjlaeuu/Build/Products/Debug/t.fasta", NULL);
    
    char *smatrixfile = (char *)build_pathname(para->conf_dir,para->SCR_MATRIX_FILE_NAME);
    struct scr_matrix *smatrix = read_scr_matrix(smatrixfile);
    
    // print the score matrix
    if( para->DEBUG >5) print_scr_matrix(smatrix);
    
    // read the probability distribution for diagonals
    char *pdistfilename = (char *)build_pathname(para->conf_dir,para->DIAG_PROB_FILE_NAME);
    struct prob_dist *pdist = read_diag_prob_dist(smatrix, pdistfilename);
    struct seq_col *in_seq_col = read_fasta(para->in_file);
    
    // print read input sequences
    if(para->DEBUG>5) {
        int sc,scl = in_seq_col->length;
        for(sc=0; sc < scl;sc++) {
            print_seq(&(in_seq_col->seqs[sc]));
            printf("\n");
        }
    }
    // fast mode has higher threshold weights
    if(para->FAST_MODE) {
        para->PROT_SIM_SCORE_THRESHOLD += 0.25;
    }
    
    // ----------------------------------------------------------------------
    // Consider Anchors
    // ----------------------------------------------------------------------
    struct alignment *algn= NULL;
    if(! para->FAST_MODE) {
        algn = create_empty_alignment(in_seq_col);
    }
    struct alignment *salgn = create_empty_alignment(in_seq_col);
    
    //    struct diag_col adcol;
    //    struct simple_diag_col *anchors = NULL;
    //    if(para->DO_ANCHOR>0) {
    //        anchors = read_anchors(para->ANCHOR_FILE_NAME, in_seq_col);
    //
    //        adcol.diags = anchors->data;
    //        adcol.diag_amount = anchors->length;
    //
    //
    //        simple_aligner(in_seq_col, &adcol,smatrix,pdist,salgn,1);
    //        if(! para->FAST_MODE) simple_aligner(in_seq_col, &adcol,smatrix,pdist,algn,1);
    //
    //
    //        if(anchors!=NULL) {
    //            for(i=0;i<adcol.diag_amount;i++) {
    //                free_diag(adcol.diags[i]);
    //            }
    //            free(adcol.diags);
    //            free(anchors);
    //        }
    //    }
    
    // ----------------------------------------------------------------------
    // Compute pairwise diagonals
    // ----------------------------------------------------------------------
    struct diag_col *all_diags = find_all_diags(smatrix, pdist, in_seq_col,salgn,1);
    if(para->DEBUG >1) printf("Found %i diags\n", all_diags->diag_amount);
    int diag_amount = all_diags->diag_amount;
    
    // ----------------------------------------------------------------------
    // Compute alignment
    // ----------------------------------------------------------------------
    if(! para->FAST_MODE) {
        struct diag *cp_diags[all_diags->diag_amount];
        for(int i=0;i<diag_amount;i++) {
            cp_diags[i] = malloc(sizeof (struct diag));
            *(cp_diags[i]) = *(all_diags->diags[i]);
        }
        guided_aligner(algn, in_seq_col, all_diags,smatrix,pdist,all_diags->gt_root, 1);
        
        
        for(int i=0;i<diag_amount;i++) {
            all_diags->diags[i] = cp_diags[i];
        }
        all_diags->diag_amount = diag_amount;
    }
    //struct alignment *algn = salgn;
    simple_aligner(in_seq_col, all_diags,smatrix,pdist,salgn,1);
    
    // Clean up!
    free_diag_col(all_diags);
    
    para->DO_ANCHOR = 0; // anchors done
    
    // ----------------------------------------------------------------------
    // round 2+
    // ----------------------------------------------------------------------
    int round;
    char newFound = 0;
    int type;
    
    // consider sensitivity level
    if(! para->FAST_MODE) {
        if(para->SENS_MODE==0) {
            para->DIAG_THRESHOLD_WEIGHT = 0.0;
        } else if(para->SENS_MODE==1) {
            if(para->DNA_PARAMETERS)
                para->DIAG_THRESHOLD_WEIGHT = -log(0.75);//-log(.875+0.125/2.0);
            else
                para->DIAG_THRESHOLD_WEIGHT = -log(0.75);
        }else if(para->SENS_MODE==2) {
            if(para->DNA_PARAMETERS)
                para->DIAG_THRESHOLD_WEIGHT = -log(0.5);//-log(0.875);
            else
                para->DIAG_THRESHOLD_WEIGHT = -log(0.5);
        }
    }
    
    int stype = (para->FAST_MODE ? 1 : 0);
    for(type=stype;type<2;type++) {
        for(round=2;round<=20;round++) {
            all_diags = find_all_diags(smatrix, pdist, in_seq_col,(type ? salgn : algn), round);
            if(para->DEBUG >1) printf("Found %i diags", all_diags->diag_amount);
            if(all_diags->diag_amount ==0) {
                free_diag_col(all_diags);
                break;
            } else {
                // round 2 and further we use the simple aligner
                newFound = simple_aligner(in_seq_col, all_diags,smatrix,pdist,(type ? salgn : algn),round);
                free_diag_col(all_diags);
                if(!newFound) break;
            }
        }
    }
    if(para->DEBUG >1)
        printf("Alignment ready!\n");
    
    if(! para->FAST_MODE) {
        if(para->DEBUG >1) printf("Final alignment simple: %f guided: %f\n", salgn->total_weight, algn->total_weight);
    } else {
        if(para->DEBUG >1) printf("Final alignment simple: %f \n", salgn->total_weight);
    }
    
    if( ( para->FAST_MODE) || (salgn->total_weight > algn->total_weight)) {
        if(! para->FAST_MODE) free_alignment(algn);
        algn = salgn;
    }
    
    if(para->out_file==NULL) {
        simple_print_alignment_default(algn);
    }else {
        fasta_print_alignment_default(algn, para->out_file);
    }
    
    // Clean up!
    free_alignment(algn);
    free(smatrixfile);
    free(pdistfilename);
    alder_align_free_seq_col(in_seq_col); in_seq_col = NULL;
    alder_align_free_prob_dist(pdist); pdist = NULL;
    alder_align_free_scr_matrix(smatrix); smatrix = NULL;
    //    free_diag_col(all_diags); all_diags = NULL; // freed multiple times.
    free_parameters();
    return 0;
}

