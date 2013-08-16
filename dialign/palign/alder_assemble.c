//
//  alder_align_palign.c
//  dialign
//
//  Created by Sang Chul Choi on 8/14/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "alder_assemble.h"

#ifdef DEBUG
#undef DEBUG
#endif

/************************************************/
/*                                              */
/*              global variable                 */
/*                                              */
/************************************************/
static struct parameters *para;
static struct scr_matrix *smatrix;
static struct prob_dist *pdist;

////////////////////////////////////////////////////////////////////////////////
// parameters.c
////////////////////////////////////////////////////////////////////////////////
#pragma mark parameters

void init_parameters()
{
	if((para =(struct parameters *) malloc(sizeof(struct parameters)) ) == NULL)
	{
		error("init_parameters(): Out of memory when allocating data !");
	}
	para->VERSION = "1.0.2";
	para->DEBUG = 0;
	para->MAX_SEQ_AMOUNT = 5000;
	para->MAX_FASTA_LINE_LENGTH = 100;
	para->PRINT_SEQ_LINE_LENGTH = 80;
//	para->SCR_MATRIX_FILE_NAME="BLOSUM.scr";
    para->SCR_MATRIX_FILE_NAME = NULL;
	para->DIAG_CALC_WEIGHT_THRESHOLD = 0.000000065;
//	para->DIAG_PROB_FILE_NAME="BLOSUM.diag_prob_t10";
    para->DIAG_PROB_FILE_NAME = NULL;
	para->SCR_MATRIX_ADD = 0;
	para->PROT_SIM_SCORE_THRESHOLD = 4.0;
	//para->PROT_DIAG_MAX_UNDER_THRESHOLD_POS = 4;
	//para->PROT_DIAG_MIN_LENGTH_THRESHOLD = 10.0;
	para->PROT_DIAG_MAX_UNDER_THRESHOLD_POS = 4;
	para->PROT_DIAG_MIN_LENGTH_THRESHOLD = 40.0;
	para->PROT_DIAG_AVG_SCORE_THRESHOLD = 4.0;
	para->DO_ANCHOR = 0;
	para->ANCHOR_FILE_NAME = NULL;
	para->DO_OVERLAP = 0;
	para->DIAG_MIN_LENGTH = 1;
	para->FAST_MODE = 0;
	para->SENS_MODE = 0;
	para->DIAG_THRESHOLD_WEIGHT = -log(0.5);
	para->FAST_PAIRWISE_ALIGNMENT = 0;
	para->conf_dir = NULL;
	para->in_file = NULL;
	para->out_file = NULL;
 	para->COMPUTE_PROB=0;
	para->STATE_ORPHANE=1;
	para->STATE_INHERITED=2;
	para->DNA_PARAMETERS = 0;
	para->DNA_TRANSLATION = 0;
	para->FIND_ORF = 0;
	para->ORF_FRAME = 0;
	para->OUTPUT = 0;
}

void free_parameters()
{
//	char *DIAG_PROB_FILE_NAME; // = "BLOSUM.diag_prob_t10";
//	char *SCR_MATRIX_FILE_NAME; // = "BLOSUM.scr";
//	char *ANCHOR_FILE_NAME; // = NULL;
//    char *conf_dir ;
//    char *in_file ;
//    char *out_file ;
    if (para->ANCHOR_FILE_NAME != NULL) free(para->ANCHOR_FILE_NAME);
    if (para->DIAG_PROB_FILE_NAME != NULL) free(para->DIAG_PROB_FILE_NAME);
    if (para->SCR_MATRIX_FILE_NAME != NULL) free(para->SCR_MATRIX_FILE_NAME);
    if (para->conf_dir != NULL) free(para->conf_dir);
    if (para->in_file != NULL) free(para->in_file);
    if (para->out_file != NULL) free(para->out_file);
    free(para);
    para = NULL;
}

/****************************
 *     DNA DEFAULT VALUES!   *
 ****************************/
void set_parameters_dna()
{
//	para->VERSION = "1.0.2";
	para->DEBUG = 0;
	para->MAX_SEQ_AMOUNT = 5000;
	para->MAX_FASTA_LINE_LENGTH = 100;
	para->PRINT_SEQ_LINE_LENGTH = 80;
//	para->SCR_MATRIX_FILE_NAME="dna_matrix.scr";
	para->DIAG_CALC_WEIGHT_THRESHOLD = 0.000000065;
//	para->DIAG_PROB_FILE_NAME="dna_diag_prob_100_exp_550000";
	para->SCR_MATRIX_ADD = 0;
	para->PROT_SIM_SCORE_THRESHOLD = 0.25;
	para->PROT_DIAG_MAX_UNDER_THRESHOLD_POS = 4; // 1
	para->PROT_DIAG_MIN_LENGTH_THRESHOLD = 40.0;//40.0; // 1
	para->PROT_DIAG_AVG_SCORE_THRESHOLD = 0.25; //1.0
	para->DO_ANCHOR = 0;
	para->ANCHOR_FILE_NAME = NULL;
	para->DO_OVERLAP = 0;
	para->DIAG_MIN_LENGTH = 1;
	para->FAST_MODE = 0;
	para->SENS_MODE = 0;
	para->DIAG_THRESHOLD_WEIGHT = -log(0.5);//-log(0.875);
	para->FAST_PAIRWISE_ALIGNMENT = 0;
	para->conf_dir = NULL;
	para->in_file = NULL;
	para->out_file = NULL;
 	para->COMPUTE_PROB=0;
	para->STATE_ORPHANE=1;
	para->STATE_INHERITED=2;
	para->DNA_PARAMETERS = 1;
	para->DNA_TRANSLATION = 0;
	para->FIND_ORF = 0;
	para->ORF_FRAME = 0;
	para->OUTPUT = 1;
}

//	para->SCR_MATRIX_FILE_NAME="dna_matrix.scr";
//	para->DIAG_PROB_FILE_NAME="dna_diag_prob_100_exp_550000";
//	para->conf_dir = NULL;
//	para->in_file = NULL;
//	para->out_file = NULL;
void set_parameters(const char *confDir, const char *inFile, const char *outFile)
{
	para->SCR_MATRIX_FILE_NAME = strdup("dna_matrix.scr");
	para->DIAG_PROB_FILE_NAME  = strdup("dna_diag_prob_100_exp_550000");
	para->conf_dir = strdup(confDir);
	para->in_file = strdup(inFile);
    if (outFile != NULL) para->out_file = strdup(outFile);
}

/*
void check_input(int argc, char** argv)
{
	int flag_protein_output = 0;
	opterr = 1;
	optind = 1; // for mac! 0 for others. Why?
	int opt, only_config_dir = 0;
    
	while((opt = getopt(argc, argv,"PDTLOCFHhA:d:s:a:c:l:m:w:p:v:t:n:g:o:r:u"))!= -1){
        switch(opt){
            case 'd':
                para->DEBUG = atoi(optarg);
                break;
            case 's':
                para->MAX_SEQ_AMOUNT = atoi(optarg);
                break;
            case 'a':
                para->MAX_FASTA_LINE_LENGTH = atoi(optarg);
                break;
            case 'c':
                para->PRINT_SEQ_LINE_LENGTH = atoi(optarg);
                break;
            case 'l':
		        para->SENS_MODE = atoi(optarg);
                break;
            case 'A':
                para->ANCHOR_FILE_NAME = optarg;
                para->DO_ANCHOR = 1;
                break;
            case 'm':
                para->SCR_MATRIX_FILE_NAME = optarg;
                break;
            case 'w':
                para->DIAG_CALC_WEIGHT_THRESHOLD = atof(optarg);
                break;
            case 'p':
                para->DIAG_PROB_FILE_NAME = optarg;
                break;
            case 'v':
                para->SCR_MATRIX_ADD = atoi(optarg);
                break;
            case 't':
                para->PROT_SIM_SCORE_THRESHOLD = atoi(optarg);
                break;
            case 'n':
                para->PROT_DIAG_MAX_UNDER_THRESHOLD_POS = atoi(optarg);
                break;
            case 'g':
                para->PROT_DIAG_MIN_LENGTH_THRESHOLD = atof(optarg);
                break;
            case 'r':
                para->PROT_DIAG_AVG_SCORE_THRESHOLD = atof(optarg);
                break;
            case 'o':
                para->DO_OVERLAP = atoi(optarg);
                break;
            case 'u':
                para->DIAG_MIN_LENGTH = atoi(optarg);
                break;
            case 'h':
                wrong_input();
            case 'H':
                wrong_input();
            case 'C':
                only_config_dir = 1;
                para->COMPUTE_PROB = 1;
                break;
            case 'F':
  		        para->DIAG_THRESHOLD_WEIGHT = 0.0;
		        para->FAST_MODE = 1;
                break;
            case 'O':
		        para->DNA_PARAMETERS = 0;
                para->DNA_TRANSLATION = 1;
                para->FIND_ORF = 1;
                para->ORF_FRAME = 1;
                para->OUTPUT = 1;
                break;
            case 'L':
                para->DNA_PARAMETERS = 0;
                para->DNA_TRANSLATION = 1;
                para->FIND_ORF = 1;
                para->ORF_FRAME = 0;
                para->OUTPUT = 1;
                break;
            case 'T':
                para->DNA_PARAMETERS = 0;
                para->DNA_TRANSLATION = 1;
                para->FIND_ORF = 0;
                para->ORF_FRAME = 0;
                para->OUTPUT = 1;
                break;
            case 'D':
                break;
            case 'P':
                flag_protein_output = 1;
                break;
            case '?':
                break;
		}
	}
	
	if(flag_protein_output){
		para->OUTPUT = 0;
	}
    
	if(argc-optind == 0 ){
		wrong_input();
		error("conf-directory and infile needed !");
	}
	else if(argc-optind > 3){
        fprintf(stderr, "%d %d\n", argc, optind);
		wrong_input();
		error("too many arguments -> conf-directory, infile, [outfile] !");
	}
    else{
		struct stat attribut_dir;
    	if(only_config_dir){
			if(stat(argv[optind],&attribut_dir)==-1){
				wrong_input();
				error("conf-directory doesn't exist");
			}
			else {
				if(attribut_dir.st_mode & S_IFDIR){
					para->conf_dir = argv[optind++];
				}
				else {
					wrong_input();
					error("conf-directory is no directory");
				}
			}
		}
		else{
			struct stat attribut_file;
			if(stat(argv[optind],&attribut_dir)==-1){
				wrong_input();
				error("conf-directory doesn't exist");
			}
			else{
				if(attribut_dir.st_mode & S_IFDIR){
					para->conf_dir = argv[optind++];
					if(stat(argv[optind],&attribut_file)==-1){
						wrong_input();
						error("infile doesn't exist");
					}
					else{
						if(attribut_file.st_mode & S_IFREG){
							para->in_file = argv[optind++];
						}
						else{
							wrong_input();
							error("infile isn't a regular file!");
						}
					}
				}
				else{
					wrong_input();
					error("conf-directory is no directory");
				}
			}
			if(argv-optind >0)
				para->out_file = argv[optind];
		}
	}
	return;
}
*/

#pragma mark -
#pragma mark io

////////////////////////////////////////////////////////////////////////////////
// io.c
////////////////////////////////////////////////////////////////////////////////

/**
 * print error message and exit
 */
void error(char *message)
{
    printf("ERROR: %s\n", message);
    //  if(errno) perror("perror()");
    exit(1);
}

/**
 * print error message and exit
 */
void merror(char *msg1, char *msg2)
{
    printf("ERROR: %s %s\n", msg1, msg2);
    //  if(errno) perror("perror()");
    exit(1);
}

/**
 * strips off leading whitespace characters
 */
void strip_leading_ws( char *str )
{
    int s,d;
    for(s=d=0; str[s]==' ' || str[s]=='\t'; s++){}
    for(;;s++,d++){
        str[d]=str[s];
        if( str[s] == '\0')break;
    }
}

/**
 * builds a pathname from a dir-name and a filename.
 *
 * The pointer returned (and the ones included in the struct)
 * has to be deallocted explicitely from memory.
 */
char* build_pathname(char *dir, char *file)
{
    int dirslen = (int) strlen(dir);
    
    char *pathn = calloc(dirslen+strlen(file)+2, sizeof(char));
    if(pathn==NULL) error("build_pathname(): out of memory !");
    
    strcpy(pathn, dir);
    if(dirslen>0 && dir[dirslen-1]!='/') strcat(pathn, "/");
    strcat(pathn, file);
    //  if(para->DEBUG)
    //    printf("DEBUG build_pathname(): Created filename: %s from dir=%s and file=%s\n", pathn, dir, file);
    
    return pathn;
}


/**
 * prints a sequence
 */
void print_seq(struct seq * aSeq)
{
    int row;
    int slen = aSeq->length;
    int maxrow = slen/para->PRINT_SEQ_LINE_LENGTH;
    int row_residue = slen % para->PRINT_SEQ_LINE_LENGTH;
    
    printf("Sequence: %s\nLength: %i\n", aSeq->name, slen);
    
    
    char line[para->PRINT_SEQ_LINE_LENGTH+1];
    
    for(row=0;row <=maxrow; row++) {
        if(row<maxrow) {
            strncpy(line, &(aSeq->data[row*para->PRINT_SEQ_LINE_LENGTH]),
                    para->PRINT_SEQ_LINE_LENGTH);
            line[para->PRINT_SEQ_LINE_LENGTH]='\0';
        } else{
            if(row_residue==0) break;
            strncpy(line, &(aSeq->data[row*para->PRINT_SEQ_LINE_LENGTH]),
                    row_residue);
            line[row_residue]='\0';
        }
        printf("%s\n", line);
    }
    
}

/**
 * prints a diagional
 */
void print_diag(struct diag *aDiag)
{
    int row;
    int slen = aDiag->length;
    int maxrow = slen/para->PRINT_SEQ_LINE_LENGTH;
    int row_residue = slen % para->PRINT_SEQ_LINE_LENGTH;
    
    printf("Diag: %s\n      %s\nLength: %i startpos1: %i startpos2: %i\n", aDiag->seq_p1.sq->name,
           aDiag->seq_p2.sq->name, slen, aDiag->seq_p1.startpos,aDiag->seq_p2.startpos );
    
    char *data1 =aDiag->seq_p1.sq->data;
    char *data2 =aDiag->seq_p2.sq->data;
    char *data;
    
    int startpos1 = aDiag->seq_p1.startpos;
    int startpos2 = aDiag->seq_p2.startpos;
    int snum, startpos;
    
    char line[para->PRINT_SEQ_LINE_LENGTH+1];
    
    for(row=0;row <=maxrow; row++) {
        for (snum=0;snum<2;snum++) {
            startpos = (snum==0 ? startpos1 : startpos2);
            data = (snum==0 ? data1 : data2);
            if(row<maxrow) {
                strncpy(line, &(data[startpos+row*para->PRINT_SEQ_LINE_LENGTH]),
                        para->PRINT_SEQ_LINE_LENGTH);
                line[para->PRINT_SEQ_LINE_LENGTH]='\0';
            } else{
                if(row_residue==0) break;
                strncpy(line, &(data[startpos+row*para->PRINT_SEQ_LINE_LENGTH]),
                        row_residue);
                line[row_residue]='\0';
            }
            printf("%s\n", line);
        }
        if(row<maxrow) printf("\n");
    }
    printf("Score: %li Weight: %e \n", aDiag->score, aDiag->weight);
}

/**
 * prints a score matrix
 */
void print_scr_matrix(struct scr_matrix *aSmatrix)
{
    int len =aSmatrix->length;
    printf("Length: %i, maximal score: %i\n", len, aSmatrix->max_score);
    
    int r, c;
    for (r=-1; r<len;r++) {
        for (c=0; c<=len;c++) {
            if(r==-1) {
                if(c<len) printf("  %c ", aSmatrix->num2char[c]);
            } else if(c<len) {
                printf("%3i ", aSmatrix->data[len*r+c]);
            } else {
                printf(" %c ", aSmatrix->num2char[r]);
            }
        }
        printf("\n");
    }
}

/**
 * reads score matrix from the file
 * indicated by filename parameter.
 *
 * The pointer returned (and the ones included in the struct)
 * has to be deallocted explicitely from memory.
 */
struct scr_matrix* read_scr_matrix(char *filename)
{
    
    if(para->DEBUG)
        printf("DEBUG read_scr_matrix(): Processing input file: %s\n", filename);
    
    int length = 0;
    
    struct scr_matrix *smatrix = calloc(1, sizeof(struct scr_matrix));
    int *char2num = (smatrix->char2num =calloc(256, sizeof(int)));
    int *num2char = (smatrix->num2char =calloc(256, sizeof(int)));
    
    if(smatrix==NULL || char2num==NULL) error("read_scr_matrix(): Out of memory !");
    
    FILE *fp;
    
    char rline[100];
    
    if( (fp = fopen( filename , "r")) == NULL) {
        merror("read_scr_matrix(): Cannot open input score matrix file", filename );
    }
    
    int sret;
    char amino;
    while( (sret=fscanf(fp, " %c ", &amino))!=EOF && sret>0) {
        if(amino=='#') {
            break;
        } else {
            num2char[length]=amino;
            char2num[(int)amino]=length++;
        }
    }
    
    num2char[length]='?';
    char2num[(int)'?']=length++;
    num2char[length]='#';
    char2num[(int)'#']=length++;
    num2char[length]='$';
    char2num[(int)'$']=length++;
    int additional=3;
    
    if(sret==EOF) merror("read_scr_matrix(): Unexpected end of file ",filename);
    if(length==0) merror("read_scr_matrix(): Invalid format of file ",filename);
    
    smatrix->length = length;
    int *data = (smatrix->data = calloc(length*length, sizeof(int)));
    if(data==NULL) error("read_scr_matrix(): Out of memory when allocating data !");
    
    // read the matrix entries
    int r,c;
    int is;//,tis;
    long double frac = (long double)2.0/(long double)(length*length);
    long double avg_score = 0.0;
    // generate Array from Matrix
    for( r=0; r<length; r++) {
        for( c=r; c<length; c++) {
            // check whether it is a regular acid or a special character like '$',...
            if( (r<length-additional) && (c<length-additional)) {
                fscanf( fp, "%i", &is);
            } else {
                is = 0;
            }
            //tis = is;
            is +=para->SCR_MATRIX_ADD;
            if(smatrix->max_score<is) smatrix->max_score =is;
            avg_score += (long double)is*frac;
            data[length*r+c] = is;
            // ensure symmetry of the weight matrix
            data[length*c+r] = is;
        }
        fscanf(fp, "%s\n", rline);
    }
    fclose(fp);
    
    // Array is a symmetric Matrix
    /************************************ vvv Vorsicht !!! ***********/
    smatrix->avg_sim_score = para->PROT_SIM_SCORE_THRESHOLD;
    int ms = smatrix->max_score;
    
    int *dist = (smatrix->dist=calloc(ms+1, sizeof(int)));
    for(r=0;r<ms;r++) {
        dist[r]=0;
    }
    // count how often score x appears and save in dist[x]
    for(r=0;r<length;r++) {
        for(c=0;c<length;c++) {
            if(num2char[r]!='X' && (num2char[r] >='A') && (num2char[r]<='Z'))
                if(num2char[c]!='X' && (num2char[c] >='A') && (num2char[c]<='Z'))
                    dist[data[length*r+c]]++;
        }
    }
    return smatrix;
}

// We read the matrix from the header file.
// This follows closely function read_scr_matrix().
struct scr_matrix* read_scr_matrix_from_code()
{
    int upperDiagonalMatrix[10] = {1,0,0,0,1,0,0,1,0,1};
    size_t upperDiagonalMatrixIndex = 0;
    int length = 0;
    
    struct scr_matrix *smatrix = calloc(1, sizeof(struct scr_matrix));
    int *char2num = (smatrix->char2num =calloc(256, sizeof(int)));
    int *num2char = (smatrix->num2char =calloc(256, sizeof(int)));
    
    if(smatrix==NULL || char2num==NULL) error("read_scr_matrix(): Out of memory !");
    
    char amino;
    
    amino = 'A'; num2char[length]=amino; char2num[(int)amino]=length++;
    amino = 'T'; num2char[length]=amino; char2num[(int)amino]=length++;
    amino = 'G'; num2char[length]=amino; char2num[(int)amino]=length++;
    amino = 'C'; num2char[length]=amino; char2num[(int)amino]=length++;
    
    num2char[length]='?';
    char2num[(int)'?']=length++;
    num2char[length]='#';
    char2num[(int)'#']=length++;
    num2char[length]='$';
    char2num[(int)'$']=length++;
    int additional=3;
    
    smatrix->length = length;
    int *data = (smatrix->data = calloc(length*length, sizeof(int)));
    if(data==NULL) error("read_scr_matrix(): Out of memory when allocating data !");
    
    // read the matrix entries
    int r,c;
    int is;//,tis;
    long double frac = (long double)2.0/(long double)(length*length);
    long double avg_score = 0.0;
    // generate Array from Matrix
    for( r=0; r<length; r++) {
        for( c=r; c<length; c++) {
            // check whether it is a regular acid or a special character like '$',...
            if( (r<length-additional) && (c<length-additional)) {
                is = upperDiagonalMatrix[upperDiagonalMatrixIndex++];
            } else {
                is = 0;
            }
            //tis = is;
            is +=para->SCR_MATRIX_ADD;
            if(smatrix->max_score<is) smatrix->max_score =is;
            avg_score += (long double)is*frac;
            data[length*r+c] = is;
            // ensure symmetry of the weight matrix
            data[length*c+r] = is;
        }
    }
    assert(upperDiagonalMatrixIndex == 10);
    
    // Array is a symmetric Matrix
    /************************************ vvv Vorsicht !!! ***********/
    smatrix->avg_sim_score = para->PROT_SIM_SCORE_THRESHOLD;
    int ms = smatrix->max_score;
    
    int *dist = (smatrix->dist=calloc(ms+1, sizeof(int)));
    for(r=0;r<ms;r++) {
        dist[r]=0;
    }
    // count how often score x appears and save in dist[x]
    for(r=0;r<length;r++) {
        for(c=0;c<length;c++) {
            if(num2char[r]!='X' && (num2char[r] >='A') && (num2char[r]<='Z'))
                if(num2char[c]!='X' && (num2char[c] >='A') && (num2char[c]<='Z'))
                    dist[data[length*r+c]]++;
        }
    }
    return smatrix;
}

/**
 * reads the probability distribution for diagonal lengths from the file
 * indicated by filename parameter.
 *
 * The pointer returned (and the ones included in the struct)
 * has to be deallocted explicitely from memory.
 */
struct prob_dist* read_diag_prob_dist(struct scr_matrix* smatrix, char *filename)
{
    
    if(para->DEBUG>1)
        printf("DEBUG read_diag_prob_dist(): Processing input file: %s\n", filename);
    
    int length = 0;
    
    struct prob_dist *pdist = calloc(1, sizeof(struct prob_dist));
    pdist->smatrix = smatrix;
    
    if(pdist==NULL) error("read_diag_prob_dist(): Out of memory !");
    
    FILE *fp;
    
    
    if( (fp = fopen( filename , "r")) == NULL) {
        merror("read_diag_prob_dist(): Cannot open input file", filename );
    }
    
    int sret;
    sret=fscanf(fp, "%i\n", &length);
    if(sret<=0)
        merror("read_diag_prob_dist(): Invalid format in file", filename );
    
    if(length==0) merror("read_scr_matrix(): Invalid format of file ",filename);
    
    //  fscanf(fp, "%s\n", rline);
    //  printf("rline:%s %i\n",rline, length);
    
    //  length=40;
    
    pdist->max_dlen = length;
    pdist->data = calloc(length+1, sizeof(long double *));
    long double **dist =pdist->data;
    if(dist==NULL) error("read_diag_prob_dist(): (1) Out of memory when allocating data !");
    
    pdist->log_data = calloc(length+1, sizeof(double *));
    double **log_dist =pdist->log_data;
    if(log_dist==NULL) error("read_diag_prob_dist(): (1.1) Out of memory when allocating data !");
    
    
    // read the entries
    unsigned long i, scr, mxscr, sm_max_scr=smatrix->max_score;
    unsigned long ti, tscr;
    long double weight;
    long size=0;
    
    for( i=1; i<=length; i++) {
        mxscr = i*sm_max_scr;
        size += mxscr+1;
        dist[i] = calloc(mxscr+1, sizeof(long double ));
        log_dist[i] = calloc(mxscr+1, sizeof(long double ));
        if(dist[i]==NULL) error("read_diag_prob_dist(): (3) Out of memory at iteration" );
        for(scr=0;scr<=mxscr;scr++) {
            dist[i][scr]=1.0;
        }
        for(scr=0;scr<=mxscr;scr++) {
            dist[i][scr]=1.0;
            fscanf( fp, "%li %li %Le\n", &ti,&tscr,&weight );
            //if(i!=ti || tscr!=scr) merror("read_scr_matrix(): (4) Invalid format of file ",filename);
            scr = tscr;
            if(weight==0.0) weight = 1.0;
            dist[i][scr]=weight;
            log_dist[i][scr]=-log(weight);
            if(para->DEBUG>5)printf("%li %li %Le\n", i, scr,dist[i][scr] );
        }
    }
    if(para->DEBUG >1) printf("DEBUG: PROB DIST SIZE: %li bytes\n", size*sizeof(long double)+length*sizeof(long double *));
    
    //pdist->avg_sim_score = 4;
    
    return pdist;
}

// We read the pdist from the code not from a file.
// We closely follow read_diag_prob_dist() function.
// The default was dna_diag_prob_100_exp_550000,
// which can be verified from set_parameters() function.
struct prob_dist* read_diag_prob_dist_from_code(struct scr_matrix* smatrix)
{
    long double weightMatrix[5150] = {1.000000e+00,6.730455e-01,1.000000e+00,9.670818e-01,4.120255e-01,1.000000e+00,9.979000e-01,8.519509e-01,1.974073e-01,1.000000e+00,9.998618e-01,9.809545e-01,5.966600e-01,7.885273e-02,1.000000e+00,9.999909e-01,9.982182e-01,8.905636e-01,3.229873e-01,2.844545e-02,1.000000e+00,1.000000e+00,9.998564e-01,9.827782e-01,6.597564e-01,1.446764e-01,9.710909e-03,1.000000e+00,1.000000e+00,9.999964e-01,9.980182e-01,8.957327e-01,3.821818e-01,5.771455e-02,3.210909e-03,1.000000e+00,1.000000e+00,1.000000e+00,9.998164e-01,9.798309e-01,6.726545e-01,1.831455e-01,2.081818e-02,1.034545e-03,1.000000e+00,1.000000e+00,1.000000e+00,9.999891e-01,9.971818e-01,8.838709e-01,4.005764e-01,7.684182e-02,7.283636e-03,3.327273e-04,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.996382e-01,9.730073e-01,6.584691e-01,1.977636e-01,2.972182e-02,2.398182e-03,1.127273e-04,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999655e-01,9.953236e-01,8.598509e-01,3.929982e-01,8.693273e-02,1.099091e-02,7.872727e-04,3.272727e-05,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999945e-01,9.993891e-01,9.605727e-01,6.266636e-01,1.981436e-01,3.510364e-02,3.932727e-03,2.781818e-04,1.007318e-05,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999000e-01,9.921364e-01,8.258491e-01,3.724073e-01,8.930182e-02,1.366727e-02,1.394545e-03,7.818182e-05,2.920628e-06,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999964e-01,9.987527e-01,9.410745e-01,5.849291e-01,1.902582e-01,3.750727e-02,5.054545e-03,4.563636e-04,3.454545e-05,8.381903e-07,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.998218e-01,9.857345e-01,7.810727e-01,3.435873e-01,8.694000e-02,1.504909e-02,1.870909e-03,1.400000e-04,1.096725e-05,2.384186e-07,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999818e-01,9.972691e-01,9.120782e-01,5.356636e-01,1.751473e-01,3.724182e-02,5.770909e-03,6.545455e-04,4.363636e-05,3.297115e-06,6.728806e-08,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.996127e-01,9.740327e-01,7.278618e-01,3.094855e-01,8.132909e-02,1.516182e-02,2.134545e-03,2.309091e-04,2.406444e-05,9.806827e-07,1.885928e-08,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999382e-01,9.941927e-01,8.733745e-01,4.831091e-01,1.582600e-01,3.553818e-02,6.020000e-03,8.090909e-04,8.000000e-05,7.522642e-06,2.889283e-07,5.253241e-09,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999927e-01,9.989600e-01,9.548018e-01,6.672091e-01,2.747782e-01,7.364909e-02,1.470364e-02,2.361818e-03,3.127273e-04,3.454545e-05,2.323941e-06,8.440111e-08,1.455192e-09,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999945e-01,9.998364e-01,9.880182e-01,8.238018e-01,4.288382e-01,1.396818e-01,3.265455e-02,5.916364e-03,9.327273e-04,1.145455e-04,1.305579e-05,7.103254e-07,2.446632e-08,4.010872e-10,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999709e-01,9.974091e-01,9.260218e-01,6.026582e-01,2.398145e-01,6.590545e-02,1.380364e-02,2.380000e-03,3.672727e-04,4.363636e-05,4.166890e-06,2.150355e-07,7.043127e-09,1.100489e-10,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999964e-01,9.995200e-01,9.760564e-01,7.642291e-01,3.756655e-01,1.211818e-01,2.931455e-02,5.654545e-03,9.218182e-04,1.345455e-04,1.913187e-05,1.314848e-06,6.453058e-08,2.014701e-09,3.007017e-11,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999091e-01,9.939800e-01,8.861836e-01,5.363855e-01,2.062655e-01,5.769273e-02,1.282000e-02,2.223636e-03,3.818182e-04,4.909091e-05,6.281672e-06,4.106150e-07,1.921126e-08,5.729817e-10,8.185452e-12,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999873e-01,9.987582e-01,9.557455e-01,6.970909e-01,3.241800e-01,1.044855e-01,2.595818e-02,5.249091e-03,9.036364e-04,1.381818e-04,2.497199e-05,2.038172e-06,1.270206e-07,5.677681e-09,1.620926e-10,2.220446e-12,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.997836e-01,9.863527e-01,8.327455e-01,4.696527e-01,1.758091e-01,4.943091e-02,1.134000e-02,2.156364e-03,3.509091e-04,6.727273e-05,8.405790e-06,6.541608e-07,3.895211e-08,1.666734e-09,4.563105e-11,6.004086e-13,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999618e-01,9.967691e-01,9.239927e-01,6.252236e-01,2.766891e-01,8.827091e-02,2.257636e-02,4.838182e-03,8.618182e-04,1.400000e-04,2.909091e-05,2.795292e-06,2.078667e-07,1.184957e-08,4.862590e-10,1.278777e-11,1.618705e-13,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999945e-01,9.993291e-01,9.726164e-01,7.696218e-01,4.050855e-01,1.481582e-01,4.218000e-02,1.004000e-02,1.972727e-03,3.290909e-04,6.727273e-05,1.031061e-05,9.192084e-07,6.544501e-08,3.578101e-09,1.410507e-10,3.568701e-12,4.352074e-14,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999000e-01,9.922545e-01,8.798709e-01,5.509618e-01,2.333200e-01,7.442364e-02,1.933455e-02,4.320000e-03,7.781818e-04,1.381818e-04,2.727273e-05,3.504586e-06,2.991620e-07,2.042947e-08,1.073040e-09,4.069754e-11,9.920537e-13,1.167122e-14,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999873e-01,9.982291e-01,9.487145e-01,6.981673e-01,3.453327e-01,1.235055e-01,3.544727e-02,8.707273e-03,1.783636e-03,3.090909e-04,5.090909e-05,1.183615e-05,1.177724e-06,9.643441e-08,6.326911e-09,3.197442e-10,1.168440e-11,2.747802e-13,3.122502e-15,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.996145e-01,9.822418e-01,8.227145e-01,4.789527e-01,1.952782e-01,6.181273e-02,1.640727e-02,3.714545e-03,7.036364e-04,1.200000e-04,3.611135e-05,4.102757e-06,3.916149e-07,3.080943e-08,1.944996e-09,9.471121e-11,3.339140e-12,7.585165e-14,8.335346e-16,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999218e-01,9.951964e-01,9.121473e-01,6.207782e-01,2.910200e-01,1.022600e-01,2.944727e-02,7.225455e-03,1.632727e-03,2.781818e-04,5.454545e-05,1.289846e-05,1.405896e-06,1.289439e-07,9.761676e-09,5.938159e-10,2.789857e-11,9.501289e-13,2.087219e-14,2.220446e-16,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999800e-01,9.989327e-01,9.638855e-01,7.548182e-01,4.098745e-01,1.615600e-01,5.084727e-02,1.364182e-02,3.136364e-03,6.763636e-04,1.345455e-04,2.363636e-05,4.550656e-06,4.766309e-07,4.206818e-08,3.068960e-09,1.801294e-10,8.175199e-12,2.692578e-13,5.726376e-15,5.903481e-17,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.997782e-01,9.881109e-01,8.623436e-01,5.440909e-01,2.428600e-01,8.408545e-02,2.426545e-02,6.121818e-03,1.385455e-03,2.618182e-04,5.818182e-05,1.347999e-05,1.587125e-06,1.599811e-07,1.360743e-08,9.578529e-10,5.431157e-11,2.383911e-12,7.601493e-14,1.566672e-15,1.566672e-17,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999491e-01,9.967473e-01,9.345491e-01,6.803855e-01,3.466745e-01,1.326655e-01,4.164000e-02,1.138000e-02,2.674545e-03,6.127273e-04,1.236364e-04,3.272727e-05,4.832540e-06,5.476121e-07,5.319727e-08,4.366179e-09,2.969215e-10,1.628299e-11,6.919649e-13,2.138318e-14,4.274975e-16,4.150461e-18,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999891e-01,9.993000e-01,9.741855e-01,8.002673e-01,4.693836e-01,2.005109e-01,6.863273e-02,2.008182e-02,4.994545e-03,1.203636e-03,2.509091e-04,6.909091e-05,1.361253e-05,1.712671e-06,1.870482e-07,1.753457e-08,1.390408e-09,9.145275e-11,4.855738e-12,1.999846e-13,5.994838e-15,1.163620e-16,1.097755e-18,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.998473e-01,9.916673e-01,8.920873e-01,6.013491e-01,2.898691e-01,1.085455e-01,3.413636e-02,9.327273e-03,2.260000e-03,5.000000e-04,1.254545e-04,2.727273e-05,4.951677e-06,6.004733e-07,6.328771e-08,5.732104e-09,4.396344e-10,2.799794e-11,1.440749e-12,5.756166e-14,1.675315e-15,3.159878e-17,2.898970e-19,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999709e-01,9.978782e-01,9.506036e-01,7.282236e-01,3.987636e-01,1.646855e-01,5.604727e-02,1.618545e-02,4.296364e-03,1.007273e-03,2.272727e-04,5.272727e-05,1.335903e-05,1.780765e-06,2.084087e-07,2.122332e-08,1.859323e-09,1.380787e-10,8.522708e-12,4.254562e-13,1.650411e-14,4.667715e-16,8.561809e-18,7.644472e-20,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.995291e-01,9.810600e-01,8.353509e-01,5.225836e-01,2.401600e-01,8.822364e-02,2.739455e-02,7.583636e-03,1.885455e-03,4.309091e-04,1.000000e-04,2.909091e-05,4.924639e-06,6.335722e-07,7.164674e-08,7.057612e-09,5.986987e-10,4.309330e-11,2.580401e-12,1.250740e-13,4.714790e-15,1.296792e-16,2.314981e-18,2.013027e-20,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.998836e-01,9.940309e-01,9.138745e-01,6.495091e-01,3.354945e-01,1.346200e-01,4.504364e-02,1.313636e-02,3.481818e-03,8.290909e-04,1.836364e-04,5.636364e-05,1.279770e-05,1.794965e-06,2.231459e-07,2.441015e-08,2.328391e-09,1.914474e-10,1.336871e-11,7.772841e-13,3.661232e-14,1.342230e-15,3.593008e-17,6.246868e-19,5.293956e-21,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999855e-01,9.985218e-01,9.615964e-01,7.668636e-01,4.470873e-01,1.971200e-01,7.127636e-02,2.214545e-02,6.107273e-03,1.563636e-03,4.000000e-04,9.818182e-05,2.727273e-05,4.775773e-06,6.472909e-07,7.784511e-08,8.246177e-09,7.624211e-10,6.081897e-11,4.123850e-12,2.330079e-13,1.067409e-14,3.808568e-16,9.929493e-18,1.682494e-19,1.390491e-21,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.996836e-01,9.854945e-01,8.630436e-01,5.692127e-01,2.783564e-01,1.091036e-01,3.642364e-02,1.052545e-02,2.803636e-03,7.127273e-04,1.727273e-04,4.363636e-05,1.200995e-05,1.762333e-06,2.310793e-07,2.691219e-08,2.763383e-09,2.478829e-10,1.920108e-11,1.265251e-12,6.952903e-14,3.100028e-15,1.077303e-16,2.737359e-18,4.523354e-20,3.647867e-22,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999400e-01,9.956200e-01,9.298000e-01,6.904818e-01,3.770509e-01,1.607691e-01,5.732000e-02,1.790182e-02,4.956364e-03,1.287273e-03,3.236364e-04,8.909091e-05,2.870078e-05,4.532606e-06,6.434737e-07,8.171026e-08,9.224648e-09,9.190054e-10,8.005158e-11,6.026240e-12,3.862141e-13,2.065689e-14,8.970424e-16,3.038228e-17,7.528752e-19,1.214006e-20,9.559106e-23,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999873e-01,9.989127e-01,9.693127e-01,7.987545e-01,4.900527e-01,2.294218e-01,8.803091e-02,2.904545e-02,8.496364e-03,2.352727e-03,5.927273e-04,1.581818e-04,3.454545e-05,1.107221e-05,1.691782e-06,2.326039e-07,2.863275e-08,3.136358e-09,3.034254e-10,2.568689e-11,1.880737e-12,1.173182e-13,6.111650e-15,2.586715e-16,8.544185e-18,2.066084e-19,3.252888e-21,2.502221e-23,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999945e-01,9.997709e-01,9.887491e-01,8.835436e-01,6.091655e-01,3.144418e-01,1.304509e-01,4.616182e-02,1.412727e-02,4.100000e-03,1.101818e-03,2.872727e-04,6.545455e-05,2.580812e-05,4.222470e-06,6.248615e-07,8.328601e-08,9.947662e-09,1.058166e-09,9.949435e-11,8.192334e-12,5.838328e-13,3.547233e-14,1.801075e-15,7.434345e-17,2.396322e-18,5.657843e-20,8.702360e-22,6.543128e-24,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999564e-01,9.966509e-01,9.415818e-01,7.239727e-01,4.167873e-01,1.870564e-01,7.046545e-02,2.312182e-02,6.969091e-03,1.892727e-03,5.000000e-04,1.309091e-04,2.363636e-05,1.005114e-05,1.592761e-06,2.285070e-07,2.955320e-08,3.427961e-09,3.544035e-10,3.241173e-11,2.597673e-12,1.803171e-13,1.067811e-14,5.287662e-16,2.129909e-17,6.703430e-19,1.546225e-20,2.324636e-22,1.709291e-24,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999873e-01,9.991945e-01,9.749400e-01,8.239800e-01,5.291509e-01,2.600491e-01,1.046618e-01,3.656909e-02,1.147636e-02,3.390909e-03,8.654545e-04,2.163636e-04,6.363636e-05,2.291027e-05,3.870297e-06,5.946037e-07,8.277658e-08,1.039699e-08,1.172141e-09,1.178721e-10,1.049294e-11,8.191423e-13,5.542111e-14,3.200859e-15,1.546779e-16,6.083654e-18,1.870577e-19,4.217467e-21,6.200846e-23,4.461040e-25,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.998400e-01,9.908709e-01,9.002327e-01,6.445527e-01,3.496745e-01,1.514055e-01,5.606727e-02,1.855091e-02,5.649091e-03,1.465455e-03,4.400000e-04,1.018182e-04,2.181818e-05,9.001442e-06,1.474317e-06,2.197961e-07,2.971721e-08,3.627943e-09,3.978434e-10,3.894349e-11,3.376828e-12,2.569460e-13,1.695502e-14,9.556262e-16,4.509145e-17,1.732648e-18,5.207469e-20,1.148217e-21,1.651776e-23,1.163223e-25,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999691e-01,9.972855e-01,9.503600e-01,7.527745e-01,4.519164e-01,2.128200e-01,8.375636e-02,2.908182e-02,9.198182e-03,2.627273e-03,7.218182e-04,1.927273e-04,4.545455e-05,2.010826e-05,3.497392e-06,5.558840e-07,8.048867e-08,1.057759e-08,1.256115e-09,1.340855e-10,1.278501e-11,1.080571e-12,8.019253e-14,5.164104e-15,2.842071e-16,1.310165e-17,4.920988e-19,1.446419e-20,3.120494e-22,4.394208e-24,3.030488e-26,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999964e-01,9.993364e-01,9.790891e-01,8.448345e-01,5.628055e-01,2.895964e-01,1.218327e-01,4.456364e-02,1.467455e-02,4.443636e-03,1.227273e-03,3.490909e-04,9.090909e-05,4.327777e-05,7.965546e-06,1.344503e-06,2.075567e-07,2.921216e-08,3.734365e-09,4.316877e-10,4.488768e-11,4.171887e-12,3.439059e-13,2.490755e-14,1.566203e-15,8.421283e-17,3.794760e-18,1.393928e-19,4.008833e-21,8.466055e-23,1.167514e-24,7.888609e-27,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.998582e-01,9.925655e-01,9.130836e-01,6.745400e-01,3.807000e-01,1.719945e-01,6.671273e-02,2.311818e-02,7.265455e-03,2.103636e-03,5.945455e-04,1.854545e-04,4.363636e-05,1.747206e-05,3.120954e-06,5.116619e-07,7.677976e-08,1.051201e-08,1.308154e-09,1.473078e-10,1.493061e-11,1.353458e-12,1.088853e-13,7.700562e-15,4.730807e-16,2.486481e-17,1.095785e-18,3.938416e-20,1.108758e-21,2.293122e-23,3.098259e-25,2.051827e-27,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999818e-01,9.977582e-01,9.574091e-01,7.771127e-01,4.843073e-01,2.370891e-01,9.729818e-02,3.544000e-02,1.151636e-02,3.529091e-03,9.763636e-04,2.963636e-04,8.727273e-05,3.700954e-05,6.974399e-06,1.210079e-06,1.928461e-07,2.815128e-08,3.752039e-09,4.548453e-10,4.992671e-11,4.935757e-12,4.366615e-13,3.430342e-14,2.370237e-15,1.423408e-16,7.316749e-18,3.155025e-19,1.110044e-20,3.060458e-22,6.201397e-24,8.212358e-26,5.332700e-28,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999927e-01,9.994291e-01,9.819764e-01,8.613145e-01,5.942745e-01,3.165400e-01,1.389745e-01,5.301636e-02,1.834909e-02,5.754545e-03,1.683636e-03,5.000000e-04,1.636364e-04,2.909091e-05,1.504559e-05,2.754107e-06,4.645180e-07,7.201697e-08,1.023441e-08,1.328815e-09,1.570260e-10,1.681192e-11,1.622068e-12,1.401310e-13,1.075558e-14,7.264703e-16,4.266767e-17,2.146032e-18,9.058708e-20,3.121320e-21,8.431414e-23,1.674536e-24,2.174362e-26,1.384944e-28,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.998927e-01,9.935582e-01,9.235200e-01,7.005618e-01,4.099745e-01,1.929582e-01,7.798000e-02,2.814909e-02,9.010909e-03,2.787273e-03,8.090909e-04,2.509091e-04,6.363636e-05,3.140881e-05,6.048937e-06,1.076413e-06,1.766232e-07,2.665825e-08,3.690638e-09,4.671135e-10,5.384108e-11,5.625972e-12,5.300656e-13,4.474118e-14,3.356926e-15,2.217554e-16,1.274407e-17,6.274730e-19,2.593959e-20,8.756985e-22,2.318520e-23,4.515094e-25,5.750796e-27,3.594247e-29,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999673e-01,9.980673e-01,9.630091e-01,7.965091e-01,5.125364e-01,2.606636e-01,1.116873e-01,4.187091e-02,1.438727e-02,4.538182e-03,1.352727e-03,4.254545e-04,1.200000e-04,3.090909e-05,1.285200e-05,2.406249e-06,4.165814e-07,6.654741e-08,9.785129e-09,1.320579e-09,1.630338e-10,1.834062e-11,1.871477e-12,1.722800e-13,1.421521e-14,1.043135e-15,6.742650e-17,3.793308e-18,1.829139e-19,7.408627e-21,2.451469e-22,6.364255e-24,1.215710e-25,1.519405e-27,9.321501e-30,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999909e-01,9.995073e-01,9.845073e-01,8.751073e-01,6.197382e-01,3.424109e-01,1.563109e-01,6.148000e-02,2.202182e-02,7.229091e-03,2.185455e-03,6.818182e-04,2.054545e-04,5.818182e-05,2.647316e-05,5.201807e-06,9.475367e-07,1.597089e-07,2.485547e-08,3.562824e-09,4.690213e-10,5.651425e-11,6.208515e-12,6.189887e-13,5.570305e-14,4.495265e-15,3.227786e-16,2.042454e-17,1.125344e-18,5.316680e-20,2.110727e-21,6.848393e-23,1.743970e-24,3.268936e-26,4.010372e-28,2.415887e-30,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.998964e-01,9.944836e-01,9.311891e-01,7.227964e-01,4.365145e-01,2.129782e-01,8.882182e-02,3.311455e-02,1.130182e-02,3.514545e-03,1.089091e-03,3.145455e-04,9.636364e-05,2.545455e-05,1.089870e-05,2.083571e-06,3.695176e-07,6.067924e-08,9.206175e-09,1.287242e-09,1.653934e-10,1.946183e-11,2.089027e-12,2.036059e-13,1.792048e-14,1.415118e-15,9.947271e-17,6.164546e-18,3.327854e-19,1.541073e-20,5.999095e-22,1.909302e-23,4.771033e-25,8.778431e-27,1.057492e-28,6.257346e-31,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999691e-01,9.982818e-01,9.670418e-01,8.133236e-01,5.386000e-01,2.829636e-01,1.253164e-01,4.860545e-02,1.748545e-02,5.656364e-03,1.780000e-03,5.363636e-04,1.690909e-04,4.727273e-05,2.217481e-05,4.439106e-06,8.262776e-07,1.427695e-07,2.285578e-08,3.382612e-09,4.616369e-10,5.792491e-11,6.659900e-12,6.988516e-13,6.661924e-14,5.737602e-15,4.435469e-16,3.053546e-17,1.854109e-18,9.810829e-20,4.454908e-21,1.701125e-22,5.312699e-24,1.303145e-25,2.354406e-27,2.785896e-29,1.619707e-31,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999891e-01,9.996018e-01,9.863200e-01,8.862327e-01,6.426800e-01,3.667018e-01,1.722927e-01,7.062909e-02,2.618909e-02,8.863636e-03,2.780000e-03,8.654545e-04,2.836364e-04,8.363636e-05,4.389190e-05,9.181637e-06,1.789636e-06,3.245551e-07,5.467178e-08,8.537870e-09,1.233334e-09,1.643778e-10,2.015353e-11,2.265251e-12,2.324916e-13,2.168693e-14,1.828519e-15,1.384415e-16,9.338316e-18,5.557891e-19,2.883751e-20,1.284484e-21,4.813036e-23,1.475504e-24,3.553893e-26,6.306941e-28,7.332677e-30,4.190101e-32,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999964e-01,9.999127e-01,9.950327e-01,9.379564e-01,7.414873e-01,4.604600e-01,2.318527e-01,9.998182e-02,3.844182e-02,1.356727e-02,4.440000e-03,1.414545e-03,4.527273e-04,1.290909e-04,3.636364e-05,1.846973e-05,3.761991e-06,7.144417e-07,1.263181e-07,2.075750e-08,3.164058e-09,4.463720e-10,5.813116e-11,6.967641e-12,7.660021e-13,7.693110e-14,7.025349e-15,5.801376e-16,4.303671e-17,2.845480e-18,1.660648e-19,8.452131e-21,3.694305e-22,1.358839e-23,4.090516e-25,9.677654e-27,1.687512e-28,1.928337e-30,1.083336e-32,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999764e-01,9.984418e-01,9.702473e-01,8.273545e-01,5.613109e-01,3.040182e-01,1.385364e-01,5.582545e-02,2.043636e-02,6.976364e-03,2.280000e-03,7.400000e-04,2.327273e-04,7.272727e-05,2.363636e-05,7.688956e-06,1.525950e-06,2.825366e-07,4.873234e-08,7.816599e-09,1.163633e-09,1.604075e-10,2.042263e-11,2.394269e-12,2.575747e-13,2.532532e-14,2.265097e-15,1.832716e-16,1.332664e-17,8.640147e-19,4.946376e-20,2.470440e-21,1.059957e-22,3.828378e-24,1.132023e-25,2.631550e-27,4.510064e-29,5.066858e-31,2.799369e-33,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999945e-01,9.995945e-01,9.874273e-01,8.950982e-01,6.628473e-01,3.880236e-01,1.881018e-01,7.946909e-02,3.018182e-02,1.063455e-02,3.509091e-03,1.150909e-03,3.527273e-04,1.309091e-04,4.545455e-05,1.530465e-05,3.168073e-06,6.130048e-07,1.107264e-07,1.864204e-08,2.920318e-09,4.248067e-10,5.725076e-11,7.129487e-12,8.179200e-13,8.614410e-14,8.295594e-15,7.269902e-16,5.765781e-17,4.111223e-18,2.614679e-19,1.468878e-20,7.201500e-22,3.034106e-23,1.076440e-24,3.127496e-26,7.145780e-28,1.204044e-29,1.330276e-31,7.229760e-34,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999055e-01,9.954673e-01,9.430218e-01,7.576345e-01,4.823636e-01,2.498327e-01,1.111527e-01,4.388727e-02,1.600727e-02,5.414545e-03,1.745455e-03,5.872727e-04,1.963636e-04,6.000000e-05,2.971200e-05,6.403902e-06,1.292477e-06,2.439800e-07,4.301795e-08,7.073523e-09,1.082786e-09,1.539895e-10,2.029911e-11,2.473719e-12,2.778383e-13,2.866035e-14,2.704311e-15,2.323072e-16,1.806687e-17,1.263706e-18,7.886742e-20,4.349284e-21,2.093878e-22,8.665508e-24,3.020800e-25,8.626352e-27,1.937777e-28,3.211009e-30,3.489822e-32,1.866215e-34,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999782e-01,9.985273e-01,9.728036e-01,8.386218e-01,5.818691e-01,3.231600e-01,1.519018e-01,6.306909e-02,2.366000e-02,8.256364e-03,2.745455e-03,9.290909e-04,2.745455e-04,1.036364e-04,3.090909e-05,1.262229e-05,2.652579e-06,5.222986e-07,9.624253e-08,1.657350e-08,2.663040e-09,3.985459e-10,5.544069e-11,7.151814e-12,8.532648e-13,9.386522e-14,9.487489e-15,8.775163e-16,7.391887e-17,5.639350e-18,3.870787e-19,2.371410e-20,1.284179e-21,6.072903e-23,2.469506e-24,8.461326e-26,2.375578e-27,5.247986e-29,8.554499e-31,9.148167e-33,4.814825e-35,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.995673e-01,9.884709e-01,9.025364e-01,6.803455e-01,4.089236e-01,2.032127e-01,8.823091e-02,3.457636e-02,1.244364e-02,4.245455e-03,1.403636e-03,4.454545e-04,1.563636e-04,4.363636e-05,2.429234e-05,5.307045e-06,1.088091e-06,2.091422e-07,3.764001e-08,6.334051e-09,9.950509e-10,1.456645e-10,1.982948e-11,2.504367e-12,2.926505e-13,3.154507e-14,3.125437e-15,2.834734e-16,2.342444e-17,1.753691e-18,1.181631e-19,7.108705e-21,3.781370e-22,1.757087e-23,7.022788e-25,2.365732e-26,6.531982e-28,1.419496e-29,2.276751e-31,2.396319e-33,1.241616e-35,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999073e-01,9.958218e-01,9.467673e-01,7.717836e-01,5.023564e-01,2.667618e-01,1.216418e-01,4.977636e-02,1.856727e-02,6.527273e-03,2.114545e-03,7.218182e-04,2.127273e-04,6.909091e-05,4.570693e-05,1.036508e-05,2.209270e-06,4.421772e-07,8.301201e-08,1.459960e-08,2.402039e-09,3.691116e-10,5.287847e-11,7.047590e-12,8.717975e-13,9.982346e-14,1.054755e-14,1.024783e-15,9.117874e-17,7.393733e-18,5.433866e-19,3.595355e-20,2.124675e-21,1.110521e-22,5.071958e-24,1.993065e-25,6.602797e-27,1.793391e-28,3.834823e-30,6.053656e-32,6.272555e-34,3.200283e-36,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999727e-01,9.986800e-01,9.743327e-01,8.483073e-01,5.999273e-01,3.415018e-01,1.643382e-01,6.988000e-02,2.721636e-02,9.720000e-03,3.261818e-03,1.074545e-03,3.327273e-04,1.018182e-04,2.909091e-05,1.978679e-05,4.377932e-06,9.109396e-07,1.780794e-07,3.267067e-08,5.617875e-09,9.041309e-10,1.359651e-10,1.907033e-11,2.489505e-12,3.017574e-13,3.387003e-14,3.509452e-15,3.344901e-16,2.920528e-17,2.324854e-18,1.677829e-19,1.090499e-20,6.332189e-22,3.253082e-23,1.460748e-24,5.645127e-26,1.839710e-27,4.916764e-29,1.034766e-30,1.608103e-32,1.640753e-34,8.244991e-37,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999964e-01,9.996564e-01,9.892164e-01,9.085345e-01,6.953600e-01,4.264436e-01,2.178000e-01,9.685636e-02,3.918909e-02,1.462000e-02,5.047273e-03,1.730909e-03,5.400000e-04,1.727273e-04,5.454545e-05,3.696268e-05,8.477655e-06,1.831149e-06,3.721599e-07,7.109862e-08,1.275342e-08,2.145197e-09,3.378709e-10,4.974639e-11,6.834239e-12,8.742175e-13,1.038745e-13,1.143340e-14,1.162160e-15,1.087000e-16,9.316973e-18,7.283141e-19,5.163188e-20,3.297431e-21,1.881974e-22,9.505793e-24,4.197813e-25,1.595852e-26,5.117440e-28,1.346098e-29,2.788948e-31,4.267918e-33,4.288938e-35,2.123237e-37,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999091e-01,9.960218e-01,9.500491e-01,7.830400e-01,5.190255e-01,2.823655e-01,1.322473e-01,5.526364e-02,2.137091e-02,7.643636e-03,2.589091e-03,8.781818e-04,2.727273e-04,9.818182e-05,2.181818e-05,1.606108e-05,3.596260e-06,7.587403e-07,1.507004e-07,2.814974e-08,4.939385e-09,8.130997e-10,1.253855e-10,1.808261e-11,2.434270e-12,3.052438e-13,3.556715e-14,3.840482e-15,3.830887e-16,3.517491e-17,2.960674e-18,2.273444e-19,1.583675e-20,9.941117e-22,5.578389e-23,2.771022e-24,1.203781e-25,4.503013e-27,1.421213e-28,3.680320e-30,7.508525e-32,1.131712e-33,1.120399e-35,5.465360e-38,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999673e-01,9.987545e-01,9.754800e-01,8.571036e-01,6.147436e-01,3.579145e-01,1.770345e-01,7.712364e-02,3.093455e-02,1.142182e-02,4.003636e-03,1.376364e-03,3.981818e-04,1.436364e-04,3.818182e-05,2.980109e-05,6.908438e-06,1.510983e-06,3.115482e-07,6.050374e-08,1.105559e-08,1.898527e-09,3.059943e-10,4.621947e-11,6.531641e-12,8.619517e-13,1.059931e-13,1.211584e-14,1.283857e-15,1.257197e-16,1.133580e-17,9.372644e-19,7.071979e-20,4.842130e-21,2.988432e-22,1.649208e-23,8.059003e-25,3.444902e-26,1.268323e-27,3.940849e-29,1.004900e-30,2.019292e-32,2.998366e-34,2.924961e-36,1.406231e-38,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999945e-01,9.996545e-01,9.895091e-01,9.136618e-01,7.081473e-01,4.425527e-01,2.320400e-01,1.058727e-01,4.385091e-02,1.680182e-02,5.967273e-03,2.050909e-03,6.600000e-04,2.145455e-04,6.727273e-05,2.000000e-05,1.299508e-05,2.942654e-06,6.290003e-07,1.268119e-07,2.409148e-08,4.308299e-09,7.243892e-10,1.143620e-10,1.692706e-11,2.344964e-12,3.034710e-13,3.660913e-14,4.106721e-15,4.272031e-16,4.108083e-17,3.638674e-18,2.956248e-19,2.192480e-20,1.475948e-21,8.958564e-23,4.863479e-24,2.338529e-25,9.838693e-27,3.566119e-28,1.091098e-29,2.740347e-31,5.424843e-33,7.937298e-35,7.631322e-37,3.616740e-39,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.999164e-01,9.961473e-01,9.527655e-01,7.929055e-01,5.340600e-01,2.971418e-01,1.422764e-01,6.150909e-02,2.423273e-02,8.849091e-03,3.105455e-03,1.063636e-03,3.072727e-04,1.236364e-04,3.090909e-05,2.395975e-05,5.610527e-06,1.241665e-06,2.595177e-07,5.118347e-08,9.516609e-09,1.666332e-09,2.744391e-10,4.245695e-11,6.160422e-12,8.369309e-13,1.062556e-13,1.257932e-14,1.385296e-15,1.415151e-16,1.336797e-17,1.163482e-18,9.291277e-20,6.775049e-21,4.485495e-22,2.678282e-23,1.430726e-24,6.771001e-26,2.804494e-27,1.000974e-28,3.016481e-30,7.463613e-32,1.455901e-33,2.099473e-35,1.989846e-37,9.298344e-40,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999836e-01,9.987836e-01,9.770964e-01,8.636182e-01,6.278745e-01,3.720873e-01,1.881782e-01,8.431455e-02,3.458727e-02,1.306182e-02,4.589091e-03,1.563636e-03,4.781818e-04,1.800000e-04,4.727273e-05,4.334026e-05,1.048307e-05,2.399161e-06,5.191813e-07,1.061555e-07,2.049087e-08,3.730404e-09,6.398217e-10,1.032622e-10,1.566068e-11,2.228439e-12,2.970059e-13,3.700530e-14,4.300826e-15,4.651158e-16,4.667474e-17,4.332488e-18,3.706400e-19,2.910132e-20,2.086960e-21,1.359229e-22,7.986046e-24,4.198890e-25,1.956318e-26,7.979072e-28,2.804993e-29,8.327564e-31,2.030341e-32,3.903424e-34,5.548905e-36,5.185446e-38,2.389606e-40,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.996455e-01,9.902618e-01,9.175218e-01,7.185400e-01,4.565764e-01,2.443255e-01,1.140218e-01,4.873636e-02,1.892909e-02,6.830909e-03,2.400000e-03,7.636364e-04,2.490909e-04,8.727273e-05,2.545455e-05,1.921314e-05,4.542068e-06,1.016463e-06,2.151885e-07,4.306303e-08,8.139008e-09,1.451430e-09,2.439508e-10,3.859718e-11,5.740601e-12,8.013766e-13,1.048193e-13,1.282112e-14,1.463325e-15,1.554582e-16,1.532956e-17,1.398648e-18,1.176441e-19,9.084415e-21,6.408863e-22,4.107283e-23,2.375189e-24,1.229450e-25,5.640645e-27,2.265971e-28,7.847718e-30,2.295796e-31,5.516701e-33,1.045540e-34,1.465456e-36,1.350538e-38,6.138808e-41,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999073e-01,9.963455e-01,9.545073e-01,8.003109e-01,5.471964e-01,3.100909e-01,1.521491e-01,6.699455e-02,2.708909e-02,1.015273e-02,3.541818e-03,1.196364e-03,3.690909e-04,1.345455e-04,4.181818e-05,3.457071e-05,8.433222e-06,1.949504e-06,4.268128e-07,8.843700e-08,1.732904e-08,3.208309e-09,5.606722e-10,9.238271e-11,1.433445e-11,2.091587e-12,2.865489e-13,3.679532e-14,4.419851e-15,4.955516e-16,5.173205e-17,5.014198e-18,4.498100e-19,3.720998e-20,2.826643e-21,1.962240e-22,1.237747e-23,7.046719e-25,3.591817e-26,1.623103e-27,6.423662e-29,2.192183e-30,6.320670e-32,1.497251e-33,2.797871e-35,3.867374e-37,3.515508e-39,1.576461e-41,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999764e-01,9.987818e-01,9.776255e-01,8.685945e-01,6.387436e-01,3.855745e-01,1.986418e-01,9.116727e-02,3.802909e-02,1.482545e-02,5.290909e-03,1.870909e-03,5.854545e-04,1.963636e-04,5.454545e-05,1.818182e-05,1.536939e-05,3.666280e-06,8.291621e-07,1.776759e-07,3.604843e-08,6.919383e-09,1.255401e-09,2.150778e-10,3.475510e-11,5.290613e-12,7.576144e-13,1.018976e-13,1.284964e-14,1.516266e-15,1.670542e-16,1.714179e-17,1.633614e-18,1.441279e-19,1.172911e-20,8.767493e-22,5.990517e-23,3.720124e-24,2.085589e-25,1.047060e-26,4.661394e-28,1.817853e-29,6.114352e-31,1.737895e-32,4.059084e-34,7.480285e-36,1.019872e-37,9.146107e-40,4.046950e-42,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999945e-01,9.996673e-01,9.903782e-01,9.195927e-01,7.270855e-01,4.693382e-01,2.551018e-01,1.216400e-01,5.298545e-02,2.116545e-02,7.898182e-03,2.785455e-03,9.236364e-04,3.054545e-04,1.000000e-04,3.454545e-05,2.751660e-05,6.766674e-06,1.579193e-06,3.495682e-07,7.334807e-08,1.457783e-08,2.742159e-09,4.877450e-10,8.195054e-11,1.299200e-11,1.940951e-12,2.728683e-13,3.604170e-14,4.464826e-15,5.177169e-16,5.606671e-17,5.656630e-18,5.301809e-19,4.601626e-20,3.684930e-21,2.711123e-22,1.823700e-23,1.115229e-24,6.158181e-26,3.045854e-27,1.336168e-28,5.135738e-30,1.702873e-31,4.772319e-33,1.099239e-34,1.998124e-36,2.687631e-38,2.378250e-40,1.038537e-42,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999236e-01,9.962600e-01,9.555727e-01,8.066455e-01,5.586891e-01,3.216600e-01,1.601673e-01,7.201636e-02,2.981091e-02,1.142364e-02,4.109091e-03,1.452727e-03,4.763636e-04,1.436364e-04,6.545455e-05,4.843240e-05,1.226667e-05,2.951251e-06,6.741495e-07,1.461259e-07,3.003560e-08,5.850119e-09,1.078834e-09,1.881937e-10,3.102197e-11,4.826681e-12,7.079245e-13,9.773839e-14,1.268214e-14,1.543826e-15,1.759622e-16,1.873649e-17,1.859158e-18,1.714248e-19,1.464076e-20,1.153966e-21,8.358531e-23,5.536729e-24,3.334898e-25,1.814204e-26,8.842025e-28,3.822997e-29,1.448554e-30,4.735759e-32,1.308868e-33,2.973718e-35,5.332743e-37,7.077763e-39,6.180987e-41,2.664219e-43,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999800e-01,9.987127e-01,9.779600e-01,8.725400e-01,6.480145e-01,3.979891e-01,2.073600e-01,9.732182e-02,4.155818e-02,1.637636e-02,5.943636e-03,2.210909e-03,7.527273e-04,2.290909e-04,8.727273e-05,2.727273e-05,2.185797e-05,5.416361e-06,1.275512e-06,2.853091e-07,6.058219e-08,1.220348e-08,2.330276e-09,4.214559e-10,7.212941e-11,1.166905e-11,1.782453e-12,2.567436e-13,3.482220e-14,4.440102e-15,5.312934e-16,5.954074e-17,6.235350e-18,6.086700e-19,5.522604e-20,4.642451e-21,3.602428e-22,2.569527e-23,1.676473e-24,9.948157e-26,5.332816e-27,2.561673e-28,1.091858e-29,4.079185e-31,1.315190e-32,3.585395e-34,8.036417e-36,1.422042e-37,1.862656e-39,1.605620e-41,6.832425e-44,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999945e-01,9.996473e-01,9.902782e-01,9.217127e-01,7.343291e-01,4.810400e-01,2.647127e-01,1.285164e-01,5.689636e-02,2.329636e-02,8.629091e-03,3.203636e-03,1.160000e-03,3.763636e-04,1.272727e-04,4.909091e-05,3.831186e-05,9.769472e-06,2.369594e-06,5.464353e-07,1.197387e-07,2.491720e-08,4.920829e-09,9.215558e-10,1.635236e-10,2.746651e-11,4.362484e-12,6.544295e-13,9.260317e-14,1.234225e-14,1.546927e-15,1.820005e-16,2.006006e-17,2.066684e-18,1.985190e-19,1.772880e-20,1.467247e-21,1.121176e-22,7.876863e-24,5.063081e-25,2.960552e-26,1.564190e-27,7.407096e-29,3.112920e-30,1.146929e-31,3.647497e-33,9.809933e-35,2.169660e-36,3.788936e-38,4.898764e-40,4.168863e-42,1.751623e-44,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999109e-01,9.962982e-01,9.565618e-01,8.112491e-01,5.686655e-01,3.323945e-01,1.681582e-01,7.708909e-02,3.264364e-02,1.276000e-02,4.596364e-03,1.734545e-03,5.927273e-04,1.800000e-04,7.818182e-05,2.000000e-05,1.733034e-05,4.325712e-06,1.027437e-06,2.321085e-07,4.984573e-08,1.016947e-08,1.969709e-09,3.619126e-10,6.302740e-11,1.039354e-11,1.621226e-12,2.389227e-13,3.322273e-14,4.352566e-15,5.363947e-16,6.206826e-17,6.730181e-18,6.823033e-19,6.450933e-20,5.671815e-21,4.622428e-22,3.479077e-23,2.408038e-24,1.525239e-25,8.790225e-27,4.578355e-28,2.137700e-29,8.859910e-31,3.219899e-32,1.010239e-33,2.680997e-35,5.851920e-37,1.008728e-38,1.287553e-40,1.081902e-42,4.489218e-45,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999709e-01,9.987473e-01,9.785800e-01,8.746836e-01,6.568109e-01,4.079582e-01,2.168473e-01,1.029455e-01,4.495818e-02,1.837091e-02,6.714545e-03,2.478182e-03,9.290909e-04,2.872727e-04,1.018182e-04,2.909091e-05,1.272727e-05,7.765106e-06,1.898017e-06,4.416469e-07,9.778181e-08,2.058761e-08,4.119512e-09,7.828388e-10,1.411709e-10,2.413712e-11,3.909066e-12,5.990197e-13,8.675069e-14,1.185755e-14,1.527466e-15,1.851379e-16,2.107561e-17,2.248787e-18,2.243976e-19,2.088754e-20,1.808476e-21,1.451730e-22,1.076467e-23,7.342005e-25,4.583476e-26,2.604059e-27,1.337335e-28,6.158023e-30,2.517498e-31,9.026236e-33,2.794413e-34,7.318804e-36,1.576860e-37,2.683437e-39,3.382009e-41,2.806456e-43,1.150187e-45,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.996455e-01,9.905182e-01,9.231127e-01,7.410545e-01,4.910727e-01,2.746691e-01,1.353927e-01,6.124000e-02,2.562727e-02,9.823636e-03,3.667273e-03,1.350909e-03,4.418182e-04,1.581818e-04,6.000000e-05,5.201647e-05,1.371625e-05,3.447358e-06,8.255113e-07,1.882571e-07,4.086492e-08,8.438652e-09,1.656687e-09,3.089885e-10,5.470584e-11,9.186099e-12,1.461537e-12,2.200894e-13,3.133126e-14,4.210842e-15,5.334977e-16,6.361483e-17,7.126172e-18,7.484204e-19,7.352608e-20,6.739665e-21,5.747651e-22,4.545547e-23,3.321369e-24,2.232739e-25,1.374084e-26,7.697514e-28,3.898565e-29,1.770729e-30,7.141767e-32,2.526657e-33,7.719843e-35,1.995767e-36,4.245083e-38,7.133088e-40,8.878151e-42,7.276690e-44,2.946028e-46,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999145e-01,9.963582e-01,9.573200e-01,8.156473e-01,5.770345e-01,3.419727e-01,1.758200e-01,8.197091e-02,3.548182e-02,1.419818e-02,5.261818e-03,1.983636e-03,6.890909e-04,2.363636e-04,9.454545e-05,2.909091e-05,2.385593e-05,6.160385e-06,1.516874e-06,3.559963e-07,7.959684e-08,1.694630e-08,3.433440e-09,6.615710e-10,1.211437e-10,2.106459e-11,3.474921e-12,5.433102e-13,8.042425e-14,1.125738e-14,1.488053e-15,1.854753e-16,2.176343e-17,2.399653e-18,2.481222e-19,2.400440e-20,2.167285e-21,1.820926e-22,1.419076e-23,1.021986e-24,6.772705e-26,4.109794e-27,2.270509e-28,1.134292e-29,5.082753e-31,2.022816e-32,7.062790e-34,2.130057e-35,5.436467e-37,1.141793e-38,1.894702e-40,2.329241e-42,1.885903e-44,7.543611e-47,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999727e-01,9.987236e-01,9.785109e-01,8.772145e-01,6.635636e-01,4.174327e-01,2.249873e-01,1.084073e-01,4.819091e-02,1.998182e-02,7.518182e-03,2.838182e-03,1.054545e-03,3.472727e-04,1.327273e-04,5.090909e-05,4.087779e-05,1.083778e-05,2.741887e-06,6.616934e-07,1.522582e-07,3.338996e-08,6.974803e-09,1.386981e-09,2.623881e-10,4.718815e-11,8.060890e-12,1.306780e-12,2.008437e-13,2.923289e-14,4.024523e-15,5.233617e-16,6.419295e-17,7.414018e-18,8.048306e-19,8.195076e-20,7.809230e-21,6.946390e-22,5.751147e-23,4.417509e-24,3.136278e-25,2.049347e-26,1.226424e-27,6.683321e-29,3.293987e-30,1.456468e-31,5.720560e-33,1.971569e-34,5.870187e-36,1.479355e-37,3.068356e-39,5.029082e-41,6.107403e-43,4.885613e-45,1.931073e-47,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.996273e-01,9.903636e-01,9.249673e-01,7.449036e-01,4.995436e-01,2.830582e-01,1.419964e-01,6.502545e-02,2.749273e-02,1.094182e-02,4.189091e-03,1.532727e-03,5.381818e-04,2.000000e-04,7.454545e-05,2.363636e-05,1.878203e-05,4.878657e-06,1.209711e-06,2.862349e-07,6.460062e-08,1.389997e-08,2.849826e-09,5.564007e-10,1.033783e-10,1.826500e-11,3.066200e-12,4.886250e-13,7.384287e-14,1.057102e-14,1.431755e-15,1.832213e-16,2.212018e-17,2.515282e-18,2.688876e-19,2.696813e-20,2.531824e-21,2.219244e-22,1.810971e-23,1.371303e-24,9.599656e-26,6.186215e-27,3.651740e-28,1.963271e-29,9.548075e-31,4.166544e-32,1.615356e-33,5.496263e-35,1.615858e-36,4.021493e-38,8.238551e-40,1.333917e-41,1.600497e-43,1.265137e-45,4.941942e-48,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999145e-01,9.961091e-01,9.578036e-01,8.183455e-01,5.846345e-01,3.503655e-01,1.828855e-01,8.655091e-02,3.789273e-02,1.547455e-02,5.954545e-03,2.240000e-03,7.836364e-04,2.745455e-04,1.054545e-04,3.454545e-05,3.208182e-05,8.549921e-06,2.176699e-06,5.291995e-07,1.228163e-07,2.719675e-08,5.743641e-09,1.156183e-09,2.217012e-10,4.046828e-11,7.026501e-12,1.159521e-12,1.816912e-13,2.700625e-14,3.803506e-15,5.069416e-16,6.385496e-17,7.589987e-18,8.499122e-19,8.949349e-20,8.843020e-21,8.180981e-22,7.067884e-23,5.685863e-24,4.245264e-25,2.930869e-26,1.863016e-27,1.084979e-28,5.755841e-30,2.762654e-31,1.189987e-32,4.554713e-34,1.530231e-35,4.442803e-37,1.092124e-38,2.210199e-40,3.535642e-42,4.191941e-44,3.274757e-46,1.264385e-48,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999745e-01,9.987091e-01,9.785836e-01,8.782709e-01,6.693455e-01,4.258327e-01,2.328127e-01,1.138782e-01,5.116545e-02,2.156000e-02,8.500000e-03,3.220000e-03,1.178182e-03,4.272727e-04,1.490909e-04,5.454545e-05,2.000000e-05,1.476659e-05,3.857169e-06,9.628326e-07,2.296007e-07,5.228315e-08,1.136372e-08,2.356296e-09,4.658490e-10,8.775971e-11,1.574270e-11,2.686987e-12,4.360015e-13,6.719599e-14,9.826247e-15,1.361858e-15,1.786641e-16,2.215694e-17,2.593540e-18,2.860626e-19,2.967627e-20,2.889629e-21,2.634880e-22,2.244124e-23,1.780087e-24,1.310752e-25,8.926149e-27,5.597773e-28,3.216830e-29,1.684220e-30,7.979445e-32,3.393251e-33,1.282426e-34,4.254941e-36,1.220184e-37,2.963035e-39,5.924554e-41,9.365135e-43,1.097344e-44,8.473199e-47,3.234045e-49,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999945e-01,9.996291e-01,9.903109e-01,9.251400e-01,7.494745e-01,5.065945e-01,2.911964e-01,1.479655e-01,6.874727e-02,2.957091e-02,1.206727e-02,4.640000e-03,1.761818e-03,6.236364e-04,2.127273e-04,8.363636e-05,3.272727e-05,2.514700e-05,6.735024e-06,1.724962e-06,4.223463e-07,9.882074e-08,2.208713e-08,4.713477e-09,9.599105e-10,1.864484e-10,3.451829e-11,6.086932e-12,1.021576e-12,1.630406e-13,2.472109e-14,3.557449e-15,4.853071e-16,6.268445e-17,7.655455e-18,8.826555e-19,9.591617e-20,9.805396e-21,9.410516e-22,8.459321e-23,7.104121e-24,5.557467e-25,4.036540e-26,2.711971e-27,1.678206e-28,9.517924e-30,4.918924e-31,2.300767e-32,9.660825e-34,3.605759e-35,1.181654e-36,3.347490e-38,8.031396e-40,1.586836e-41,2.478979e-43,2.871065e-45,2.191526e-47,8.269910e-50,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999018e-01,9.961400e-01,9.579236e-01,8.207636e-01,5.906891e-01,3.584673e-01,1.897818e-01,9.075818e-02,4.045636e-02,1.669636e-02,6.596364e-03,2.523636e-03,9.290909e-04,3.272727e-04,1.145455e-04,4.727273e-05,4.224722e-05,1.159424e-05,3.044763e-06,7.649037e-07,1.837631e-07,4.220317e-08,9.261558e-09,1.941200e-09,3.883970e-10,7.413919e-11,1.349295e-11,2.339617e-12,3.862091e-13,6.064107e-14,9.048317e-15,1.281666e-15,1.721443e-16,2.189659e-17,2.634065e-18,2.992133e-19,3.204116e-20,3.228483e-21,3.054581e-22,2.707471e-23,2.242394e-24,1.730346e-25,1.239934e-26,8.220240e-28,5.020302e-29,2.810509e-30,1.433978e-31,6.622837e-33,2.746329e-34,1.012437e-35,3.277628e-37,9.173809e-39,2.174925e-40,4.246864e-42,6.557695e-44,7.507942e-46,5.666052e-48,2.114199e-50,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999727e-01,9.985945e-01,9.783782e-01,8.799582e-01,6.740891e-01,4.329364e-01,2.400436e-01,1.191818e-01,5.422545e-02,2.318727e-02,9.343636e-03,3.709091e-03,1.358182e-03,4.909091e-04,1.563636e-04,6.000000e-05,2.545455e-05,1.968780e-05,5.297929e-06,1.364697e-06,3.364010e-07,7.932701e-08,1.788794e-08,3.855562e-09,7.939514e-10,1.561159e-10,2.929481e-11,5.242538e-12,8.941039e-13,1.452069e-13,2.243691e-14,3.295356e-15,4.595696e-16,6.078716e-17,7.616165e-18,9.026558e-19,1.010427e-19,1.066475e-20,1.059369e-21,9.883072e-23,8.639305e-24,7.058014e-25,5.373259e-26,3.799397e-27,2.485918e-28,1.498619e-29,8.282795e-31,4.172864e-32,1.903284e-33,7.795539e-35,2.838967e-36,9.080582e-38,2.511465e-39,5.884445e-41,1.135722e-42,1.733626e-44,1.962373e-46,1.464377e-48,5.403605e-51,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999964e-01,9.995255e-01,9.901091e-01,9.248764e-01,7.525418e-01,5.127691e-01,2.985800e-01,1.536127e-01,7.255091e-02,3.156545e-02,1.297273e-02,5.234545e-03,1.960000e-03,7.527273e-04,2.472727e-04,8.727273e-05,4.000000e-05,3.299227e-05,9.091975e-06,2.399893e-06,6.065901e-07,1.467688e-07,3.398244e-08,7.526385e-09,1.593814e-09,3.225486e-10,6.234811e-11,1.150434e-11,2.024995e-12,3.397775e-13,5.430351e-14,8.259320e-15,1.194339e-15,1.640293e-16,2.137098e-17,2.638071e-18,3.081076e-19,3.399423e-20,3.537193e-21,3.464561e-22,3.187636e-23,2.748605e-24,2.215397e-25,1.664252e-26,1.161404e-27,7.500937e-29,4.464280e-30,2.436340e-31,1.212171e-32,5.460947e-34,2.209581e-35,7.950357e-37,2.512835e-38,6.868500e-40,1.590683e-41,3.034946e-43,4.580272e-45,5.126603e-47,3.783267e-49,1.380754e-51,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.998873e-01,9.959782e-01,9.570600e-01,8.220836e-01,5.950418e-01,3.650345e-01,1.957327e-01,9.528000e-02,4.273818e-02,1.803818e-02,7.307273e-03,2.823636e-03,1.096364e-03,3.727273e-04,1.236364e-04,5.272727e-05,1.454545e-05,1.090909e-05,4.161937e-06,1.077973e-06,2.674447e-07,6.353810e-08,1.444947e-08,3.144210e-09,6.543615e-10,1.301828e-10,2.474457e-11,4.490891e-12,7.777170e-13,1.284188e-13,2.020249e-14,3.025295e-15,4.308219e-16,5.828219e-17,7.481321e-18,9.100644e-19,1.047634e-19,1.139519e-20,1.169148e-21,1.129370e-22,1.024978e-23,8.719570e-25,6.935020e-26,5.141663e-27,3.541839e-28,2.258361e-29,1.327184e-30,7.152978e-32,3.515187e-33,1.564418e-34,6.254000e-36,2.223614e-37,6.945795e-39,1.876566e-40,4.296222e-42,8.104234e-44,1.209386e-45,1.338659e-47,9.770718e-50,3.527335e-52,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999709e-01,9.984582e-01,9.776200e-01,8.802127e-01,6.768945e-01,4.390200e-01,2.459364e-01,1.235055e-01,5.715273e-02,2.466727e-02,1.020364e-02,4.041818e-03,1.576364e-03,5.490909e-04,1.854545e-04,6.909091e-05,2.181818e-05,1.090909e-05,7.121256e-06,1.888941e-06,4.802404e-07,1.169907e-07,2.729938e-08,6.099614e-09,1.304430e-09,2.668755e-10,5.220905e-11,9.760883e-12,1.742896e-12,2.970295e-13,4.827832e-14,7.477851e-15,1.102779e-15,1.546909e-16,2.061788e-17,2.608074e-18,3.127066e-19,3.548830e-20,3.806212e-21,3.851407e-22,3.669824e-23,3.285956e-24,2.758396e-25,2.165203e-26,1.584589e-27,1.077643e-28,6.784882e-30,3.937769e-31,2.096247e-32,1.017665e-33,4.474797e-35,1.767681e-36,6.211426e-38,1.917778e-39,5.122030e-41,1.159374e-42,2.162529e-44,3.191405e-46,3.493870e-48,2.522520e-50,9.008998e-53,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999873e-01,9.994909e-01,9.895655e-01,9.249655e-01,7.539382e-01,5.178655e-01,3.043455e-01,1.588255e-01,7.545818e-02,3.347455e-02,1.406909e-02,5.674545e-03,2.172727e-03,7.963636e-04,2.690909e-04,1.036364e-04,4.181818e-05,1.272727e-05,1.202746e-05,3.265403e-06,8.502207e-07,2.122480e-07,5.078575e-08,1.164339e-08,2.556769e-09,5.375208e-10,1.081403e-10,2.080870e-11,3.827549e-12,6.725812e-13,1.128290e-13,1.805610e-14,2.754225e-15,4.000915e-16,5.529409e-17,7.262619e-18,9.055116e-19,1.070344e-19,1.197757e-20,1.266940e-21,1.264571e-22,1.188798e-23,1.050365e-24,8.702131e-26,6.742652e-27,4.871728e-28,3.271486e-29,2.034152e-30,1.166081e-31,6.132288e-33,2.941371e-34,1.278040e-35,4.989550e-37,1.732982e-38,5.289366e-40,1.396710e-41,3.126090e-43,5.766438e-45,8.416817e-47,9.114743e-49,6.510202e-51,2.300425e-53,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999945e-01,9.998709e-01,9.957073e-01,9.568036e-01,8.226800e-01,5.988291e-01,3.702836e-01,2.010164e-01,9.868545e-02,4.486545e-02,1.930364e-02,7.840000e-03,3.081818e-03,1.209091e-03,4.290909e-04,1.490909e-04,6.363636e-05,1.818182e-05,7.272727e-06,5.571383e-06,1.484788e-06,3.796085e-07,9.308038e-08,2.188249e-08,4.930610e-09,1.064392e-09,2.200458e-10,4.354404e-11,8.243685e-12,1.492250e-12,2.581165e-13,4.263287e-14,6.718942e-15,1.009548e-15,1.444879e-16,1.967832e-17,2.547587e-18,3.131435e-19,3.649819e-20,4.028078e-21,4.202883e-22,4.138802e-23,3.839339e-24,3.347959e-25,2.737976e-26,2.094450e-27,1.494262e-28,9.909705e-30,6.086073e-31,3.446553e-32,1.790789e-33,8.487866e-35,3.644855e-36,1.406511e-37,4.829242e-39,1.457297e-40,3.805092e-42,8.422257e-44,1.536581e-45,2.218544e-47,2.376770e-49,1.679613e-51,5.872774e-54,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.999764e-01,9.984782e-01,9.773127e-01,8.798055e-01,6.793218e-01,4.432273e-01,2.516127e-01,1.279745e-01,5.969636e-02,2.622909e-02,1.099818e-02,4.398182e-03,1.745455e-03,6.236364e-04,2.218182e-04,8.363636e-05,2.545455e-05,7.272727e-06,1.818182e-06,1.818182e-06,6.696392e-07,1.681618e-07,4.051297e-08,9.360587e-09,2.073478e-09,4.401612e-10,8.950545e-11,1.742623e-11,3.246706e-12,5.785165e-13,9.852473e-14,1.602616e-14,2.487929e-15,3.683077e-16,5.194626e-17,6.973308e-18,8.900105e-19,1.078722e-19,1.239996e-20,1.349926e-21,1.389636e-22,1.350350e-23,1.236291e-24,1.064166e-25,8.592018e-27,6.489944e-28,4.572678e-29,2.995323e-30,1.817286e-31,1.016804e-32,5.220649e-34,2.445493e-35,1.037994e-36,3.959693e-38,1.344180e-39,4.010897e-41,1.035684e-42,2.267316e-44,4.091775e-46,5.844501e-48,6.194970e-50,4.331937e-52,1.498940e-54,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.999873e-01,9.994655e-01,9.893491e-01,9.243073e-01,7.551709e-01,5.215800e-01,3.095800e-01,1.632655e-01,7.875273e-02,3.541091e-02,1.515455e-02,6.174545e-03,2.432727e-03,9.418182e-04,3.345455e-04,1.290909e-04,4.545455e-05,1.272727e-05,3.636364e-06,3.636364e-06,1.165628e-06,2.996145e-07,7.392618e-08,1.750409e-08,3.975984e-09,8.660753e-10,1.808412e-10,3.618065e-11,6.932340e-12,1.271378e-12,2.230515e-13,3.741038e-14,5.994193e-15,9.168284e-16,1.337523e-16,1.859408e-17,2.460799e-18,3.096948e-19,3.701967e-20,4.197663e-21,4.508584e-22,4.579841e-23,4.392273e-24,3.969448e-25,3.373309e-26,2.689355e-27,2.006170e-28,1.396167e-29,9.034712e-31,5.415782e-32,2.994357e-33,1.519425e-34,7.035069e-36,2.951896e-37,1.113342e-38,3.737146e-40,1.102792e-41,2.816444e-43,6.099007e-45,1.088886e-46,1.538832e-48,1.614003e-50,1.116904e-52,3.825014e-55,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.999982e-01,9.998291e-01,9.954018e-01,9.559018e-01,8.228018e-01,6.024091e-01,3.748891e-01,2.062673e-01,1.021473e-01,4.720727e-02,2.056909e-02,8.587273e-03,3.423636e-03,1.374545e-03,4.818182e-04,1.836364e-04,7.090909e-05,2.363636e-05,7.272727e-06,1.818182e-06,1.818182e-06,5.267047e-07,1.330220e-07,3.225799e-08,7.508939e-09,1.677266e-09,3.593731e-10,7.382976e-11,1.453660e-11,2.741727e-12,4.950843e-13,8.554002e-14,1.413230e-14,2.231013e-15,3.362806e-16,4.835559e-17,6.627328e-18,8.648544e-19,1.073461e-19,1.265758e-20,1.416022e-21,1.500800e-22,1.504624e-23,1.424407e-24,1.270905e-25,1.066465e-26,8.396791e-28,6.186907e-29,4.253515e-30,2.719524e-31,1.610901e-32,8.802389e-34,4.414932e-35,2.020777e-36,8.383250e-38,3.126483e-39,1.037857e-40,3.029097e-42,7.652349e-44,1.639369e-45,2.895834e-47,4.049526e-49,4.203260e-51,2.878812e-53,9.758684e-56,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.999982e-01,9.999509e-01,9.982255e-01,9.766164e-01,8.793091e-01,6.818582e-01,4.474327e-01,2.563855e-01,1.315418e-01,6.242545e-02,2.772000e-02,1.168909e-02,4.869091e-03,1.847273e-03,7.490909e-04,2.781818e-04,1.109091e-04,3.454545e-05,9.090909e-06,3.636364e-06,3.399340e-06,9.139701e-07,2.361419e-07,5.861544e-08,1.397429e-08,3.198841e-09,7.028326e-10,1.481641e-10,2.995602e-11,5.805985e-12,1.078201e-12,1.917417e-13,3.263364e-14,5.312044e-15,8.264092e-16,1.227802e-16,1.740577e-17,2.352283e-18,3.027479e-19,3.706736e-20,4.312225e-21,4.760386e-22,4.979561e-23,4.927930e-24,4.605854e-25,4.057863e-26,3.362842e-27,2.615259e-28,1.903623e-29,1.293075e-30,8.169561e-32,4.782606e-33,2.583121e-34,1.280779e-35,5.796027e-37,2.377613e-38,8.769106e-40,2.879119e-41,8.312080e-43,2.077380e-44,4.403236e-46,7.696459e-48,1.065101e-49,1.094180e-51,7.417834e-54,2.489206e-56,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,1.000000e+00,9.999982e-01,9.999873e-01,9.993691e-01,9.888091e-01,9.235400e-01,7.562327e-01,5.248073e-01,3.143236e-01,1.679527e-01,8.159091e-02,3.699455e-02,1.610727e-02,6.689091e-03,2.636364e-03,1.078182e-03,4.018182e-04,1.472727e-04,6.000000e-05,9.090909e-06,3.636364e-06,1.818182e-06,1.566172e-06,4.137515e-07,1.050673e-07,2.563978e-08,6.011147e-09,1.353504e-09,2.925965e-10,6.070414e-11,1.208155e-11,2.305576e-12,4.216651e-13,7.386612e-14,1.238651e-14,1.986969e-15,3.046910e-16,4.462873e-17,6.238579e-18,8.315159e-19,1.055676e-19,1.275228e-20,1.463930e-21,1.594991e-22,1.646936e-23,1.609128e-24,1.485065e-25,1.292139e-26,1.057696e-27,8.125980e-29,5.844013e-30,3.922698e-31,2.449347e-32,1.417312e-33,7.567497e-35,3.709746e-36,1.660037e-37,6.734411e-39,2.456615e-40,7.978409e-42,2.278723e-43,5.634718e-45,1.181819e-46,2.044278e-48,2.799988e-50,2.847185e-52,1.910777e-54,6.348098e-57};
    size_t weightMatrixIndex = 0;
    int length = 0;
    
    struct prob_dist *pdist = calloc(1, sizeof(struct prob_dist));
    pdist->smatrix = smatrix;
    
    if(pdist==NULL) error("read_diag_prob_dist(): Out of memory !");
    
    length = 100;
    
    pdist->max_dlen = length;
    pdist->data = calloc(length+1, sizeof(long double *));
    long double **dist =pdist->data;
    if(dist==NULL) error("read_diag_prob_dist(): (1) Out of memory when allocating data !");
    
    pdist->log_data = calloc(length+1, sizeof(double *));
    double **log_dist =pdist->log_data;
    if(log_dist==NULL) error("read_diag_prob_dist(): (1.1) Out of memory when allocating data !");
    
    
    // read the entries
    unsigned long i, scr, mxscr, sm_max_scr=smatrix->max_score;
    unsigned long ti, tscr;
    long double weight;
    long size=0;
    
    for( i=1; i<=length; i++) {
        mxscr = i*sm_max_scr;
        size += mxscr+1;
        dist[i] = calloc(mxscr+1, sizeof(long double ));
        log_dist[i] = calloc(mxscr+1, sizeof(long double ));
        if(dist[i]==NULL) error("read_diag_prob_dist(): (3) Out of memory at iteration" );
        for(scr=0;scr<=mxscr;scr++) {
            dist[i][scr]=1.0;
        }
        for(scr=0;scr<=mxscr;scr++) {
            dist[i][scr]=1.0;
            
            ti = i;
            tscr = scr;
            weight = weightMatrix[weightMatrixIndex++];
//            fscanf( fp, "%li %li %Le\n", &ti,&tscr,&weight );
            
            //if(i!=ti || tscr!=scr) merror("read_scr_matrix(): (4) Invalid format of file ",filename);
            scr = tscr;
            if(weight==0.0) weight = 1.0;
            dist[i][scr]=weight;
            log_dist[i][scr]=-log(weight);
            if(para->DEBUG>5)printf("%li %li %Le\n", i, scr,dist[i][scr] );
        }
    }
    assert(weightMatrixIndex == 5150);
    
    return pdist;
}


/**
 * reads sequence collection (seq_col) from the file
 * indicated by filename parameter.
 *
 * The pointer returned (and the ones included in the struct)
 * has to be deallocted explicitely from memory.
 */
struct seq_col* read_fasta(char *filename)
{
    
    if(para->DEBUG)
        printf("DEBUG read_seq_col(): Processing input file: %s\n", filename);
    
    struct seq_col *scol = calloc(1, sizeof(struct seq_col));
    struct seq* seqs = (scol->seqs = calloc(para->MAX_SEQ_AMOUNT, sizeof(struct seq)));
    struct seq* seq;
    
    if(scol==NULL || seqs==NULL) error("read_fasta(): Out of memory !");
    
    FILE *fp;
    
    char rline[para->MAX_FASTA_LINE_LENGTH];
    
    if( (fp = fopen( filename , "r")) == NULL) {
        merror("read_fasta(): Cannot open input FASTA file", filename );
    }
    
    scol->length = 0;
    char *data; char ch;
    unsigned int *slen;
    int data_maxlen;
    int c, rlen;
    int valid =0;
    char long_name=0;
    int name_length;
    
    while( fgets( rline , para->MAX_FASTA_LINE_LENGTH+1, fp ) != NULL ) {
        //    if(para->DEBUG)
        //      printf(rline);
        
        strip_leading_ws(rline);
        
        rlen = (int)strlen(rline);
        
        if(rline[0] == '>') {
            valid = 1;
            long_name=1;
            
            seq = &(scol->seqs[scol->length]);
            seq->max_seen = 0;
            seq->name = calloc(rlen, sizeof(char));
            seq->num = scol->length;
            seq->orf_frame=0;
            seq->crick_strand=0;
            strncpy(seq->name, &(rline[1]), rlen-2);
            slen = (unsigned int *) &(seq->length);
            
            *slen = 0;
            
            data_maxlen = 1024;
            seq->data = (data = calloc(data_maxlen, sizeof(char)));
            if(data==NULL) error("read_fasta(): Out of memory: seq->data alloc");
            
            scol->length++;
            if(rlen < para->MAX_FASTA_LINE_LENGTH)
            {
                long_name = 0; // if relen =  then line is longer
            }
            
        }
        else if(long_name)
        {
            long_name=1;
            if(rlen < para->MAX_FASTA_LINE_LENGTH)
            {
                long_name = 0; // if relen =  then line is longer
                rline[rlen-1]='\0';
            }
            
            
            name_length = (int)strlen(seq->name);
            if(NULL == (seq->name = realloc(seq->name,rlen + name_length) ) )
            {
                error("read_fasta(): Out of memory: seq->data realloc");
            }
            
            strcat(seq->name, rline);
            
        }
        else {
            for( c=0;c<rlen;c++) {
                ch = rline[c];
                if(ch=='?' || ch=='$' || ch=='#') ch='X';
                if( (ch >= 65 && ch < 91) || (ch >= 97 && ch < 123)) {
                    if(! valid) merror("read_fasta(): File is not in FASTA format:", filename);
                    
                    // realloc memory when necessary
                    if(*slen >= data_maxlen)
                    {
                        data_maxlen += 1024;
                        seq->data = (data = realloc(data, data_maxlen*sizeof(char)));
                        if(data==NULL) error("read_fasta(): Out of memory: seq->data alloc");
                    }
                    data[(*slen)++] = ((ch >= 97) ? toupper(ch) : ch);
                }
            }
        }
    }
	
    int avg=0;
    for(c=0;c<scol->length;c++) {
        avg += scol->seqs[c].length;
    }
    scol->avg_length = avg/scol->length;
    
    if(para->DEBUG)
        printf("\n");
    
    fclose(fp);
    return scol;
}

// We create a seq_col structure from two strings rather than from a FASTA file.
// The function closely follows read_fasta of Dialign.
struct seq_col* read_fasta2(const char *s1, const char *s2)
{
    if(para->DEBUG)
        printf("DEBUG read_seq_col(): Processing two sequences\n"
               "[Seq1] %s\n"
               "[Seq2] %s\n",
               s1, s2);
    
    struct seq_col *scol = calloc(1, sizeof(struct seq_col));
    struct seq* seqs = (scol->seqs = calloc(para->MAX_SEQ_AMOUNT, sizeof(struct seq)));
//    struct seq* seqs = (scol->seqs = calloc(3, sizeof(struct seq)));
    struct seq* seq;
    
    if(scol==NULL || seqs==NULL) error("read_fasta(): Out of memory !");
    
    scol->length = 2;
    for (size_t i = 0; i < 2; i++) {
        seq = &(scol->seqs[i]);
        seq->max_seen = 0;
        seq->name = strdup("name");
        seq->num = (int)i;
        seq->orf_frame=0;
        seq->crick_strand=0;
        if (i == 0)
            seq->data = strdup(s1);
        else
            seq->data = strdup(s2);
        if(seq->data == NULL) error("read_fasta(): Out of memory: seq->data alloc");
        seq->length = (int)strlen(seq->data);
    }
	
    int avg=0;
    for(size_t c=0;c<scol->length;c++) {
        avg += scol->seqs[c].length;
    }
    scol->avg_length = avg/scol->length;
    
    return scol;
}

/**
 * reads the given anchor file and returns the accordingly created
 * simple_diag_col structure pointer
 */
struct simple_diag_col* read_anchors(char *filename, struct seq_col* scol)
{
    
    if(para->DEBUG)
        printf("DEBUG read_anchors(): Processing anchor file: %s\n", filename);
    
    struct simple_diag_col *sdcol = malloc(sizeof(struct simple_diag_col));
    
    FILE *fp;
    
    if( (fp = fopen( filename , "r")) == NULL) {
        merror("read_anchros(): Cannot open input anchor file", filename );
    }
    
    //fscanf( fp, "%li %li %Le\n", &ti,&tscr,&weight );
    
    int s1,s2,sp1,sp2,len;
    double score;
    
    int alloc_size = 64;
    sdcol->data = malloc(sizeof (struct diag*)*alloc_size);
    sdcol->length=0;
    
    while( fscanf(fp,"%i %i %i %i %i %le\n",&s1,&s2,&sp1,&sp2,&len,&score ) == 6) {
        if(sdcol->length >= alloc_size) {
            alloc_size+=16;
            sdcol->data = realloc(sdcol->data,sizeof (struct diag*)*alloc_size);
        }
        sdcol->data[sdcol->length]= create_diag(s1-1,&(scol->seqs[s1-1]),sp1-1,
                                                s2-1,&(scol->seqs[s2-1]),sp2-1,len);
        sdcol->data[sdcol->length]->anchor = 1;
        sdcol->data[sdcol->length]->meetsThreshold = 1;
        //printf(" total weight %e\n", score);
        sdcol->data[sdcol->length++]->total_weight = score;
    }
    
    if(para->DEBUG) printf("DEBUG  read_anchors(): Read %i anchors from file\n", sdcol->length);
    fclose(fp);
    return sdcol;
}

struct simple_diag_col* read_anchors_from_code(struct seq_col* scol, const int *anchor1, const int *anchor2, const int *anchorLength, const double *anchorScore, const size_t numberOfAnchor)
{
    struct simple_diag_col *sdcol = malloc(sizeof(struct simple_diag_col));
    
    int s1,s2,sp1,sp2,len;
    double score;
    
    int alloc_size = 64;
    sdcol->data = malloc(sizeof (struct diag*)*alloc_size);
    sdcol->length=0;
    
    s1 = 1;
    s2 = 2;
    for (size_t i = 0; i < numberOfAnchor; i++) {
        sp1 = anchor1[i];
        sp2 = anchor2[i];
        len = anchorLength[i];
        score = anchorScore[i];
    
//    while( fscanf(fp,"%i %i %i %i %i %le\n",&s1,&s2,&sp1,&sp2,&len,&score ) == 6) {
    
        if(sdcol->length >= alloc_size) {
            alloc_size+=16;
            sdcol->data = realloc(sdcol->data,sizeof (struct diag*)*alloc_size);
        }
        sdcol->data[sdcol->length]= create_diag(s1-1,&(scol->seqs[s1-1]),sp1-1,
                                                s2-1,&(scol->seqs[s2-1]),sp2-1,len);
        sdcol->data[sdcol->length]->anchor = 1;
        sdcol->data[sdcol->length]->meetsThreshold = 1;
        //printf(" total weight %e\n", score);
        sdcol->data[sdcol->length++]->total_weight = score;
        
    }
    
    if(para->DEBUG) printf("DEBUG  read_anchors(): Read %i anchors from file\n", sdcol->length);
    return sdcol;
}


/**
 * prints the given alignment  in a simple way.
 */
void simple_print_alignment_default(struct alignment *algn)
{
    struct seq_col *scol = algn->scol;
    unsigned int slen = scol->length;
    
    unsigned int i,j,s,pos,max,tmax;
    struct seq* sq;
    struct algn_pos **ap = algn->algn;
    int proc[slen];
    //  char proceed[slen];
    
    for(i=0;i<slen;i++) {
        proc[i]=0;
    }
    
    
    prepare_alignment(algn);
    max = algn->max_pos;
    struct algn_pos *ap1;
    
    //
    // print block
    // WARNING!  print_info reallocates memory, this loses the algn pointer
    //   since the new one is not stored when it is returned here, so it cannot
    //   be freed later. In the present version the leak is relatively minor and
    //   should not crash anything.
    //
    char *print_info_string = print_info(algn);
	// printf("%s",print_info(algn)); FIXED.
	printf("%s",print_info_string);
    free(print_info_string);
    
    printf("\n   ALIGNMENT OUTPUT:\n");
    printf("   -----------------\n\n");
    //  printf("%i\n", max);
    for(pos=0;pos<max;pos+=50) {
        tmax = pos+50;
        //printf("tmax: %i\n", tmax);
        
        if(tmax>max) tmax = max;
        for(s=0;s<slen;s++) {
            sq = &scol->seqs[s];
            for(j=pos;j<tmax;j++) {
                
                if( (j%50)==0) {
                    printf("%12.12s%10i   ", sq->name, proc[s]+1);
                } else if( (j%10)==0) {
                    printf(" ");
                }
                if(proc[s]<sq->length) {
                    ap1 = find_eqc(ap,s,proc[s]);
                    if( (*ap1->eqcAlgnPos) < j) {
                        printf ("\nALARM %i %i %i %i\n", s,j, proc[s], *ap1->eqcAlgnPos);
                    }
                    //	  if(proc[0]==244 && s==0) printf("\nMOVE FORWARD: %i %i %i\n",j, *ap[0][244].eqcAlgnPos, *ap[1][241].eqcAlgnPos);
                    
                    if( (*ap1->eqcAlgnPos) == j) {
                        if(ap1->state & para->STATE_ORPHANE) {
                            printf("%c", tolower(sq->data[proc[s]]));
                        } else {
                            printf("%c", toupper(sq->data[proc[s]]));
                        }
                        proc[s]++;
                    } else {
                        printf("-");
                        //printf("%i",*ap[s][proc[s]].maxpos);
                    }
                } else {
                    printf("-");
                    //printf("%i",*ap[s][proc[s]].maxpos);
                    //	  printf("\n%i %i %i\n", s, j,proc[s]);
                }
            }
            printf("\n");
        }
        printf("\n");
        printf("\n");
    }
}

//void simple_print_alignment_dna_retranslate(struct alignment *algn)
//{
//	char *tmp = "000";
//    struct seq_col *scol = algn->scol;
//    unsigned int slen = scol->length;
//    
//    unsigned int i,j,s,pos,max,tmax;
//    struct seq* sq;
//    struct algn_pos **ap = algn->algn;
//    int proc[slen];
//    //  char proceed[slen];
//    
//    for(i=0;i<slen;i++) {
//        proc[i]=0;
//    }
//    
//    prepare_alignment(algn);
//    max = algn->max_pos;
//    struct algn_pos *ap1;
//    
//    //
//    // print block
//    //
//	printf("%s",print_info(algn));
//    printf("\n   ALIGNMENT OUTPUT:\n");
//    printf("   -----------------\n\n");
//    //  printf("%i\n", max);
//    for(pos=0;pos<max;pos+=16) {
//        tmax = pos+16;
//        //printf("tmax: %i\n", tmax);
//        
//        if(tmax>max) tmax = max;
//        for(s=0;s<slen;s++) {
//            sq = &scol->seqs[s];
//            for(j=pos;j<tmax;j++) {
//                
//                if( (j%16)==0) {
//                    printf("%12.12s%10i   ", sq->name, ((proc[s])*3)+1);
//                } else if( (j%4)==0) {
//                    printf(" ");
//                }
//                if(proc[s]<sq->length) {
//                    ap1 = find_eqc(ap,s,proc[s]);
//                    if( (*ap1->eqcAlgnPos) < j) {
//                        printf ("\nALARM %i %i %i %i\n", s,j, proc[s], *ap1->eqcAlgnPos);
//                    }
//                    //	  if(proc[0]==244 && s==0) printf("\nMOVE FORWARD: %i %i %i\n",j, *ap[0][244].eqcAlgnPos, *ap[1][241].eqcAlgnPos);
//                    
//                    if( (*ap1->eqcAlgnPos) == j) {
//                        
//                        tmp = retranslate(sq->dna_num[proc[s]]);
//                        if(ap1->state & para->STATE_ORPHANE) {
//                            printf("%c%c%c", tolower(tmp[0]),tolower(tmp[1]), tolower(tmp[2]) );
//                        } else {
//                            printf("%c%c%c", toupper(tmp[0]), toupper(tmp[1]), toupper(tmp[2]) );
//                        }
//                        proc[s]++;
//                    } else {
//                        printf("---");
//                        //printf("%i",*ap[s][proc[s]].maxpos);
//                    }
//                } else {
//                    printf("---");
//                    //printf("%i",*ap[s][proc[s]].maxpos);
//                    //	  printf("\n%i %i %i\n", s, j,proc[s]);
//                }
//            }
//            printf("\n");
//        }
//        printf("\n");
//        printf("\n");
//    }
//    
//}
//

/**
 * prints the given alignment in fasta format
 * to the given file.
 */
void fasta_print_alignment_default(struct alignment *algn, char *filename)
{
    struct seq_col *scol = algn->scol;
    unsigned int slen = scol->length;
    
    unsigned int j,s,proc, max;
    struct seq* sq;
    struct algn_pos **ap = algn->algn;
    
    prepare_alignment(algn);
    max = algn->max_pos;
    struct algn_pos *ap1;
    
    FILE *fp;
    if( (fp = fopen( filename , "w")) == NULL) {
        merror("fasta_print_alignment(): Cannot open  file", filename );
    }
    //  fprintf(fp,"%s",print_info(algn));
    max = algn->max_pos;
    for(s=0;s<slen;s++) {
        sq = &(scol->seqs[s]);
        fprintf(fp, ">%s",sq->name);
        proc = 0;
        for(j=0;j<max;j++) {
            if(proc <sq->length) {
                if( (j%60)==0) fprintf(fp,"\n");
                ap1 = find_eqc(ap,s,proc);
                if(*ap1->eqcAlgnPos==j) {
                    if(ap1->state & para->STATE_ORPHANE) {
                        fprintf(fp, "%c", tolower(sq->data[proc]));
                    } else {
                        fprintf(fp, "%c", toupper(sq->data[proc]));
                    }
                    proc++;
                } else {
                    fprintf(fp,"-");
                }
            } else {
                if( (j%60)==0) fprintf(fp,"\n");
                fprintf(fp,"-");
            }
        }
        fprintf(fp,"\n");
    }
    fclose(fp);
}

//void fasta_print_alignment_dna_retranslate(struct alignment *algn, char *filename)
//{
//	char *tmp = "000";
//	struct seq_col *scol = algn->scol;
//    unsigned int slen = scol->length;
//    
//    unsigned int j,s,proc, max;
//    struct seq* sq;
//    struct algn_pos **ap = algn->algn;
//    
//    prepare_alignment(algn);
//    max = algn->max_pos;
//    struct algn_pos *ap1;
//    
//    FILE *fp;
//    if( (fp = fopen( filename , "w")) == NULL) {
//        merror("fasta_print_alignment(): Cannot open  file", filename );
//    }
//    //  fprintf(fp,"%s",print_info(algn));
//    max = algn->max_pos;
//    for(s=0;s<slen;s++)
//	{
//        sq = &(scol->seqs[s]);
//        fprintf(fp, ">%s",sq->name);
//        proc = 0;
//    	for(j=0;j<max;j++)
//		{
//    		if(proc <sq->length)
//			{
//				if( (j%20)==0) fprintf(fp,"\n");
//				ap1 = find_eqc(ap,s,proc);
//				if(*ap1->eqcAlgnPos==j)
//				{
//					tmp = retranslate(sq->dna_num[proc]);
//                    //					printf(fp,"%c,\n",sq->dna_num[proc]);
//	 				if(ap1->state & para->STATE_ORPHANE) {
// 						fprintf(fp, "%c%c%c", tolower(tmp[0]),tolower(tmp[1]), tolower(tmp[2]));
//	 				}
//					else
//					{
//	   					fprintf(fp, "%c%c%c", toupper(tmp[0]), toupper(tmp[1]), toupper(tmp[2]));
//	 				}
//					proc++;
//				}
//				else
//				{
//	  				fprintf(fp,"---");
//				}
//			}
//	 		else
//			{
//				if( (j%20)==0) fprintf(fp,"\n");
//				fprintf(fp,"---");
//			}
//		}
//        fprintf(fp,"\n");
//	}
//  	fclose(fp);
//}

char* print_info(struct alignment *algn)
{
	
	int i;
	char *output;
	char *line, *line2;
    
	if(NULL == ( output = (calloc(63,sizeof(char)))))
	{
		error("print_info(): Out of memory !");
	}
	strcat(output, "\n");
	for(i=0;i!=60; ++i)
	{
		strcat(output, "*");
	}
	strcat(output, "\n");
	if (para->DNA_TRANSLATION) {
		if(para->FIND_ORF){
			if(!para->ORF_FRAME){
                //				-L :
				if(NULL == ( line = (calloc(62, sizeof(char)))))
				{
					error("print_info(): Out of memory !");
				}
				if(NULL == ( output = (realloc(output,strlen(output)+15*61))))
				{
					error("print_info(): Out of memory !");
				}
				line = "Multiple Sequence Alignment (with translation)";
				line = output_line(line);
				strcat(output, line);
				line = "Input sequences in DNA";
				line = output_line(line);
				strcat(output, line);
				if(!para->OUTPUT) line = "Alignment output in aminoacids";
				else line = "Alignment output in DNA";
				line = output_line(line);
				strcat(output, line);
				line = "Sequences translated into aminoacids";
				line = output_line(line);
				strcat(output, line);
				line = "Only longest open reading frames aligned";
				line = output_line(line);
				strcat(output, line);
				line = "Sequence lengths cut mod 3 = 0";
				line = output_line(line);
				strcat(output, line);
				strcat(output,blank_line());
				line = "reading frame 1 :      123 123 123 123 ...";
				line = output_line_left(line);
				strcat(output, line);
				line = "reading frame 2 :    X 123 123 123 123 ...";
				line = output_line_left(line);
				strcat(output, line);
				line = "reading frame 3 :   XX 123 123 123 123 ...";
				line = output_line_left(line);
				strcat(output, line);
				line = "reading frame 4 :  ... 321 321 321 321 XX";
				line = output_line_left(line);
				strcat(output, line);
				line = "reading frame 5 :  ... 321 321 321 321 X";
				line = output_line_left(line);
				strcat(output, line);
				line = "reading frame 6 :  ... 321 321 321 321";
				line = output_line_left(line);
				strcat(output, line);
				strcat(output,blank_line());
				strcat(output,blank_line());
				for (i = 0; i != algn->scol->length; ++i)
				{
					int k;
					char *tmp;
					int tmp2;
					tmp = " :    reading frame  =  ";
					line[0] = algn->scol->seqs[i].name[0];
					
					for(k=1; k!=12 ; ++k)
					{
						line[k]=algn->scol->seqs[i].name[k];
						tmp2=algn->scol->seqs[i].orf_frame+48;
					}
                    
					line[12]='\0';
					strcat(line, tmp);
					line[strlen(line)-1]=tmp2;
					line[strlen(line)]='\0';
					if(NULL == ( output = (realloc(output,strlen(output)+62))))
					{
						error("print_info(): Out of memory !");
					}
					line = output_line(line);
					strcat(output, line);
                    
				}
				free(line);
			}
            
			else{
                //				-O :
				if(NULL == ( line = (calloc(62, sizeof(char)))))
				{
					error("print_info(): Out of memory !");
				}
				if(NULL == ( output = (realloc(output,strlen(output)+15*61))))
				{
					error("print_info(): Out of memory !");
				}
				line = "Multiple Sequence Alignment (with translation)";
				line = output_line(line);
				strcat(output, line);
				line = "Input sequences in DNA";
				line = output_line(line);
				strcat(output, line);
				if(!para->OUTPUT) line = "Alignment output in aminoacids";
				else line = "Alignment output in DNA";
				line = output_line(line);
				strcat(output, line);
				line = "Sequences translated into aminoacids";
				line = output_line(line);
				strcat(output, line);
				line = "reading frames found due to longest ORF";
				line = output_line(line);
				strcat(output, line);
				line = "Sequence lengths cut mod 3 = 0";
				line = output_line(line);
				strcat(output, line);
				strcat(output,blank_line());
				line = "reading frame 1 :      123 123 123 123 ...";
				line = output_line_left(line);
				strcat(output, line);
				line = "reading frame 2 :    X 123 123 123 123 ...";
				line = output_line_left(line);
				strcat(output, line);
				line = "reading frame 3 :   XX 123 123 123 123 ...";
				line = output_line_left(line);
				strcat(output, line);
				line = "reading frame 4 :  ... 321 321 321 321 XX";
				line = output_line_left(line);
				strcat(output, line);
				line = "reading frame 5 :  ... 321 321 321 321 X";
				line = output_line_left(line);
				strcat(output, line);
				line = "reading frame 6 :  ... 321 321 321 321";
				line = output_line_left(line);
				strcat(output, line);
				strcat(output,blank_line());
				strcat(output,blank_line());
				for (i = 0; i != algn->scol->length; ++i)
				{
					int k;
					char *tmp;
					int tmp2;
					tmp = " :    reading frame  =  ";
					line[0] = algn->scol->seqs[i].name[0];
					
					for(k=1; k!=12 ; ++k)
					{
						line[k]=algn->scol->seqs[i].name[k];
						tmp2=algn->scol->seqs[i].orf_frame+48;
					}
                    
					line[12]='\0';
					strcat(line, tmp);
					line[strlen(line)-1]=tmp2;
					line[strlen(line)]='\0';
					if(NULL == ( output = (realloc(output,strlen(output)+62))))
					{
						error("print_info(): Out of memory !");
					}
					line = output_line(line);
					strcat(output, line);
                    
				} 
				free(line);
			}
		}
		else{
            //			-T :
			if(NULL == ( line = (calloc(62, sizeof(char)))))
			{
				error("print_info(): Out of memory !");
			}
			if(NULL == ( output = (realloc(output,strlen(output)+5*61))))
			{
				error("print_info(): Out of memory !");
			}
			line = "Multiple Sequence Alignment (with translation)";
			line = output_line(line);
			strcat(output, line);
			line = "Input sequences in DNA";
			line = output_line(line);
			strcat(output, line);
			if(!para->OUTPUT) line = "Alignment output in aminoacids";
			else line = "Alignment output in DNA";
			line = output_line(line);
			strcat(output, line);
			line = "Sequences translated into aminoacids";
			line = output_line(line);
			strcat(output, line);
			line = "Sequence lengths cut mod 3 = 0";
			line = output_line(line);
			strcat(output, line);
			free(line);
		}
	}		
	else{
        // 		-D :
		if(NULL == ( line = (calloc(62, sizeof(char)))))
		{
			error("print_info(): Out of memory !");
		}
		if(NULL == ( line2 = (calloc(62, sizeof(char)))))
		{
			error("print_info(): Out of memory !");
		}
		if(NULL == ( output = (realloc(output,strlen(output)+3*61+1))))
		{
			error("print_info(): Out of memory !");
		}
        (void) strcpy(line,"Multiple Sequence Alignment");
		line = output_line(line);
		strcat(output, line);
		if(para->DNA_PARAMETERS) (void) strcpy(line2,"in DNA");
		else (void) strcpy(line2,"in aminoacids");
		sprintf(line,"Input sequences %s", line2);
		line = output_line(line);
		strcat(output, line);
		if(para->DNA_PARAMETERS) (void) strcpy(line2,"in DNA");
		else (void) strcpy(line2,"in aminoacids");
		sprintf(line,"Alignment output %s", line2);
		line = output_line(line);
		strcat(output, line);
		free(line);
		free(line2);
		
	}
	if(NULL == ( output = (realloc(output,strlen(output)+63))))
	{
		error("print_info(): Out of memory !");
	}
	
	for(i=0;i!=60; ++i)
	{
		strcat(output, "*");
	}
	strcat(output, "\n\n");
	return output;
}


/*SIDE EFFECT, releases original memory held by string */
char* output_line(char *string)
{
	char *tmp;
	if(NULL == ( tmp = (calloc(62, sizeof(char)))))
	{
        error("print_info(): Out of memory !");
	}
	int x, i, y;
	y = 0;
	if (strlen(string) %2 == 0) y = 1;
	x = (60 - (int)strlen(string)) /2 ;
	strcat(tmp, "*");
	for (i = 0; i != x-y; ++i)
	{
		strcat(tmp, " ");
	}
	strcat(tmp,string);
	for (i = 0; i != x-1; ++i)
	{
		strcat(tmp, " ");
	}
	strcat(tmp, "*\n");
    free(string);
	string=&(tmp[0]);
	return string;
}

/*SIDE EFFECT, releases original memory held by string */
char* output_line_left(char *string)
{
	char *tmp;
	if(NULL == ( tmp = (calloc(62, sizeof(char)))))
	{
        error("print_info(): Out of memory !");
	}
	int x, i, y;
	y = 0;
	x = (60 - (int)strlen(string)-3) ;
	strcat(tmp, "* ");
	strcat(tmp,string);
	for (i = 0; i != x; ++i)
	{
		strcat(tmp, " ");
	}
	strcat(tmp, "*\n");
    free(string);
	string=&(tmp[0]);
	return string;
}
char* blank_line()
{
	int i;
	char *string;
    
	if(NULL == ( string = (calloc(62,sizeof(char)))))
	{
		error("print_info(): Out of memory !");
	}
	strcat(string, "*");
	for (i = 0; i != 58; ++i)
	{
		strcat(string, " ");
	}
    
	strcat(string, "*\n");
	return string;
    
}

void print_pdist_matrix(struct prob_dist *sdist, char *filename)
{
	int lh, scr, mxscr;
	FILE *fp;
  	if( (fp = fopen( filename , "w")) == NULL) 
	{ 
    	merror("fasta_print_alignment(): Cannot open  file", filename );
  	}
	fprintf(fp,"%d\n",sdist->max_dlen);
 	for(lh=1; lh!=sdist->max_dlen+1; ++lh)
	{
		mxscr = lh * sdist->smatrix->max_score;
		for(scr=0; scr!=mxscr+1; ++scr) 
		{
			fprintf(fp,"%d %d %Le\n",lh,scr,sdist->data[lh][scr]);
		}
	}
    
}

#pragma mark -
#pragma mark diag

/**
 * factory method that creates a diagonal from the given sequence parts
 *
 * The pointer returned has to be deallocted explicitely from memory.
 */
struct diag* create_diag(int n1, struct seq* sq1, unsigned int sp1, int n2, struct seq* sq2, unsigned int sp2, int dlength)
{
    struct diag* dg = malloc(sizeof(struct diag));
    if(dg==NULL) error("create_diag(): Out of Memory !");
    
    if(sq1->length < sp1+dlength) {
        printf(" startpos=%i diaglength=%i seqlength=%i\n",sp1,dlength,sq1->length);
        assert(0);
        merror("create_diag(): startpos+diaglength exceeds sequence length in diag ", sq1->name);
    }
    if(sq2->length < sp2+dlength) {
        printf(" startpos=%i diaglength=%i seqlength=%i\n",sp2,dlength,sq2->length);
        assert(0);
        merror("create_diag(): startpos+diaglength exceeds sequence length in diag ", sq2->name);
    }
    
    dg->seq_p1.num = n1;
    dg->seq_p1.sq = sq1;
    dg->seq_p1.startpos = sp1;
    //dg->seq_p1.leftmargin = sp1;
    //dg->seq_p1.rightmargin = sq1->length - dlength - sp1;
    
    dg->seq_p2.num = n2;
    dg->seq_p2.sq = sq2;
    dg->seq_p2.startpos = sp2;
    //dg->seq_p1.leftmargin = sp2;
    //dg->seq_p1.rightmargin = sq2->length - dlength - sp2;
    
    dg->pred_diag = NULL;
    dg->col_pred_diag = NULL;
    dg->length = dlength;
    //dg->onlyOverThres= 0;
    dg->score= -1;
    dg->orig_score= -1;
    dg->weight = 0.0;
    dg->weight_sum = 0.0;
    dg->ov_weight = 0.0;
    dg->weight_fac = 1.0;
    dg->anchor = 0;
    dg->marked = 0;
    dg->multi_dg = 0;
    dg->multi_cont = NULL;
    dg->multi_length = 0;
    dg->meetsThreshold = 0;
    
    dg->neighbours = NULL;
    dg->degree = 0;
    dg->max_degree= 0;
    return dg;
}

/**
 * frees the memory of the given diagonal and the included seq_parts
 */
void free_diag(struct diag* dg)
{
    if(dg->multi_dg) {
        int i;
        for(i=0;i<dg->multi_length;i++) {
            if(dg->multi_cont[i]!=NULL) free_diag(dg->multi_cont[i]);
        }
        free(dg->multi_cont);
    }
    free(dg);
}

/**
 * frees the (sub) tree of the given gt_node
 */
void free_gt_node(struct gt_node *gtn)
{
    if(! gtn->isLeaf) {
        free_gt_node(gtn->succ1);
        free_gt_node(gtn->succ2);
    }
    free(gtn->seq_num);
    free(gtn);
}

/**
 * calculuates "m over n"
 */
unsigned long binomial(long m, long n)
{
    double result=1.0;
    long i;
    for(i=0;i<n;i++) {
        result *= ((double)(m-i))/(double)(n-i);
    }
    return (unsigned long)result;
}

/**
 * creates temporary probability distribution
 */
long double **create_tmp_pdist(struct prob_dist *pdist)
{
    int length = pdist->max_dlen;
    struct scr_matrix *smatrix = pdist->smatrix;
    
    long double **dist = calloc(length+1, sizeof(long double *));
    if(dist==NULL) error("create_tmp_pdist(): (1) Out of memory when allocating data !");
    
    int i;
    long mxscr, sm_max_scr=smatrix->max_score;
    for(i=0;i<=length;i++) {
        mxscr = i *sm_max_scr;
        dist[i] = calloc(mxscr+1, sizeof(long double ));
        if(dist[i]==NULL) error("create_tmp_pdist(): (3) Out of memory at iteration" );
    }
    return dist;
}

/**
 * frees temporary probability distribution
 */
void free_tmp_pdist(long double **dist, int length)
{
    int i;
    for(i=0;i<=length;i++) {
        free(dist[i]);
    }
    free(dist);
}

void fill_tmp_pdist(struct prob_dist *pdist, long double **tmp_dist, int slen1, int slen2)
{
    unsigned int length = pdist->max_dlen;
    struct scr_matrix * smatrix = pdist->smatrix;
    
    
    unsigned int i;
    long mxscr, sm_max_scr=smatrix->max_score,scr;
    
    long double factor, np, np2,prob;
    long double seq_factor= (((long double)slen1)*(slen2));
    
    for(i=1;i<=length;i++) {
        mxscr = i *sm_max_scr;
        
        factor = (long double)(seq_factor)/(long double)(4.0*i*i); // original !
        
        for(scr=0;scr<=mxscr;scr++) {
            prob = pdist->data[i][scr];
            
            np2 =prob * (factor);
            if(np2>=para->DIAG_CALC_WEIGHT_THRESHOLD) { // curent
//                np = (long double)1.0- pow(1.0-prob,factor); // current
                np = (long double)1.0- exp(factor * log(1.0-prob)); // current
            } else {
                np = np2;
            }
            tmp_dist[i][scr] = -log(np);
        }
    }
}

/**
 * calculates the score of the given diag by using the given score matrix. the
 * resulting score is stored within the diag
 * omitScore = -1: score calculation but weight interpolation with seqlen = 100
 * omitScore = 0:  normal
 * omitScore = 1:  no score calculation
 */
void real_calc_weight(struct diag* dg, struct scr_matrix* smatrix, struct prob_dist *pdist, char omitScore, long double **tmp_dist, struct alignment *algn )
{
    
    if(dg->multi_dg) {
        int i;
        dg->weight = 0.0;
        dg->meetsThreshold = 0;
        dg->length = 0;
        for(i=0;i<dg->multi_length;i++) {
            if(dg->multi_cont[i]!=NULL) {
                //printf("   before real calc %i %i %f\n", dg->length, dg->score, dg->weight);
                real_calc_weight(dg->multi_cont[i],smatrix, pdist, omitScore, tmp_dist, algn);
                if(dg->multi_cont[i]->length > dg->length)
                    dg->length = dg->multi_cont[i]->length;
                
                //printf("   real calc %i %i %f\n", dg->length, dg->score,dg->weight);
                dg->weight += dg->multi_cont[i]->weight;
                dg->meetsThreshold = dg->meetsThreshold | dg->multi_cont[i]->meetsThreshold;
                //printf(" readjusted %i %i %i %i %i %i %f %f %i\n", dg, dg->multi_cont[i],dg->multi_cont[i], dg->multi_dg, dg->multi_length, dg->weight, dg->multi_cont[i]->weight, dg->length);
                
            }
        }
        if(dg->length<=0) dg->meetsThreshold = 0;
        dg->total_weight = dg->weight;
        return;
    }
    
    if(dg->length==0) {
        dg->score = 0;
        dg->weight = 0.0;
        dg->meetsThreshold = 0;
        return;
    }
    
    unsigned int len = dg->length;
    
    int pos;
    long double np=0.0,np2;
    
    if(omitScore<=0) {
        unsigned int sp1=dg->seq_p1.startpos;
        unsigned int sp2=dg->seq_p2.startpos;
        char *data1 = dg->seq_p1.sq->data;
        char *data2 = dg->seq_p2.sq->data;
        int a1, a2;
        int *c2n = smatrix->char2num;
        int *sdata = smatrix ->data;
        int slen = smatrix->length;
        
        dg->score = 0;
        for(pos=0;pos<len;pos++) {
            a1 = c2n[(int) data1[sp1+pos]];
            a2 = c2n[(int) data2[sp2+pos]];
            dg->score+=(long)sdata[slen*a1+a2];
        }
    }
    
    long double prob;
    
    long double factor;
    
    dg->meetsThreshold = 0;
    
    
    if(dg->score <= pdist->smatrix->avg_sim_score*dg->length) {
        dg->total_weight = 0.0;
        dg->ov_weight = 0.0;
        dg->weight_fac = 1.0;
        dg->weight = 0.0;
        return;
    }
    
    // interpolate only for splitted diags that fall beyond the threshold
    /*
     if( (dg->orig_score>0)) {
     //if( (dg->weight<=0.0) && (dg->orig_score>0)) {
     dg->weight = ((double)dg->score)/((double)dg->orig_score)*dg->weight_sum;
     //    printf(" ws %.20f %.20f \n", dg->weight, dg->weight_sum);
     //if(dg->weight_sum>0.0) dg->weight = dg->weight_sum;
     }else */
    if(len<=pdist->max_dlen) {
        
        if(tmp_dist==NULL) {
            prob  =pdist->data[len][dg->score];
            
            /*
             if(omitScore>=0) {
             factor = (long double)((len + dg->seq_p1.leftmargin + dg->seq_p1.rightmargin )* (len + dg->seq_p2.leftmargin + dg->seq_p2.rightmargin ))/(long double)(4.0*len*len); // original
             } else {
             factor = (long double)(10000.0)/(long double)(4.0*len*len);
             }
             */
            //printf(" %i %i\n", dg->seq_p1.sq->length, len+dg->seq_p1.leftmargin+dg->seq_p1.rightmargin);
            
            if(omitScore>=0) {
                factor = (long double)((dg->seq_p1.sq->length )* (dg->seq_p2.sq->length ))/(long double)(4.0*len*len); // original
            } else {
                factor = (long double)(10000.0)/(long double)(4.0*len*len);
            }
            
            np2 =prob * (factor);
            if(np2>=para->DIAG_CALC_WEIGHT_THRESHOLD) {
                np = (long double)1.0- pow(1.0-prob,factor); // current
            } else {
                np = np2;
            }
            dg->weight = -log(np);
        } else {
            dg->weight = tmp_dist[len][dg->score];
        }
    }
    
    dg->total_weight = (dg->weight);//+dg->ov_weight);//* dg->weight_fac;
    if( (dg->length >= para->DIAG_MIN_LENGTH) && (dg->weight > para->DIAG_THRESHOLD_WEIGHT)) {
        dg->meetsThreshold = 1;
    } else {
    }
}

void calc_weight(struct diag* dg, struct scr_matrix* smatrix, struct prob_dist *pdist)
{
    real_calc_weight(dg, smatrix, pdist, 0,NULL,NULL);
}



/**
 * calculates the overlap weight for the given diag
 */
void calc_ov_weight(struct diag* dg, struct diag_col *dcol, struct scr_matrix* smatrix, struct prob_dist *pdist)
{
    int sn1 = dg->seq_p1.num;
    int sn2 = dg->seq_p2.num;
    int snt, sn;
    struct seq *seq1 = dg->seq_p1.sq;
    struct seq *seq2 = dg->seq_p1.sq;
    struct diag* tdg = create_diag(sn1, seq1,0,
                                   1, seq2,0,0);
    struct diag* dg2;
    struct simple_diag_col *sdcol;
    
    int i,j, slen=dcol->seq_amount, dlen;
    int sp1 = dg->seq_p2.startpos,tsp1;
    int tep1,t;
    double w;
    struct seq_part *seq_p, *d_seq_p1, *d_seq_p2;
    dg->ov_weight = 0.0;
    int tstartpos;
    if(dg->length >0) {
        for(sn=0;sn<2;sn++) {
            tstartpos = (sn==0 ? dg->seq_p1.startpos : dg->seq_p2.startpos);
            tdg->seq_p1.sq = (sn==0 ? seq1 : seq2);
            tdg->seq_p1.num = (sn==0 ? sn1 : sn2);;
            // other way
            seq_p = (sn==0 ? &dg->seq_p2 : &dg->seq_p1);
            tsp1 = seq_p->startpos;
            tep1 = seq_p->startpos+dg->length-1;
            
            snt = (sn==0 ? sn2 : sn1);
            //printf(" slen %i\n", slen);
            for(i=0; i<slen;i++) {
                if((i!=sn1) && (i!=sn2)) {
                    //printf("OV %i %i!\n",i, slen);
                    
                    sdcol = dcol->diag_matrix[(snt < i) ? (snt*slen + i) : (i*slen+snt)];
                    
                    tdg->seq_p2.num=i;
                    dlen = sdcol->length;
                    for(j=0;j<dlen;j++) {
                        dg2 = sdcol->data[j];
                        if(snt<i) {
                            d_seq_p1 = &dg2->seq_p1;
                            d_seq_p2 = &dg2->seq_p2;
                        } else {
                            d_seq_p1 = &dg2->seq_p2;
                            d_seq_p2 = &dg2->seq_p1;
                        }
                        if(j==0) {
                            tdg->seq_p2.sq = d_seq_p2->sq;
                        }
                        if(dg2->length >0) {
                            if(d_seq_p1->startpos>tsp1) tsp1 = d_seq_p1->startpos;
                            t=d_seq_p1->startpos+dg2->length-1;
                            if(t<tep1) tep1 = t;
                            if(tsp1<=tep1) {
                                //tdg->seq_p2.sq=dg2->seq_p2.sq;
                                tdg->seq_p1.startpos =  tstartpos + tsp1- sp1;
                                tdg->seq_p2.startpos = d_seq_p2->startpos + tsp1- d_seq_p1->startpos;
                                
                                tdg->length = tep1-tsp1+1;
                                //real_calc_weight(tdg, smatrix, pdist,-1,NULL,NULL);
                                real_calc_weight(tdg, smatrix, pdist,0,NULL,NULL);
                                if(tdg->meetsThreshold ) {
                                    w = tdg->weight;
                                    //printf("add %.20f\n",w);
                                    dg->ov_weight += w/2.0;
                                    //dg2->ov_weight += w;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    dg->total_weight = (dg->weight);//+dg->ov_weight)*dg->weight_fac;// + dg->ov_weight;
    free_diag(tdg);
}


/**
 * creates the collection of all diags
 *
 * The pointer returned (and the ones included in the struct)
 * has to be deallocted explicitely from memory.
 */
struct diag_col* create_diag_col(int seq_amount)
{
    struct diag_col *dc = calloc(1, sizeof(struct diag_col));
    if(dc==NULL) error("create_diag_col(): (1) Out of memory !");
    
    //printf("go for k\n");
    dc->diag_matrix = malloc(seq_amount*seq_amount*
                             sizeof(struct simple_diag_col *));
    //printf("2go for k\n");
    dc->gt_root = NULL;
    if(dc->diag_matrix==NULL) error("create_diag_col(): (2) Out of memory !");
    return dc;
}

/**
 * frees a diagcol and all data included in it
 */
void free_diag_col(struct diag_col* dcol)
{
    int s1;
    int s2;
    int sl=dcol->seq_amount;
    // printf("damount: %i\n", dcol->diag_amount);
    //printf("--------------------------------\n");
    for(s1 =0;s1<dcol->diag_amount;s1++) {
        //    print_diag(dcol->diags[s1]);
        //printf(" NO FREEEEEEEEEE %i %i\n", s1,dcol->diags[s1]);
        free_diag(dcol->diags[s1]);
    }
    free(dcol->diags);
    for(s1=0;s1<sl;s1++) {
        for(s2=s1+1;s2<sl;s2++) {
            free(dcol->diag_matrix[s1+sl*s2]->data);
            free(dcol->diag_matrix[s1+sl*s2]);
        }
    }
    free(dcol->diag_matrix);
    if(dcol->gt_root!=NULL) free_gt_node(dcol->gt_root);
    free(dcol);
}


/**
 * finds all relevant multi diags with respect to the given alignment
 * and two groups of sequences that come from the guided method
 *
 * The pointer returned (and the ones included in the struct)
 * has to be deallocted explicitely from memory.
 */
struct simple_diag_col* find_diags_guided(struct scr_matrix *smatrix, struct prob_dist *pdist, struct gt_node* n1, struct gt_node* n2, struct alignment *algn, double thres_weight, int*** diag_info)
{
    
    struct simple_diag_col* dcol = calloc(1, sizeof(struct simple_diag_col));
    if(dcol==NULL) error("find_diags_guided(): (1) Out of memory !");
    
    prepare_alignment(algn);
    
    
    
    struct seq_col *scol = algn->scol;
    struct seq *seq1 = &(scol->seqs[n1->seq_num[0]]);
    struct seq *seq2 = &(scol->seqs[n2->seq_num[0]]);
    int slen = scol->length;
    
    int ni1, ni2;
    struct seq *seq_n1, *seq_n2;
    
    unsigned int size = 16;
    int length = 0;
    struct diag **data = calloc(size, sizeof(struct diag* ));
    //   printf("go for k\n");
    if(data==NULL) error("find_diags_guided(): (2) Out of memory !");
    
    struct algn_pos *tap;// = find_eqc(algn->algn, seq1->num, seq1->length-1);
    long slen1 = algn->max_pos;
    long slen2 = slen1;
    
    
    unsigned int max_dlen = pdist->max_dlen;
    
    struct diag *sdg, *tsdg;
    struct diag *dg = create_diag(seq1->num, seq1,0,
                                  seq2->num, seq2,0,0);
    
    struct diag* tdg,*tdg2;
    
    int  i,j,k,l,kposi,kposj,jmin=0,jmax=(int)slen2;
    
    
    int sn1 = seq1->num;
    int sn2 = seq2->num;
    
    int *c2n = smatrix->char2num;
    int *sdata = smatrix ->data;
    char *data1;// = seq1->data;
    char *data2;// = seq2->data;
    int smatrixlen = smatrix->length;
    int a1,a2;
    
    int maxslen = (int)slen1;
    if(slen2>maxslen) maxslen = (int)slen2;
    
    //long double **tmp_dist = create_tmp_pdist(pdist);
    //fill_tmp_pdist(pdist, tmp_dist, slen1, slen1);
    
    
    int max_pool = (int)(slen1+slen2-1);
    struct diag **diag_col = malloc(sizeof(struct diag*)*(slen1+1));
    struct diag **diag_row = malloc(sizeof(struct diag*)*(slen2+1));
    struct diag **pool_diags=malloc(sizeof(struct diag *)*max_pool);
    int pooled = 0;
    double thres_sim_score =para->PROT_SIM_SCORE_THRESHOLD;
//    char hasAli = (algn!=NULL);
    double diag_thres = para->DIAG_THRESHOLD_WEIGHT;
    double avg_sim_score = pdist->smatrix->avg_sim_score;
    
    int maxd,maxd2;
//    int cons_maxd;
//    int score1=0;
    int score2=0;
    char prevail;
    int extended,passed;//,actuals;
    int passed2;
    int max_under = 0;//para->PROT_DIAG_MAX_UNDER_THRESHOLD_POS;
    int max_under_inner =0;//2*para->PROT_DIAG_MAX_UNDER_THRESHOLD_POS;
    
    double dg_thres_weight = thres_weight;// * n1->seq_num_length*n2->seq_num_length;
    
    //int min_motives = n1->seq_num_length;
    //if(n2->seq_num_length > min_motives) min_motives = n2->seq_num_length;
    //min_motives++;
    int min_motives = sqrt(n1->seq_num_length * n2->seq_num_length);
    if(min_motives<2) min_motives = 2;
    //  if(min_motives>(n1->seq_num_length*n2->seq_num_length)) min_motives = (n1->seq_num_length * n2->seq_num_length);
    
    char stst;
//    char mstatus;
    int slensq = slen*slen;
    int sqind;
    char startstop[slensq];
    char startstop_pre[slen];
    int c_klen[slensq];
    int c_kscore[slensq];
    
    int rev_pos[slen][slen1];
    struct gt_node *tgtn;
    
    
    // build reverse algn pos
    
    int iterlen = 0;
    int iter[slen*slen];
    for(ni1=0;ni1<n1->seq_num_length;ni1++) {
        sn1 = n1->seq_num[ni1];
        for(ni2=0;ni2<n2->seq_num_length;ni2++) {
            //printf("   begin MARKER MARKER MARKER %i %i %i\n",i,j,k);
            sn2 = n2->seq_num[ni2];
            sqind = sn1*slen+sn2;
            iter[iterlen++] = sqind;
        }
    }
    
    
    dg->multi_dg = 1;
    dg->marked = 1;
    dg->multi_length = slensq;
    dg->multi_cont = malloc(sizeof(struct diag *)*slensq);
    memset(dg->multi_cont, 0, sizeof(struct diag *)*slensq);
    for(i=0;i<slensq;i++) {
        dg->multi_cont[i]=NULL;
    }
    
    maxd = n1->seq_num_length + n2->seq_num_length;
    for(i=0;i<maxd;i++) {
        k = i;
        tgtn = n1;
        if(i>=n1->seq_num_length) {
            k-=n1->seq_num_length;
            tgtn = n2;
        }
        ni1  = tgtn->seq_num[k];
        seq_n1 = &(scol->seqs[ni1]);
        for(j=0;j<slen1;j++) {
            rev_pos[ni1][j]=-1;
        }
        for(j=0;j<seq_n1->length;j++) {
            tap = find_eqc(algn->algn, seq_n1->num, j);
            rev_pos[ni1][*(tap->eqcAlgnPos)] = j;
        }
    }
    
    // DIALIGN
    for(k=0;k<=slen1;k++) {
        //printf(" begin %i %i %i %i\n",k,slen, seq1->num, seq2->num);
        diag_col[k]=create_diag(seq1->num, seq1,0,
                                seq2->num, seq2,0,0);
        //printf(" middle\n");
        diag_col[k]->multi_dg = 1;
        diag_col[k]->marked = 1;
        diag_col[k]->multi_length = 0;
        
        //diag_col[k]->length = 1;
        diag_col[k]->weight_sum = 0.0;
        diag_col[k]->weight = 0.0;
        diag_col[k]->meetsThreshold = 0;
        pool_diags[pooled] = diag_col[k];
        pool_diags[pooled]->pool_pos = pooled;
        pooled++;
        //printf(" end %i %i\n",k,slen);
    }
    for(k=0;k<=slen2;k++) {
        diag_row[k] = diag_col[0];
    }
    
    double old_weight;
    
    if(max_dlen> slen1/2.0) max_dlen = slen1/2.0;
    if(max_dlen> slen2/2.0) max_dlen = slen2/2.0;
    
    for(i=0;i<=slen1;i++) {
        
        // merge row/col
        //printf("before \n");
        if(i>0) {
            tdg = diag_col[i];
            while(tdg!=NULL) {
                kposi = tdg->seq_p2.startpos+tdg->length;
                //printf("  kposi %i %i %i\n", diag_row[kposi], kposi, slen2);
                if(tdg->weight_sum > diag_row[kposi]->weight_sum) {
                    diag_row[kposi] = tdg;
                    prevail = 1;
                } else {
                    prevail = 0;
                }
                tdg2 = tdg;
                tdg = tdg->col_pred_diag;
                if(! prevail) {
                    pool_diags[tdg2->pool_pos]=NULL;
                    //printf(" BEFORE\n");
                    free_diag(tdg2);
                    //printf(" AFTER\n");
                }
            }
        }
        //printf("after \n");
        
        for(j=1;j<=slen2;j++) {
            if(diag_row[j-1]->weight_sum > diag_row[j]->weight_sum) {
                diag_row[j] = diag_row[j-1];
            }
        }
        
        if(i==slen1) break;
        
        for(j=jmin;j<jmax;j++) {
            old_weight = 0.0;
            memset(startstop, 0, sizeof(char)*slen*slen);
            memset(startstop_pre, 0, sizeof(char)*slen);
            maxd = max_dlen;
            if( (i+maxd)>slen1) maxd = (int)slen1-i;
            if( (j+maxd)>slen1) maxd = (int)slen1-j;
            /*
             for(l=0;l<iterlen;l++) {
             sdg = dg->multi_cont[iter[l]];
             if(sdg!=NULL) {
             free_diag(sdg);
             dg->multi_cont[iter[l]]=NULL;
             }
             }
             */
            dg->seq_p1.startpos = i;
            dg->seq_p2.startpos = j;
            
            passed = 0;
            passed2 = 0;
            //      actuals = 0;
            for(k=1;k<=maxd;k++) {
                //printf(" %i %i %i %i\n",i,j,k,slen1);
                kposi = i+k-1;
                kposj = j+k-1;
                maxd2 = maxd;
                maxd = 0;
                //printf(" k= %i\n",k);
                extended = 0;
                //mstatus = 0;
                
                for(ni1=0;ni1<n1->seq_num_length;ni1++) {
                    sn1 = n1->seq_num[ni1];
                    seq_n1 = &(scol->seqs[sn1]);
                    
                    if( (rev_pos[sn1][kposi]>=0)&& (startstop_pre[sn1]!=2) )  {
                        data1 = seq_n1->data;
                        startstop_pre[sn1]=2;
                        for(ni2=0;ni2<n2->seq_num_length;ni2++) {
                            //printf("   begin MARKER MARKER MARKER %i %i %i\n",i,j,k);
                            sn2 = n2->seq_num[ni2];
                            seq_n2 = &(scol->seqs[sn2]);
                            sqind = sn1*slen+sn2;
                            stst = startstop[sqind];
                            //if(stst==1) mstatus += stst;
                            
                            //printf("   end MARKER MARKER MARKER %i %i\n",sn2, kpos);
                            if( (rev_pos[sn2][kposj]>=0) && (stst!=2)
                               && (diag_info[ni1][ni2][rev_pos[sn1][kposi]]==rev_pos[sn2][kposj])
                               ) {
                                
                                dg->length = k;
                                
                                data2 = seq_n2->data;
                                
                                a1 = c2n[(int) data1[rev_pos[sn1][kposi]]];
                                a2 = c2n[(int) data2[rev_pos[sn2][kposj]]];
                                score2 = sdata[smatrixlen*a1+a2];
                                sdg = NULL;
                                if( (stst==0) && (score2 >= thres_sim_score)) {
                                    //if(rev_pos[sn1][kpos]>1000) printf(" %i %i %i\n", sn1,kpos, rev_pos[sn1][kpos]);
                                    sdg = dg->multi_cont[sqind];
                                    if(sdg==NULL) {
                                        sdg = create_diag(sn1, seq_n1, rev_pos[sn1][kposi],
                                                          sn2, seq_n2, rev_pos[sn2][kposj],0);
                                    } else {
                                        sdg->seq_p1.num = sn1;
                                        sdg->seq_p1.sq = seq_n1;
                                        sdg->seq_p1.startpos = rev_pos[sn1][kposi];
                                        sdg->seq_p2.num = sn2;
                                        sdg->seq_p2.sq = seq_n2;
                                        sdg->seq_p2.startpos = rev_pos[sn2][kposj];
                                        sdg->length = 0;
                                    }
                                    sdg->score = 0;
                                    dg->multi_cont[sqind] = sdg;
                                    startstop[sqind] = 1;
                                    passed++;
                                    c_klen[sqind] = 0;
                                    c_kscore[sqind] = 0;
                                } else if(stst==1) {
                                    sdg = dg->multi_cont[sqind];
                                } else {
                                    //startstop[sqind]=2;
                                    if(k>max_under) startstop[sqind] = 2;
                                    sdg = NULL; // Report: sdg == NULL.
                                }
                                
                                if(sdg!=NULL) {
                                    maxd = maxd2; // reallow next k
                                    //printf("  length compare %i %i %i %i\n",sqind, sdg->length+1,k,sdg);
                                    c_klen[sqind]++;
                                    c_kscore[sqind]+=score2;
                                    
                                    // if( (sdg->seq_p1.startpos==20) && (sdg->seq_p2.startpos==2) ) printf(" %i %i %i %i %i\n", k, c_klen[sqind], sdg->seq_p1.startpos, sdg->seq_p2.startpos);
                                    
                                    
                                    if(((sdg->score+c_kscore[sqind]) < (avg_sim_score*(sdg->length+c_klen[sqind])))) {
                                        if( (sdg->length+c_klen[sqind])>max_under_inner) startstop[sqind]=2;
                                    }
                                    /*
                                     if( (c_klen[sqind]>=1) &&
                                     (c_kscore[sqind]< (para->PROT_DIAG_AVG_SCORE_THRESHOLD*c_klen[sqind]))) {
                                     passed--;
                                     startstop[sqind]=2;
                                     }
                                     */
                                    
                                    if( (c_klen[sqind]>=para->PROT_DIAG_MAX_UNDER_THRESHOLD_POS) &&
                                       (c_kscore[sqind]< (para->PROT_DIAG_AVG_SCORE_THRESHOLD*c_klen[sqind]))) {
                                        if ( ((sdg->length+c_klen[sqind])>para->PROT_DIAG_MIN_LENGTH_THRESHOLD)) {
                                            passed--;
                                            startstop[sqind]=2;
                                        }
                                    }
                                    
                                    //printf(" diag length %i %i %i %i %i\n", sn1, sn2, i,j, sdg->length);
                                    if( (score2 >= thres_sim_score) && (startstop[sqind]==1)) {
                                        //printf("  very before score %i\n", sdg->score);
                                        sdg->score += c_kscore[sqind];
                                        sdg->length += c_klen[sqind];
                                        //printf(" begin CONSIDER SDG %i %i %i %f %f\n",sn1,sn2,sdg->length,sdg->weight, diag_thres);
                                        //sdg->weight = tmp_dist[sdg->length][sdg->score];
                                        calc_weight(sdg, smatrix, pdist);
                                        //printf(" before score %i %i  %.20f %i %i\n", sdg->score, sdg->length, sdg->weight, c_kscore[sqind], c_klen[sqind]);
                                        /*
                                         score1 = sdg->score;
                                         if(score1!=sdg->score) {
                                         print_diag(sdg);
                                         printf("  score %i %i %i %.20f sc %i %i len %i %i %i \n", score1, sdg->score, sdg->length, sdg->weight, c_kscore[sqind],score2, c_klen[sqind],a1,a2);
                                         error(" score changed!\n");
                                         }
                                         */
                                        
                                        //printf("  after score %i %i %.20f\n", sdg->score, sdg->length, sdg->weight);
                                        //printf(" END\n");
                                        //if(sdg->weight>0.0) printf(" end CONSIDER SDG %i %i %i %i %f %f\n",sn1, sn2, sdg->length, sdg->score, sdg->weight, diag_thres);
                                        c_klen[sqind]=0;
                                        c_kscore[sqind]=0;
                                        sdg->meetsThreshold = (sdg->weight>diag_thres ? 1 : 0);
                                        extended++;
                                    }
                                }
                            } else {
                                if(startstop[sqind]==1) {
                                    startstop[sqind]=2;
                                    passed--;
                                }
                            }
                            if(startstop[sqind]!=2) startstop_pre[sn1]=0;
                        }
                    } else {
                        for(ni2=0;ni2<n2->seq_num_length;ni2++) {
                            sn2 = n2->seq_num[ni2];
                            sqind = sn1*slen+sn2;
                            if(startstop[sqind]==1) {
                                startstop[sqind]=2;
                                passed--;
                            }
                        }
                    }
                }
                
                // if no inner starting diag break
                if( (k==1) && !extended) maxd = 0;
                // if only one inner diag and proceeded max_under further then break
                if( (passed2>=0) && (passed<min_motives) && ( (k-passed2)>=max_under)) {
                    //printf(" break %i\n");
                    maxd = 0;
                }
                // find the last k  where >=2 inner diags where active
                if(passed >= min_motives) passed2 = k;
                
                if( ((extended) && (passed>=min_motives))) { // && (passed>1)){// && (mstatus>1)) {
                    dg->weight = 0.0;
                    extended = 0;
                    for(l=0;l<iterlen;l++) {
                        sdg = dg->multi_cont[iter[l]];
                        if( (sdg!=NULL) && (startstop[iter[l]]>0) && (sdg->weight >= thres_weight)) {
                            extended++;
                            dg->weight += sdg->weight;
                        }
                    }
                    dg->meetsThreshold = (dg->weight>diag_thres ? 1 : 0);
                    
                    if(dg->meetsThreshold && (extended >=min_motives) && (dg->weight>=old_weight) && (dg->weight>=dg_thres_weight)) {
                        old_weight = dg->weight;
                        
                        if(max_pool<=pooled) {
                            max_pool += maxslen;
                            pool_diags = realloc(pool_diags, sizeof(struct diag*)*max_pool);
                            //printf(" pool size %i %.20f %.20f\n", max_pool, dg->weight, old_weight);
                        }
                        tdg = malloc(sizeof(struct diag));
                        pool_diags[pooled] = tdg;
                        dg->pool_pos = pooled;
                        pooled++;
                        *tdg = *dg;
                        tdg->pred_diag = diag_row[j];
                        tdg->weight_sum =  diag_row[j]->weight_sum+tdg->weight;
                        tdg->col_pred_diag = diag_col[i+k];
                        
                        diag_col[i+k] = tdg;
                        
                        dg->multi_cont = malloc(sizeof(struct diag *)*slensq);
                        memset(dg->multi_cont, 0, sizeof(struct diag *)*slensq);
                        for(l=0;l<iterlen;l++) {
                            
                            tsdg = tdg->multi_cont[iter[l]];
                            if((tsdg!=NULL)) { // && (startstop[iter[l]]==1)){
                                sdg = malloc(sizeof(struct diag));
                                
                                if(startstop[iter[l]]==0) {
                                    tsdg->length = 0;
                                    tsdg->score = 0;
                                    tsdg->weight = 0.0;
                                    tsdg->total_weight = 0.0;
                                }
                                
                                *sdg = *tsdg;
                                dg->multi_cont[iter[l]]=sdg;
                            } else {
                                dg->multi_cont[iter[l]]=NULL;
                            }
                            
                            //dg->multi_cont[iter[l]]=NULL;
                        }
                    }
                }
                //if(k > 20) printf("  maxk= %i\n",k);
                
            }
        }
    }
    //printf(" after main %i\n",k);
    tdg = diag_row[slen2];
    dcol->total_weight = 0;
    double lencomp = (log(slen1)+log(slen2));
    length = 0;
    
    while((tdg!=NULL)) {
        //if (tdg->weight <=0.0) break;
        if(tdg->meetsThreshold) {
            //      printf(" add tdg %i %i %i\n", tdg, tdg->length, tdg->meetsThreshold);
            dcol->total_weight += tdg->weight+lencomp;
            
            data[length] = tdg;
            l = 0;
            tdg->weight = 0.0;
            for(i=0;i<slensq;i++) {
                sdg = tdg->multi_cont[i];
                if(sdg!=NULL) {
                    calc_weight(sdg, smatrix, pdist);
                    
                    if ((sdg->length>0) && (sdg->meetsThreshold) && (sdg->weight > thres_weight)){
                        tdg->multi_cont[l++] = sdg;
                        tdg->weight += sdg->weight;
                        //printf ("  tdg %i %f %f\n", tdg, tdg->weight, sdg->weight);
                    }
                }
            }
            tdg->multi_length = l;
            if( (tdg->multi_length>=min_motives) && (tdg->weight >= dg_thres_weight)) {
                tdg->marked = 1;
                tdg->total_weight = tdg->weight;
                tdg->weight_sum = -1.0;
                length++;
            } else {
                // TODO free ??
            }
            
            if(length >= size) {
                size += 64;
                data = realloc(data, sizeof(struct diag *)*size);
                if(data==NULL) error("find_diags(): (3) Out of memory !");
            }
        }
        tdg = tdg->pred_diag;
    }
    
    for(k=0;k<pooled;k++) {
        if(pool_diags[k]!=NULL)
            if(pool_diags[k]->weight_sum>-1.0) {
                free_diag(pool_diags[k]);
            }
    }
    
    free(pool_diags);
    free(diag_col);
    free(diag_row);
    free_diag(dg);
    //free_tmp_pdist(tmp_dist, pdist->max_dlen);
    
    dcol->length = length;
    
    data = realloc(data, sizeof(struct diag *)*length);
    dcol->data = data;
    
    return dcol;
}

/**
 * finds all relevant diags by the DIALIGN METHOD
 *
 * The pointer returned (and the ones included in the struct)
 * has to be deallocted explicitely from memory.
 */
struct simple_diag_col* find_diags_dialign(struct scr_matrix *smatrix, struct prob_dist *pdist, struct seq* seq1, struct seq* seq2, struct alignment *algn, long double **tmp_dist, int round)
{
    struct simple_diag_col* dcol = calloc(1, sizeof(struct simple_diag_col));
    if(dcol==NULL) error("find_diags_dialign(): (1) Out of memory !");
    
    unsigned int size = 16;
    int length = 0;
    struct diag **data = calloc(size, sizeof(struct diag* ));
    //   printf("go for k\n");
    if(data==NULL) error("find_diags_dialign(): (2) Out of memory !");
    
    long slen1 = seq1->length;
    long slen2 = seq2->length;
    unsigned int max_dlen = pdist->max_dlen;
    
    struct diag *dg = create_diag(seq1->num, seq1,0,
                                  seq2->num, seq2,0,0);
    struct diag* tdg,*tdg2;
    
    int  i,j,k,kpos,jmin=0,jmax=(int)slen2;
    
    int sn1 = seq1->num;
    int sn2 = seq2->num;
    
    int *c2n = smatrix->char2num;
    int *sdata = smatrix ->data;
    char *data1 = seq1->data;
    char *data2 = seq2->data;
    int smatrixlen = smatrix->length;
    int a1,a2;
    int c_klen,c_kscore;
    
    int maxslen = (int)slen1;
    if(slen2>maxslen) maxslen = (int)slen2;
    
    int max_pool = (int)(slen1+slen2-1);
    struct diag **diag_col = malloc(sizeof(struct diag*)*(slen1+1));
    struct diag **diag_row = malloc(sizeof(struct diag*)*(slen2+1));
    struct diag **pool_diags=malloc(sizeof(struct diag *)*max_pool);
    int pooled = 0;
    double thres_sim_score =para->PROT_SIM_SCORE_THRESHOLD;
    char hasAli = (round>1); //(algn!=NULL);
    struct algn_pos **ap,*tap;
    double diag_thres = para->DIAG_THRESHOLD_WEIGHT;
    double avg_sim_score = pdist->smatrix->avg_sim_score;
    
    int maxd,maxd2,cons_maxd;
    double score1=0,score2 = 0;
    char prevail;
    
    if(hasAli || para->DO_ANCHOR) {
        ap = algn->algn;
    }
    
    // DIALIGN
    for(k=0;k<=slen1;k++) {
        diag_col[k]=create_diag(seq1->num, seq1,0,
                                seq2->num, seq2,0,1);
        
        //diag_col[k]->length = 1;
        diag_col[k]->weight_sum = 0.0;
        diag_col[k]->weight = 0.0;
        pool_diags[pooled] = diag_col[k];
        pool_diags[pooled]->pool_pos = pooled;
        pooled++;
    }
    for(k=0;k<=slen2;k++) {
        diag_row[k] = diag_col[0];
    }
    
    double old_weight;
    
    if(max_dlen> slen1/2.0) max_dlen = slen1/2.0;
    if(max_dlen> slen2/2.0) max_dlen = slen2/2.0;
    
    for(i=0;i<=slen1;i++) {
        
        // merge row/col
        if(i>0) {
            tdg = diag_col[i];
            while(tdg!=NULL) {
                kpos = tdg->seq_p2.startpos+tdg->length;
                if(tdg->weight_sum > diag_row[kpos]->weight_sum) {
                    diag_row[kpos] = tdg;
                    prevail = 1;
                } else {
                    prevail = 0;
                }
                tdg2 = tdg;
                tdg = tdg->col_pred_diag;
                if(! prevail) {
                    pool_diags[tdg2->pool_pos]=NULL;
                    free_diag(tdg2);
                }
            }
        }
        for(j=1;j<=slen2;j++) {
            if(diag_row[j-1]->weight_sum > diag_row[j]->weight_sum) {
                diag_row[j] = diag_row[j-1];
            }
        }
        if(i==slen1) break;
        if(hasAli || para->DO_ANCHOR) {
            tap = find_eqc(ap, sn1, i);
            
            if(tap->predF!=NULL) {
                jmin = tap->predF[sn2]+1;
            } else {
                jmin = 0;
            }
            
            if(tap->succF!=NULL) {
                jmax = tap->succF[sn2];
            }else {
                jmax = (int)slen2;
            }
            
            if(jmin<0) jmin = 0;
        }
        
        if(para->DO_ANCHOR) {
            if( (jmin-1)==jmax) {
                jmin--;
                jmax++;
            }
        }
        
        for(j=jmin;j<jmax;j++) {
            
            if(i<slen1 && j<slen2) {
                a1 = c2n[(int) data1[i]];
                a2 = c2n[(int) data2[j]];
                score1 = sdata[smatrixlen*a1+a2];
            } else {
                score1 = 0;
            }
            
            if(score1>=thres_sim_score) {
                maxd = (int)slen1 - i;
                maxd2 = (int)slen2 - j;
                if(maxd >maxd2) maxd = maxd2;
                if(maxd > max_dlen) maxd = max_dlen;
                
                dg->seq_p1.startpos = i;
                dg->seq_p2.startpos = j;
                dg->score = score1;
                
                cons_maxd = maxd+1;
                old_weight = 0.0;
                
                c_klen = 0;
                c_kscore = 0;
                
                for(k=1;k<=maxd;k++) {
                    dg->length = k;
                    kpos = i+k;
                    if(hasAli || para->DO_ANCHOR) {
                        a1 = i+k-1;
                        a2 = j+k-1;
                        tap = find_eqc(ap, sn1, a1);
                        
                        if (! ( (para->DO_ANCHOR) && (tap->predF!=NULL) && (tap->succF!=NULL) &&
                               (tap->predF[sn2]==tap->succF[sn2]) &&
                               (tap->predF[sn2]==a2)
                               )
                            ) {
                            
                            if(tap->predF!=NULL) {
                                if( (tap->predF[sn2] - a2)>0) break;
                                /*
                                 if(tap->predF[sn2]==a2) {
                                 if( (tap->succF==NULL) || (tap->predF[sn2]!=tap->succF[sn2])) break;
                                 }
                                 */
                            }
                            if(tap->succF!=NULL) {
                                if((a2 - tap->succF[sn2])>0) break;
                                /*
                                 if(tap->succF[sn2]==a2) {
                                 if( (tap->predF==NULL) || (tap->predF[sn2]!=tap->succF[sn2])) break;
                                 }
                                 */
                            }
                        }
                    }
                    
                    if(k>1) {
                        a1 = c2n[(int) data1[kpos-1]];
                        a2 = c2n[(int) data2[j+k-1]];
                        score2 = sdata[smatrixlen*a1+a2];
                        dg->score += score2;
                    } else {
                        score2 = score1;
                    }
                    
                    
                    if( dg->score < (avg_sim_score*(double)k)) {
                        break;
                    }
                    
                    c_klen++;
                    c_kscore+=score2;
                    
                    if( (c_klen>=para->PROT_DIAG_MAX_UNDER_THRESHOLD_POS) &&
                       (c_kscore< (para->PROT_DIAG_AVG_SCORE_THRESHOLD*c_klen))) {
                        if ( (k>para->PROT_DIAG_MIN_LENGTH_THRESHOLD)) {
                            break;
                        } else {
                            if(maxd>para->PROT_DIAG_MIN_LENGTH_THRESHOLD) maxd = para->PROT_DIAG_MIN_LENGTH_THRESHOLD;
                        }
                    }
                    
                    if(score2 >= thres_sim_score) {
                        c_klen=0;
                        c_kscore=0;
                        if(1) {
                            if(!hasAli) {
                                dg->weight = tmp_dist[k][dg->score];
                                dg->meetsThreshold = (dg->weight>diag_thres ? 1 : 0);
                            } else {
                                real_calc_weight(dg, smatrix, pdist,1,tmp_dist,algn);
                            }
                            if(dg->meetsThreshold && (dg->weight>=old_weight)) {
                                old_weight = dg->weight;
                                if(max_pool<=pooled) {
                                    max_pool += maxslen;
                                    pool_diags = realloc(pool_diags, sizeof(struct diag*)*max_pool);
                                    //printf(" old pool size %i\n", max_pool);
                                }
                                tdg = malloc(sizeof(struct diag));
                                pool_diags[pooled] = tdg;
                                dg->pool_pos = pooled;
                                pooled++;
                                *tdg = *dg;
                                tdg->pred_diag = diag_row[j];
                                tdg->weight_sum =  diag_row[j]->weight_sum+tdg->weight;
                                tdg->col_pred_diag = diag_col[kpos];
                                
                                diag_col[kpos] = tdg;
                            }
                        }
                    }
                }
            }
            
        }
    }
    
    tdg = diag_row[slen2];
    dcol->total_weight = 0;
    double lencomp = (log(slen1)+log(slen2));
    
    while((tdg!=NULL)) {
        if (tdg->weight <=0.0) break;
        if(1) {
            dcol->total_weight += tdg->weight+lencomp;
            
            data[length] = tdg;
            tdg->weight_sum = -1.0;
            length++;
            if(length >= size) {
                size += 64;
                data = realloc(data, sizeof(struct diag *)*size);
                if(data==NULL) error("find_diags(): (3) Out of memory !");
            }
        }
        tdg = tdg->pred_diag;
    }
    
    for(k=0;k<pooled;k++) {
        if(pool_diags[k]!=NULL)
            if(pool_diags[k]->weight_sum>-1.0) {
                free(pool_diags[k]);
            }
    }
    
    free(pool_diags);
    free(diag_col);
    free(diag_row);
    free_diag(dg);
    dcol->length = length;
    
    data = realloc(data, sizeof(struct diag *)*length);
    dcol->data = data;
    
    if(para->DEBUG>5) {
        for(i=0;i<length;i++) {
            print_diag(data[i]);
            printf("\n");
        }
    }
    
    return dcol;
}


/**
 * finds all relevant diags by dynamic programming on diagonal stripes
 *
 * The pointer returned (and the ones included in the struct)
 * has to be deallocted explicitely from memory.
 */
struct simple_diag_col* find_diags_dyn(struct scr_matrix *smatrix, struct prob_dist *pdist, struct seq* seq1, struct seq* seq2, struct alignment *algn, long double **tmp_dist)
{
    
    struct simple_diag_col* dcol = calloc(1, sizeof(struct simple_diag_col));
    if(dcol==NULL) error("find_diags_dyn(): (1) Out of memory !");
    
    unsigned int size = 64;
    int  l, k,lastk, maxl;
    int length = 0;
    struct diag **data = calloc(size, sizeof(struct diag *));
    if(data==NULL) error("find_diags_dyn(): (2) Out of memory !");
    
    int slen1 = seq1->length;
    int slen2 = seq2->length;
    unsigned int max_dlen = pdist->max_dlen;
    
    struct diag* dg = create_diag(seq1->num, seq1,0,
                                  seq2->num, seq2,0,0);
    struct diag* tdg;
    
    int i,j,d;
    
    int sn1 = seq1->num;
    int sn2 = seq2->num;
    //    printf("%i\n",slen2);
    int *c2n = smatrix->char2num;
    int *sdata = smatrix ->data;
    char *data1 = seq1->data;
    char *data2 = seq2->data;
    int slen = smatrix->length;
    
    int a1,a2;
    int score1=0,score2 = 0;
    
    int maxslen = slen1;
    if(slen2>maxslen) maxslen = slen2;
    double avslen = ((double)slen1+slen2)/2.0;
    
    int delta;
    int sim_thr_pred_pos[maxslen];
    //int sim_thr_succ_pos[maxslen];
    int scores[maxslen];
    long score_sum[maxslen];
    long s_sum;
    int old_thr_pos;
    
    //double *dyn_weight = calloc(maxslen, sizeof(double));
    double weight;
    struct diag **dyn_diags=malloc(sizeof(struct diag *)*maxslen);
    int max_pool = maxslen;
    struct diag **pool_diags=malloc(sizeof(struct diag *)*max_pool);
    int pooled = 0;
    
    int thres_sim_score = para->PROT_SIM_SCORE_THRESHOLD;
    
    int kscore, klen;
    
    char hasAli = (algn!=NULL);
    
    double diag_thres = para->DIAG_THRESHOLD_WEIGHT;
    double avg_sim_score = pdist->smatrix->avg_sim_score;
    
    struct algn_pos **ap,*tap;
    if(hasAli) {
        ap = algn->algn;
        thres_sim_score =thres_sim_score/2;
        if(thres_sim_score<4) thres_sim_score = 4;
        diag_thres = 0.0;
    }
    
    
    for(d=-slen1+1;d<slen2;d++) {
        //printf(" d=%i\n",d);
        
        if(d<=0) {
            i = - d;
            j=0;
            maxl = slen1-i;
            if(slen2<maxl) maxl = slen2;
        } else {
            i=0;
            j=d;
            maxl = slen2 -j;
            if(slen1<maxl) maxl = slen1;
        }
        
        // prepare values
        old_thr_pos=-1;
        s_sum=0;
        for(k=0;k<maxl;k++) {
            // hier: hasAlipredF/SuccF abfragen !!!!
            if(hasAli) {
                a1 = i+k;
                a2 = j+k;
                //printf("pre %i\n", a1);
                tap = find_eqc(ap, sn1, a1);
                //printf("after %i\n", a1);
                if(tap->predF!=NULL) {
                    if( (tap->predF[sn2] - a2)>0) break;
                }
                if(tap->succF!=NULL) {
                    if((a2 - tap->succF[sn2])>0) break;
                }
            }
            
            a1 = c2n[(int) data1[i+k]];
            a2 = c2n[(int) data2[j+k]];
            score1 = sdata[slen*a1+a2];
            scores[k] = score1;
            s_sum+= score1;
            score_sum[k] = s_sum;
            if(score1 < thres_sim_score) {
                sim_thr_pred_pos[k] = old_thr_pos;
            } else {
                //if(old_thr_pos>=0) sim_thr_succ_pos[old_thr_pos] = k;
                old_thr_pos = k;
            }
        }
        maxl = k;
        //if(old_thr_pos>=0) sim_thr_succ_pos[old_thr_pos] = maxl;
        
        dyn_diags[0] = create_diag(seq1->num, seq1,0,
                                   seq2->num, seq2,0,0);
        dyn_diags[0]->weight_sum = 0.0;
        dyn_diags[0]->weight = 0.0;
        pool_diags[0] = dyn_diags[0];
        pooled = 1;
        lastk=0;
        
        for(k=0;k<maxl;k++) {
            //printf("process %i %i\n", k,maxl);
            
            if(k>0) {
                dyn_diags[k] = dyn_diags[lastk];
                //dyn_weight[k] = dyn_weight[lastk];
            }
            if(hasAli) {
                a1 = i+k;
                a2 = j+k;
                //printf("pre %i\n", a1);
                tap = find_eqc(ap, sn1, a1);
                //printf("after %i\n", a1);
                if(tap->predF!=NULL) {
                    if( (tap->predF[sn2] - a2)>0) break;
                }
                if(tap->succF!=NULL) {
                    if((a2 - tap->succF[sn2])>0) break;
                }
            }
            
            score1 = scores[k];
            if(score1>=thres_sim_score) {
                
                for(l=para->DIAG_MIN_LENGTH;l<=max_dlen; l++) {
                    delta = k-l+1;
                    dg->seq_p1.startpos = i+delta;
                    dg->seq_p2.startpos = j+delta;
                    
                    kscore = 0;
                    klen = 0;
                    
                    if((dg->seq_p1.startpos<0) || (dg->seq_p2.startpos<0)) {
                        break;
                    } else {
                        
                        
                        dg->length = l;
                        //printf("%i %i \n", i-l+1, j-l+1);
                        
                        score2 = scores[delta];
                        klen++;
                        kscore += score2;
                        
                        if( (kscore < avg_sim_score*klen)) { // && (dg->score>1.2*k*avg_sim_score)) {
                            
                            /* experiment */
                            if( ( (k>=0.2*avslen) || (k>=20) || ( (i-j)>0.3*avslen)) && klen>=4) break;
                        }
                        
                        if(kscore >= avg_sim_score*klen ) {
                            //if(score2 >= thres_sim_score) {
                            kscore = 0;
                            klen = 0;
                            dg->score = score_sum[k] - (delta > 0 ? score_sum[delta-1] : 0);
                            if(dg->score <= avg_sim_score*dg->length) break;
                            if(!hasAli) {
                                dg->weight = tmp_dist[dg->length][dg->score];
                                dg->meetsThreshold = (dg->weight>diag_thres ? 1 : 0);
                            } else {
                                real_calc_weight(dg, smatrix, pdist,1,tmp_dist,algn);
                            }
                            
                            if(dg->meetsThreshold) {
                                if(max_pool<=pooled) {
                                    max_pool += maxslen;
                                    pool_diags = realloc(pool_diags, sizeof(struct diag*)*max_pool);
                                }
                                if(delta==0) {
                                    if(dg->weight > dyn_diags[k]->weight_sum) {
                                        dg->weight_sum = dg->weight;
                                        dyn_diags[k] = malloc(sizeof(struct diag));
                                        pool_diags[pooled] = dyn_diags[k];
                                        pooled++;
                                        *dyn_diags[k] = *dg;
                                        dyn_diags[k]->pred_diag = NULL;
                                    }
                                } else {
                                    weight = dg->weight + dyn_diags[delta-1]->weight_sum;
                                    if( (weight) >= dyn_diags[k]->weight_sum) {
                                        dg->weight_sum = weight;
                                        dyn_diags[k] = malloc(sizeof(struct diag));
                                        pool_diags[pooled] = dyn_diags[k];
                                        pooled++;
                                        *dyn_diags[k] = *dg;
                                        if(dyn_diags[delta-1]->weight >0) {
                                            dyn_diags[k]->pred_diag = dyn_diags[delta-1];
                                        } else {
                                            dyn_diags[k]->pred_diag = NULL;
                                        }
                                    }
                                }
                                
                                lastk = k;
                            }
                            
                        } else {
                            l += (delta - sim_thr_pred_pos[delta])-1;
                        }
                    }
                }
            }
        }
        tdg = dyn_diags[lastk];
        while((tdg!=NULL)) {
            if (tdg->weight <=0.0) break;
            
            data[length] = tdg;
            tdg->weight_sum = -1.0;
            length++;
            if(length >= size) {
                size += 64;
                data = realloc(data, sizeof(struct diag *)*size);
                if(data==NULL) error("find_diags(): (3) Out of memory !");
            }
            tdg = tdg->pred_diag;
        }
        
        
        for(k=0;k<pooled;k++) {
            if(pool_diags[k]->weight_sum>-1.0) free(pool_diags[k]);
        }
    }
    
    data = realloc(data, sizeof(struct diag *)*length);
    free(pool_diags);
    free(dyn_diags);
    free_diag(dg);
    dcol->length = length;
    dcol->data = data;
    
    if(para->DEBUG>5) {
        for(i=0;i<length;i++) {
            print_diag(data[i]);
            printf("\n");
        }
    }
    
    return dcol;
}

/**
 * builds the upgma guide tree in the given diag_col
 */
void build_guide_tree(struct diag_col *dcol)
{
    int slen = dcol->seq_amount;
    double weights[slen][slen];
    struct gt_node *nodes[slen];
    struct gt_node *gtn, *gtn1, *gtn2;
    int max1=0, max2=1;
    
    int i,j,k;
    
    for(i=0;i<slen;i++) {
        gtn = malloc(sizeof(struct gt_node ));
        gtn->isLeaf = 1;
        gtn->seq_num = malloc(sizeof(int)*1);
        gtn->seq_num[0] = i;
        gtn->seq_num_length = 1;
        gtn->succ1 = NULL;
        gtn->succ2 = NULL;
        nodes[i] = gtn;
        for(j=i+1;j<slen;j++) {
            weights[i][j] = dcol->diag_matrix[slen*i+j]->total_weight;
            weights[j][i] = weights[i][j];
            if(weights[i][j] > weights[max1][max2]) {
                max1 = i;
                max2 = j;
            }
        }
    }
    
    for(k=0;k<(slen-1);k++) {
        gtn1 = nodes[max1];
        gtn2 = nodes[max2];
        gtn = malloc(sizeof(struct gt_node ));
        gtn->isLeaf = 0;
        gtn->seq_num_length = gtn1->seq_num_length + gtn2->seq_num_length;
        gtn->seq_num = malloc(sizeof(int)*gtn->seq_num_length);
        
        for(i=0;i<gtn1->seq_num_length;i++) {
            gtn->seq_num[i] = gtn1->seq_num[i];
        }
        for(i=0;i<gtn2->seq_num_length;i++) {
            gtn->seq_num[gtn1->seq_num_length+i] = gtn2->seq_num[i];
        }
        
        gtn->succ1 = gtn1;
        gtn->succ2 = gtn2;
        nodes[max1] = gtn;
        nodes[max2] = NULL;
        for(i=0;i<slen;i++) {
            if( (i!=max1) && (i!=max2)) {
                weights[i][max1] = 0.1*0.5*(weights[i][max1]+weights[i][max2]) + 0.9*
                ( (weights[i][max1] > weights[i][max2]) ? weights[i][max1] : weights[i][max2]);
                //weights[i][max1] = 1.0*(weights[i][max1]+weights[i][max2]) + 0.0*                          ( (weights[i][max1] > weights[i][max2]) ? weights[i][max1] : weights[i][max2]);
                weights[max1][i] = weights[i][max1];
            }
        }
        max1 = -1;
        max2 = -1;
        for(i=0;i<slen;i++) {
            for(j=i+1;j<slen;j++) {
                if( (nodes[i]!=NULL) && (nodes[j]!=NULL)) {
                    if( (max1<0) || (weights[max1][max2]< weights[i][j])) {
                        max1=i;
                        max2=j;
                    }
                }
            }
        }
    }
    
    dcol->gt_root = nodes[0];
    /*
     printf(" root %i\n", nodes[0]);
     printf(" left1 %i\n", nodes[0]->succ1->succ1->seq_num);
     printf(" left2 %i\n", nodes[0]->succ1->succ2->seq_num);
     printf(" right %i\n", nodes[0]->succ2->seq_num);
     */
}


/**
 * Finds all diags of each pair of sequences in in_seq_col by using
 * the function above
 *
 * The pointer returned (and the ones included in the struct)
 * has to be deallocted explicitely from memory.
 */
//void find_all_diags(struct scr_matrix *smatrix, struct prob_dist *pdist, struct seq_col *in_seq_col, struct alignment *algn, int round)
struct diag_col *find_all_diags(struct scr_matrix *smatrix, struct prob_dist *pdist, struct seq_col *in_seq_col, struct alignment *algn, int round)
{
    unsigned int s1, s2, rs2, sl = in_seq_col->length, sp, ap;
    struct diag_col *all_diags = create_diag_col(sl);
    struct simple_diag_col *sdcol;
    
    unsigned int diag_amount = 0;
    struct diag *dg;
    
    char hasAli = (round >1);//(algn!=NULL);
    
    long double **tmp_dist = NULL;
    if(!hasAli) tmp_dist = create_tmp_pdist(pdist);
    
    int s2max = sl;
    int s2width =(int) sqrt(sl);
    
    double total=0.0;
    //double imp[sl];
    //  for(s1=0;s1<sl;s1++) {
    //  imp[s1] = 0.0;
    //}
    double totala[sl];
    memset(totala,0,sizeof(double)*sl);
    for(s1=0;s1<sl;s1++) {
        if(para->FAST_PAIRWISE_ALIGNMENT && s2width+1<sl) {
            s2max = s1+s2width+1;
            //printf("%i %i\n",s1, s2max);
        }
        //printf("before enter %i\n", s1);
        for(s2=s1+1;s2<s2max;s2++) {
            rs2 = s2 % sl;
            if(!hasAli) {
                fill_tmp_pdist(pdist,tmp_dist,in_seq_col->seqs[s1].length,in_seq_col->seqs[rs2].length );
            }
            if(para->DEBUG>5) printf("%i %i\n", s1,s2);
            //time1 = clock();
            //sdcol=find_diags_dyn(smatrix, pdist, &in_seq_col->seqs[s1],
            //		 &in_seq_col->seqs[s2],algn,tmp_dist);
            
            /*
             doAlign = 1;
             if(hasAli) {
             if(algn->redo_seqs[s1*sl+s2]==0)
             doAlign = 0;
             }
             
             if(doAlign) {
             */
            if(in_seq_col->seqs[s1].length > 0 && in_seq_col->seqs[s2].length > 0) {
                //if(para->DEBUG>1) printf(" %i %i %i\n", s1, rs2,sl-1);
                //	printf("find diags %i %i\n",s1,s2);
                sdcol=find_diags_dialign(smatrix, pdist, &in_seq_col->seqs[s1],
                                         &in_seq_col->seqs[rs2],algn,tmp_dist, round);
                //	imp[s1] += sdcol->total_weight;
                //imp[s2] += sdcol->total_weight;
                total += sdcol->total_weight;
                //totala[s1] +=sdcol->total_weight;
                //totala[s2] +=sdcol->total_weight;
                //printf(" num of diags:%i\n ", sdcol->length);
                /*
                 } else {
                 sdcol = calloc(1, sizeof(struct simple_diag_col));
                 sdcol->length = 0;
                 }
                 */
                //printf("%i %i %f\n", s1,s2, (clock()-time1)/CLOCKS_PER_SEC);
                
                all_diags->diag_matrix[s1+sl*rs2] = sdcol;
                all_diags->diag_matrix[rs2+sl*s1] = sdcol;
                diag_amount += sdcol->length;
            }
        }
    }
    if(!hasAli) free_tmp_pdist(tmp_dist, pdist->max_dlen);
    all_diags->diags= calloc(diag_amount, sizeof(struct diag*));
    if(all_diags->diags==NULL) error("find_all_diags(): (1) Out of memory !");
    
    ap=0;
    for(s1=0;s1<sl;s1++) {
        for(s2=s1+1;s2<sl;s2++) {
            sdcol=all_diags->diag_matrix[s1+sl*s2];
            if(sdcol!=NULL) {
                for(sp=0;sp<sdcol->length;sp++) {
                    //	  if(hasAli || (sdcol->data[sp]->weight >0.01*(sdcol->total_weight/sdcol->length))) {
                    sdcol->data[sp]->pred_diag = NULL;
                    all_diags->diags[ap]=sdcol->data[sp];
                    ap++;
                    //} else {
                    //  free(sdcol->data[sp]);
                    //diag_amount--;
                    // }
                }
            }
        }
    }
    
    all_diags->seq_amount = sl;
    
    for(s1=0;s1<sl;s1++) {
        for(s2=s1+1;s2<sl;s2++) {
            sdcol=all_diags->diag_matrix[sl*s1+s2];
            if(sdcol!=NULL) {
                sdcol->weight_fac =pow(sdcol->total_weight/total,2.0);
                for(sp=0;sp<sdcol->length;sp++) {
                    dg = sdcol->data[sp];
                    //	  if(hasAli) print_diag(dg);
                    dg->weight_fac = sdcol->weight_fac;
                    //	  dg->ov_weight = sdcol->total_weight;
                    /*
                     if(1 || !hasAli) {
                     dg->weight_fac = sdcol->total_weight*totala[s1]/(sl-1)*sdcol->total_weight*totala[s2]/(sl-1);
                     }
                     */
                    //dg->weight_fac =pow(sdcol->total_weight*(sl-1),2.0)/(totala[s1]*totala[s2]);
                    
                    if(!hasAli) {
                        if(para->DO_OVERLAP) {
                            //printf(" do overlap\n");
                            //dg->weight_fac = 1.0;
                            calc_ov_weight(dg,all_diags, smatrix,pdist);
                        }
                        dg->total_weight = (dg->weight);//+dg->ov_weight);// *dg->weight_fac;
                    } else {
                        // changed in TX 1.0.0
                        if(para->FAST_MODE) dg->weight_fac = 1.0;
                        dg->total_weight = (dg->weight);//+dg->ov_weight);// *dg->weight_fac;
                    }
                    //if(sp==sdcol->length-1) sdcol->total_weight *= dg->weight_fac;
                }
            }
        }
    }
    
    all_diags->diag_amount = diag_amount;
    
    if(! hasAli) build_guide_tree(all_diags);
    return all_diags;
}



/**
 * Finds all diags of each pair of sequences in in_seq_col by using
 * the function above
 *
 * The pointer returned (and the ones included in the struct)
 * has to be deallocted explicitely from memory.
 */
struct diag_col *old_find_all_diags(struct scr_matrix *smatrix, struct prob_dist *pdist, struct seq_col *in_seq_col, struct alignment *algn, int round )
{
    unsigned int s1, s2, rs2, sl = in_seq_col->length, sp, ap;
    struct diag_col *all_diags = create_diag_col(sl);
    struct simple_diag_col *sdcol;
    
    unsigned int diag_amount = 0;
    struct diag *dg;
    
    char hasAli = (round >1);//(algn!=NULL);
    
    long double **tmp_dist = NULL;
    if(!hasAli) tmp_dist = create_tmp_pdist(pdist);
    
    int s2max = sl;
    int s2width =(int) sqrt(sl);
    
    double total=0.0;
    //double imp[sl];
    //  for(s1=0;s1<sl;s1++) {
    //  imp[s1] = 0.0;
    //}
    //double totala[sl];
    //memset(totala,0,sizeof(double)*sl);
    for(s1=0;s1<sl;s1++) {
        if(para->FAST_PAIRWISE_ALIGNMENT && s2width+1<sl) {
            s2max = s1+s2width+1;
            //printf("%i %i\n",s1, s2max);
        }
        //printf("before enter %i\n", s1);
        for(s2=s1+1;s2<s2max;s2++) {
            rs2 = s2 % sl;
            if(!hasAli) {
                fill_tmp_pdist(pdist,tmp_dist,in_seq_col->seqs[s1].length,in_seq_col->seqs[rs2].length );
            }
            if(para->DEBUG>5) printf("%i %i\n", s1,s2);
            //time1 = clock();
            //sdcol=find_diags_dyn(smatrix, pdist, &in_seq_col->seqs[s1],
            //		 &in_seq_col->seqs[s2],algn,tmp_dist);
            
            /*
             doAlign = 1;
             if(hasAli) {
             if(algn->redo_seqs[s1*sl+s2]==0)
             doAlign = 0;
             }
             
             if(doAlign) {
             */
            if(in_seq_col->seqs[s1].length > 0 && in_seq_col->seqs[s2].length > 0) {
                //if(para->DEBUG>1) printf(" %i %i %i\n", s1, rs2,sl-1);
                //	printf("find diags %i %i\n",s1,s2);
                sdcol=find_diags_dialign(smatrix, pdist, &in_seq_col->seqs[s1],
                                         &in_seq_col->seqs[rs2],algn,tmp_dist, round);
                //	imp[s1] += sdcol->total_weight;
                //imp[s2] += sdcol->total_weight;
                total += sdcol->total_weight;
                //totala[s1] +=sdcol->total_weight;
                //totala[s2] +=sdcol->total_weight;
                //printf(" num of diags:%i\n ", sdcol->length);
                /*
                 } else {
                 sdcol = calloc(1, sizeof(struct simple_diag_col));
                 sdcol->length = 0;
                 }
                 */
                //printf("%i %i %f\n", s1,s2, (clock()-time1)/CLOCKS_PER_SEC);
                
                all_diags->diag_matrix[s1+sl*rs2] = sdcol;
                all_diags->diag_matrix[rs2+sl*s1] = sdcol;
                diag_amount += sdcol->length;
            }
        }
    }
    
    
    // new: call of split function
    //printf("before split\n");
    /*
     if(! hasAli) {
     diag_amount = 0;
     split_diags(in_seq_col, all_diags);
     //total=0.0;
     for(s1=0;s1<sl;s1++) {
     for(s2=s1+1;s2<sl;s2++) {
     sdcol=all_diags->diag_matrix[s1+sl*s2];
     if(sdcol!=NULL) {
     //sdcol->total_weight = 0.0;
     for(sp=0;sp<sdcol->length;sp++) {
     dg = sdcol->data[sp];
     real_calc_weight(dg, smatrix, pdist, 0, NULL,algn);
     //sdcol->total_weight += dg->weight;
     //total += dg->weight;
     }
     diag_amount +=sdcol->length;
     }
     }
     }
     }
     */
    //all_diags->diag_amount = diag_amount;
    //  printf("after split\n");
    //double max = 0.0;
    
    
    all_diags->diags= calloc(diag_amount, sizeof(struct diag*));
    if(all_diags->diags==NULL) error("find_all_diags(): (1) Out of memory !");
    
    double sdtotal;
    
    ap=0;
    for(s1=0;s1<sl;s1++) {
        for(s2=s1+1;s2<sl;s2++) {
            sdcol=all_diags->diag_matrix[s1+sl*s2];
            if(sdcol!=NULL) {
                for(sp=0;sp<sdcol->length;sp++) {
                    //	  if(hasAli || (sdcol->data[sp]->weight >0.01*(sdcol->total_weight/sdcol->length))) {
                    all_diags->diags[ap]=sdcol->data[sp];
                    ap++;
                    //} else {
                    //  free(sdcol->data[sp]);
                    //diag_amount--;
                    // }
                }
            }
        }
    }
    
    all_diags->seq_amount = sl;
    
    all_diags->total_weight = 0.0;
    for(s1=0;s1<sl;s1++) {
        for(s2=s1+1;s2<sl;s2++) {
            sdcol=all_diags->diag_matrix[sl*s1+s2];
            sdcol->total_weight = 0.0;
            sdtotal = 0.0;
            if(sdcol!=NULL) {
                for(sp=0;sp<sdcol->length;sp++) {
                    dg = sdcol->data[sp];
                    //	  if(hasAli) print_diag(dg);
                    dg->weight_fac = pow(sdcol->total_weight/total,2.0);
                    //dg->weight_fac = 1.0;
                    
                    //	  dg->ov_weight = sdcol->total_weight;
                    /*
                     if(1 || !hasAli) {
                     dg->weight_fac = sdcol->total_weight*totala[s1]/(sl-1)*sdcol->total_weight*totala[s2]/(sl-1);
                     }
                     */
                    //	  dg->weight_fac =pow(sdcol->total_weight*(sl-1),2.0)/(totala[s1]*totala[s2]);
                    
                    if(!hasAli) {
                        if(para->DO_OVERLAP) {
                            //dg->weight_fac = 1.0;
                            //error("calc ov\n");
                            calc_ov_weight(dg,all_diags, smatrix,pdist);
                        }
                        //dg->total_weight = (dg->weight+dg->ov_weight) *dg->weight_fac;
                    } else {
                        dg->weight_fac = 1.0;
                    }
                    dg->total_weight = (dg->weight+dg->ov_weight);//*dg->weight_fac;
                    //if(dg->total_weight > max) max = dg->total_weight;
                    //dg->weight = dg->score;
                    //	  print_diag(dg);
                    sdtotal += dg->weight;
                    all_diags->total_weight += dg->total_weight;
                }
                sdcol->total_weight = sdtotal;
            }
        }
    }
    
    all_diags->average_weight = all_diags->total_weight / all_diags->diag_amount;
    //printf(" max %f\n", max);
    if(!hasAli) free_tmp_pdist(tmp_dist, pdist->max_dlen);
    all_diags->diag_amount = diag_amount;
    
    if(! hasAli) build_guide_tree(all_diags);
    return all_diags;
}

#pragma mark -
#pragma mark alig

////////////////////////////////////////////////////////////////////////////////
// alig.c
////////////////////////////////////////////////////////////////////////////////

//long sslen;

struct alignment* create_empty_alignment(struct seq_col *scol)
{
    /*
     printf("before\n");
     sleep(5);
     */
    struct alignment* algn = malloc(sizeof(struct alignment));
//    sslen = scol->length;
    //allocss += sizeof(struct alignment);
    if(algn==NULL) error("create_empty_alignment(): (1) Out of memory !");
    
    
    //long xsize = sizeof(struct alignment);
    
    algn->next = NULL;
    //  algn->prev = NULL;
    algn->total_weight = 0.0;
    //algn->pos = 0;
    algn->max_pos = -1;
    algn->scol = scol;
    //algn->aligned_diags_amount=0;
    //algn->aligned_diags = malloc(sizeof(struct diag*)*diag_amount);
    //algn->max_aligned_diags_amount = diag_amount;
    //algn->orig_max_aligned_diags_amount = diag_amount;
    //algn->backlog_diags = NULL;
    ////allocss += (sizeof(struct diag*)*diag_amount);
    
    //if(algn->aligned_diags==NULL)
    //  error("create_empty_alignment(): (1.5) Out of memory !");
    
    
    unsigned int slen = scol->length;
    algn->seq_is_orphane = calloc(slen, sizeof(char));
    //allocss += (sizeof(char)*slen);
    
    //xsize += slen*sizeof(char);
    
    if(algn->seq_is_orphane==NULL) error("create_empty_alignment(): (2) Out of memory !");
    //  memset(algn->seq_is_orphane, 1, slen*sizeof(char));
    
    algn->algn = malloc(sizeof(struct algn_pos *)*slen);
    //allocss += sizeof(struct algn_pos *)*slen;
    
    //xsize += slen*sizeof(struct algn_pos *);
    
    if(algn->algn==NULL) error("create_empty_alignment(): (3) Out of memory !");
    //algn->redo_seqs = calloc(slen*slen, sizeof(char));
    int i,j;
    struct seq* sq;
    for(i=0;i<slen;i++)
    {
        sq = &(scol->seqs[i]);
        algn->seq_is_orphane[i]=1;
        algn->algn[i] = malloc(sizeof(struct algn_pos)*sq->length );
        //allocss += sizeof(struct algn_pos )*sq->length;
        //xsize += sq->length*sizeof(struct algn_pos *);
        
        if(algn->algn[i]==NULL) error("create_empty_alignment(): (4) Out of memory !");
        
        for(j=0;j<sq->length;j++) {
            algn->algn[i][j].state = para->STATE_ORPHANE;
            //      algn->algn[i][j].isInherited = 0;
            algn->algn[i][j].predFPos = -1;
            algn->algn[i][j].succFPos = -1;
            
            algn->algn[i][j].eqcParent= &(algn->algn[i][j]);
            //if(j==442) printf(" parent: %i\n", algn->algn[i][j].eqcParent);
            algn->algn[i][j].eqcRank= 0;
            algn->algn[i][j].eqcAlgnPos=calloc(1, sizeof(int));;
            //allocss += sizeof(int);
            *algn->algn[i][j].eqcAlgnPos=j;
            algn->algn[i][j].proceed=calloc(1, sizeof(char));;
            //allocss += sizeof(char);
            *algn->algn[i][j].proceed = 0;
            
            algn->algn[i][j].predF = NULL;
            algn->algn[i][j].succF = NULL;
            
            algn->algn[i][j].row = i;
            algn->algn[i][j].col = j;
            algn->algn[i][j].dg_cont = NULL;
            //xsize += sizeof(char) + sizeof(int);
        }
    }
    /*
     printf("after\n");
     sleep(5);
     printf("gone\n");
     */
    //  printf(" algnsize=%i\n",xsize);
    return algn;
}


/**
 * free alignment
 *
 */
void free_alignment(struct alignment *algn)
{
    struct seq_col *scol = algn->scol;
    int slen = scol->length;
    int i,j;
    struct algn_pos *apos;
//    struct algn_pos *o_apos;
//    struct algn_pos *tpos;
    struct seq *sq;
    struct diag_cont *dgc,*ndgc;
    
    if(algn->seq_is_orphane!=NULL) {
        free(algn->seq_is_orphane);
        //freess += sizeof(char)*slen;
    }
    for(i=0;i<slen;i++) {
        sq = &(scol->seqs[i]);
        for(j=0;j<sq->length;j++) {
            apos = &(algn->algn[i][j]);
            if(! (apos->state & para->STATE_INHERITED) && ! (apos->state & para->STATE_ORPHANE)) {
                //if(! (apos->state & para->STATE_INHERITED) && ! (apos->state & para->STATE_ORPHANE)) {
                if(apos->predF!=NULL) {
                    free(apos->predF);
                    //freess += sizeof(int)*slen;
                }
                if(apos->succF!=NULL) {
                    free(apos->succF);
                    //freess += sizeof(int)*slen;
                }
                
            }
            if(! (apos->state & para->STATE_INHERITED)  || (apos->state & para->STATE_ORPHANE) ) {
                free(apos->eqcAlgnPos);
                free(apos->proceed);
                //freess += sizeof(int)+sizeof(char);
            }
            dgc = apos->dg_cont;
            while(dgc!=NULL) {
                ndgc = dgc->next;
                //printf(" free %i %i %i %i\n", i,j,dgc, dgc->next);
                free(dgc);
                //freess += sizeof(struct diag_cont);
                dgc = ndgc;
            }
            
        }
        free(algn->algn[i]);
        //freess += sizeof(struct algn_pos)*sq->length;
    }
    //dgc = algn->backlog_diags;
    //while(dgc!=NULL) {
    //  ndgc = dgc->next;
    //  free(dgc);
    //  //freess += sizeof(struct diag_cont);
    //  dgc = ndgc;
    //}
    
    
    
    //free(algn->aligned_diags);
    ////freess += sizeof(struct diag *)*algn->;
    free(algn->algn);
    free(algn);
    ////freess += sizeof(struct algn_pos *)*slen + sizeof(struct alignment);
}

/**
 * returnes the representative of the equivalence class
 */
struct algn_pos *_find_eqc(struct algn_pos *ap)
{
    if(ap!=ap->eqcParent) {
        //    if(doprint) printf("    FIND: %i %i\n", ap, ap->eqcParent);
        
        /**
         if(ap->eqcParent->eqcAlgnPos!=NULL) {
         if( (ap->eqcAlgnPos!=NULL) && *ap->eqcParent->eqcAlgnPos < *ap->eqcAlgnPos) {
         //	if(doprint)  printf("    1.1 ALGNPOS: %i %i\n", ap, ap->eqcParent);
         *ap->eqcParent->eqcAlgnPos = *ap->eqcAlgnPos;
         }
         } else {
         //	ap->eqcParent->eqcAlgnPos = ap->eqcAlgnPos;
         }
         */
        ap->eqcParent = _find_eqc(ap->eqcParent);
    }
    //  if(doprint)   printf("    1.ALGNPOS: %i %i\n", ap, ap->eqcParent);
    //  if(doprint)   printf("    2.ALGNPOS: %i %i\n", ap, ap->eqcParent);
    return ap->eqcParent;
}

/**
 * returnes the representative of the equivalence class
 */
struct algn_pos *find_eqc(struct algn_pos **ap, int seqnum, int pos)
{
    //if(1) printf("%i %i %i\n", ap, seqnum,pos);
    struct algn_pos *tap = &ap[seqnum][pos];
    struct algn_pos *eq_ap;
    eq_ap = _find_eqc(tap);
    struct diag_cont *old_dgc;
    
    //  if(eq_ap->eqcAlgnPos != tap->eqcAlgnPos) {
    if(eq_ap != tap) {
        
        /*
         if(tap->state & para->STATE_ORPHANE) {
         printf(" ALARM ORPHANE %i %i\n", eq_ap->state, tap->state);
         }
         if(eq_ap->state & para->STATE_ORPHANE) {
         printf("    ALARM ORPHANE %i %i\n", eq_ap->state, tap->state);
         }
         */
        //if((tap->eqcAlgnPos!=NULL) && (*tap->eqcAlgnPos > *eq_ap->eqcAlgnPos))
        //  *eq_ap->eqcAlgnPos = *tap->eqcAlgnPos;
        
        //    if(eq_ap->eqcAlgnPos != tap->eqcAlgnPos)
        if( (!(tap->state & para->STATE_INHERITED))) { //&& !oldparentIsInherited) {
            if(tap->eqcAlgnPos !=NULL) {
                //if(pos==175) printf("free eqcAlgnPos: %i\n", tap->eqcAlgnPos);
                //balance -= sizeof(int);
                free(tap->eqcAlgnPos);
                tap->eqcAlgnPos = NULL;
                //freess += sizeof(int);
            }
            
            if(tap->proceed !=NULL) {
                //printf("free proceed: %i\n", tap->proceed);
                //balance -= sizeof(char);
                free(tap->proceed);
                //freess += sizeof(char);
                //printf("after free proceed: %i\n", tap->proceed);
                tap->proceed = NULL;
            }
            
            //if(tap->predFPos>=0)
            if( (eq_ap->predF != tap->predF) && (1 || (tap->predFPos<0)) && (tap->predF!=NULL)){
                //printf("          free predF: %i %i %i %i\n", tap->predF, tap->isInherited, seqnum, pos);
                //balance -= sizeof(int)*sslen;
                //printf (" 3. free: %i %i %i\n",allocs, frees, allocs-frees);
                //freess += sizeof(int)*sslen;
                free(tap->predF);
                //printf("          after free predF: %i\n", tap->predF);
                tap->predF=NULL;
            }
            //if(tap->succFPos>=0)
            if((eq_ap->succF != tap->succF) && (1 || (tap->succFPos<0)) && (tap->succF!=NULL)) {
                //printf("free succ: %i\n", tap->succF);
                //balance -= sizeof(int)*sslen;
                //printf (" 4. free: %i %i %i\n",allocs, frees, allocs-frees);
                //freess += sizeof(int)*sslen;
                free(tap->succF);
                //printf("after free succ: %i\n", tap->succF);
                tap->succF=NULL;
            }
        } /*else {
           if(eq_ap->state & para->STATE_INHERITED) {
           printf(" inherited alarm!\n");
           }
           if(eq_ap->state & para->STATE_ORPHANE) {
           printf(" orphane alarm!\n");
           }
           }*/
        old_dgc = tap->dg_cont;
        *tap = *eq_ap;
        tap->dg_cont = old_dgc;
        tap->row = seqnum;
        tap->col = pos;
        tap->state = (tap->state | para->STATE_INHERITED);
    }
    //if(seqnum==0 &&pos==175)  printf("after !=\n");
    // tap = eq_ap;
    
    struct algn_pos *ttap;
    if(tap->predFPos>=0 ) {
        //if(seqnum==0 && pos==175) printf("          alarm predF: %i %i %i %i\n", tap->predF, tap->predFPos, seqnum, pos);
        //    printf ("PRE Pos %i %i \n", tap->predFPos, pos);
        if( (tap->predFPos==pos)|| !(tap->state & para->STATE_ORPHANE)) {
            printf("pred ALARM %i %i\n", tap->predFPos, pos);
            exit(99);
        }
        ttap=find_eqc(ap, seqnum, tap->predFPos);
        tap->predF = ttap->predF;
    }
    if(tap->succFPos>=0) {
        //if(seqnum==0 && pos==175) printf("          alarm succF: %i %i %i %i\n", tap->succF, tap->succFPos, seqnum, pos);
        //printf ("2. PRE Pos %i %i \n", tap->predFPos, tap->succFPos);
        if( (tap->succFPos==pos)|| !(tap->state & para->STATE_ORPHANE)) {
            printf("succ ALARM %i %i\n", tap->succFPos, pos);
            exit(99);
        }
        ttap = find_eqc(ap, seqnum, tap->succFPos);
        tap->succF = ttap->succF;
    }
    //if(seqnum==0 && pos==175) printf(" end qgc\n");
    return tap;
}

/**
 * adds the given diagional to the given alignment and updates the
 * datastructure (i.e. frontiers). The given diag must be consistent
 * to the given alignment !
 */
char align_diag(struct alignment *algn, struct scr_matrix *smatrix, struct diag* dg)
{
    
    char alignedSomething = 0;
    int i,j,k;
    char al;
    if(dg->multi_dg) {
        //return 0;
        for(i=0;i<dg->multi_length;i++) {
            if(dg->multi_cont[i]!=NULL) {
                al = align_diag(algn,smatrix, dg->multi_cont[i]);
                if(al) algn->total_weight -= dg->multi_cont[i]->total_weight;
                alignedSomething = alignedSomething || al;
                
            }
        }
        return alignedSomething;
    }
    
    if((dg->length==0) || (!dg->meetsThreshold && !dg->anchor)) return 0;
    
    struct seq_col *scol = algn->scol;
    int s1 = dg->seq_p1.num;
    int s2 = dg->seq_p2.num;
    
    
    int length = dg->length;
    int sp1 = dg->seq_p1.startpos;
    int sp2 = dg->seq_p2.startpos;
    
    struct algn_pos **ap=algn->algn;
    struct algn_pos *tp;
    
    struct algn_pos *apos1, *apos2, *tpos;
    
    int p1, p2,plen;
    int p;
    struct seq *sq = scol->seqs;
    int s, slen = scol->length;
    int *oldpredF, *oldsuccF, *otherpredF, *othersuccF;
    int pF,sF;
    /*
     int *c2n = smatrix->char2num;
     int *sdata = smatrix ->data;
     char *data1 = dg->seq_p1.sq->data;
     char *data2 = dg->seq_p2.sq->data;
     int smatrixlen = smatrix->length;
     int a1,a2;
     int score1;
     */
    char seenNonOrphane;
    int opos;
    int ms;
    char skip = 0;
    char found;
    
    /*
     struct diag_cont *dgc, *dgc_next;
     double ttim;
     int nextpos[slen];
     int firstpos;
     int oldpos;
     char firstRound;
     */
    for(i=0;i<length;i++) {
        p1 = sp1+i; p2 = sp2+i;
        //    printf("pos %i %i %i %i\n",s1,s2,p1,p2);
        //printf(" %i \n", scol->length);
        skip = 0;
        
        //if(sp1==30) printf("%i %i %i %i %i \n", p1,p2,dg->length, dg->seq_p1.sq->length, dg->seq_p2.sq->length);
        /*
         //    if(dg->onlyOverThres==1) {
         a1 = c2n[data1[p1]];
         a2 = c2n[data2[p2]];
         score1 = sdata[smatrixlen*a1+a2];
         
         if( (score1<=3) && (i>length/3)) {
         skip = 1;
         }
         //}
         */
        //    printf(" %i %i %i\n",p1,p2,skip);
        //} else {
        /*
         a1 = c2n[data1[p1]];
         a2 = c2n[data2[p2]];
         score1 = sdata[smatrixlen*a1+a2];
         
         if(score1<=4) {
         skip = 1;
         }
         */
        //if( (i==(length-1)) ){
        
        //ttim = clock();
        //tim += (clock()-ttim)/CLOCKS_PER_SEC;
        // printf(" tim %f\n", tim);
        
        
        //printf(" diag n1=%i s1=%i n2=%i s2=%i l=%i\n", s1,sp1,s2,sp2,length);
        //allocss += 2*sizeof(struct diag_cont );
        //}
        // TODO: tune ration is 1:45 (minor)
        /*
         dgc_next = ap[s1][p1].dg_cont;
         dgc = malloc(sizeof(struct diag_cont));
         dgc->dg = dg;
         dgc->next = dgc_next;
         ap[s1][p1].dg_cont = dgc;
         //if((s1==0) && (p1==133)) printf(" dddgc %i %i\n", dgc, dgc->next);
         
         dgc_next = ap[s2][p2].dg_cont;
         dgc = malloc(sizeof(struct diag_cont));
         dgc->dg = dg;
         dgc->next = dgc_next;
         ap[s2][p2].dg_cont = dgc;
         */
        
        
        if(! skip) {
            //printf("apos1 %i %i\n", s1, p1);
            apos1 = find_eqc(ap, s1,p1);//&(ap[s1][p1]);
            //printf("apos2 %i %i %i\n", s2, p2, scol->seqs[s2].length);
            apos2 = find_eqc(ap, s2,p2); //&(ap[s2][p2]);
            //printf("after apos2 %i %i\n", s2, p2);
            
            oldpredF = apos1->predF;
            oldsuccF = apos1->succF;
            if(oldpredF!=NULL && oldsuccF!=NULL)
                if(oldpredF[s2]==p2 && oldsuccF[s2]==p2) skip = 1;
            
            if(para->DEBUG>4) {
                if(!skip) {
                    if(oldpredF!=NULL) if(oldpredF[s2]>=p2) {
                        printf(" Incons1 %i %i %i\n", s2, p2,oldpredF[p2]);
                        error(" ERROR");
                    }
                    if(oldsuccF!=NULL) if(oldsuccF[s2]<=p2) {
                        printf(" Incons2 %i %i %i\n",s2, p2,oldsuccF[p2]);
                        error(" ERROR");
                    }
                    oldpredF=apos2->predF;
                    oldsuccF=apos2->succF;
                    if(oldpredF!=NULL) if(oldpredF[s1]>=p1) {
                        printf(" Incons3 %i %i %i\n", s1, p1,oldpredF[p1]);
                        error(" ERROR");
                    }
                    if(oldsuccF!=NULL) if(oldsuccF[s1]<=p1) {
                        printf(" Incons4 %i %i %i\n",s1, p1,oldsuccF[p1]);
                        error(" ERROR");
                    }
                }
            }
        }
        //}
        
        if(! skip) {
            alignedSomething = 1;
            for(k=0;k<2;k++) {
                tpos = (k==0 ? apos1 : apos2);
                //printf("tpos %i\n", tpos);
                oldpredF = tpos->predF;
                oldsuccF = tpos->succF;
                otherpredF = (k==0 ? apos2->predF : apos1->predF);
                othersuccF = (k==0 ? apos2->succF : apos1->succF);
                //printf("pre isorphane %i\n", tpos);
                if(tpos->state & para->STATE_ORPHANE) {
                    //if(scol->length>21) printf("isorphane %i %i\n", tpos,sizeof(int)*scol->length);
                    //printf("step 1\n");
                    // printf (" 3. malloc: %i\n",sizeof(int)*slen*2);
                    //allocs += sizeof(int)*slen*2;
                    //	  printf (" 3. malloc: %i %i %i\n",allocs, frees, allocs-frees);
                    //balance += sizeof(int)*slen*2;
                    tpos->predF = malloc(sizeof(int)*scol->length);
                    //if(k==0) printf("           apos1->predF = %i\n",tpos->predF),
                    //printf("pre succForphane %i %i\n", tpos->succF, scol->length);
                    //printf(" step 2\n");
                    tpos->succF = malloc(sizeof(int)*scol->length);
                    //allocss += 2*sizeof(int )*scol->length;
                    //printf("  step 3\n");
                    //printf("succForphane %i\n", tpos);
                    if( (tpos->predF==NULL) || (tpos->succF==NULL)) error("align_diag(): (1) Out of memory !");
                    
                }
                //      printf("init loop %i\n", tpos);
                
                for(j=0;j<scol->length;j++) {
                    pF = -1;
                    sF = sq[j].length;
                    
                    
                    // propagate predF and succF
                    if(oldpredF!=NULL && oldpredF[j]>pF) pF = oldpredF[j];
                    //if(j==0) printf("1 pf=%i\n", pF);
                    if(otherpredF!=NULL && otherpredF[j]> pF) pF = otherpredF[j];
                    //if(j==0) printf("2 pf=%i\n", pF);
                    
                    if(oldsuccF!=NULL && oldsuccF[j]<sF) sF = oldsuccF[j];
                    if(othersuccF!=NULL && othersuccF[j]<sF) sF = othersuccF[j];
                    //}
                    if(j==s1) {
                        pF  =  p1;
                        sF  =  p1;
                    } else if(j==s2) {
                        pF  =  p2;
                        sF  =  p2;
                    }
                    /*
                     if(pF > sF) {
                     if(oldpredF!=NULL) printf(" PRE 1. ALARM oldpredF: %i \n",oldpredF[j] );
                     if(oldsuccF!=NULL) printf(" PRE 1. ALARM oldsuccF: %i \n",oldsuccF[j] );
                     if(otherpredF!=NULL) printf(" PRE 1. ALARM otherpredF: %i \n",otherpredF[j] );
                     if(othersuccF!=NULL) printf(" PRE 1. ALARM othersuccF: %i \n",othersuccF[j] );
                     
                     printf("1. ALARM j=%i %i %i %i %i %i %i %i\n",
                     j,
                     (k==0 ? s1 : s2),
                     (k==0 ? p1 : p2),
                     (k==0 ? p2 : p1),
                     oldpredF!=NULL ? oldpredF[k==0 ? s2 : s1] : -2,
                     oldsuccF!=NULL ? oldsuccF[k==0 ? s2 : s1]: 99999,
                     tpos->predFPos,
                     tpos->succFPos);
                     exit(1);
                     }
                     */
                    //if(pF==sF && pF==0 && j==0) printf("pf=%i sf=%i j=%i s1=%i s2=%i p1=%i p2=%i iso=%i opf=%i osf=%i otpf=%i otsf=%i\n", pF,sF,j,s1,s2,p1,p2,tpos->isOrphane,oldpredF, oldsuccF, otherpredF, othersuccF);
                    tpos->predF[j]=pF;
                    tpos->succF[j]=sF;
                }
                //if(s1==0 && p1==0) printf(" SET IT 1\n");
                //if(s2==0 && p2==0) printf(" SET IT 2\n");
                //if(tpos->state & para->STATE_ORPHANE)
                tpos->state = tpos->state & (tpos->state ^ para->STATE_ORPHANE);
                tpos->predFPos = -1;
                tpos->succFPos = -1;
            }
            //printf("end pre/succ %i\n", tpos);
            apos1->predF[s1]=p1;
            apos1->succF[s1]=p1;
            apos2->predF[s2]=p2;
            apos2->succF[s2]=p2;
            
            apos1->predF[s2]=p2;
            apos1->succF[s2]=p2;
            apos2->predF[s1]=p1;
            apos2->succF[s1]=p1;
            
            
            
            if(apos2->eqcRank< apos1->eqcRank) {
                apos2->eqcParent = apos1;
            } else {
                apos1->eqcParent = apos2;
                if(apos2->eqcRank==apos1->eqcRank)
                    apos2->eqcRank++;
            }
            
            //printf("end ranking %i, %i    %i\n", s1,p1,apos1->predF);
            apos1 = find_eqc(ap, s1,p1);//&(ap[s1][p1]);
            //printf("end first egc %i\n", tpos);
            apos2 = find_eqc(ap, s2,p2); //&(ap[s2][p2]);
            //printf("end second egc %i\n", tpos);
            
            // update the other affected sites
            for(ms=0;ms<slen;ms++) {
                // spaeter ueberlegen: if((ms!=s1 && ms!=s2) || ( (ms==s1 || ms==s2) && (i==0 || i== (length-1) ))) {
                //	if(apos1->predF[ms]==apos1->succF[ms]) {
                
                p = apos1->predF[ms]; // -( (apos1->predF[ms]==apos1->succF[ms]) ? 1 : 0); // GOGOGOGO
                opos=apos1->predF[ms];
                //    printf("SUPERPRE WHILE %i %i %i\n",k,s1,s2);
                tp = ap[ms];
                //printf("PRE WHILE %i\n",k);
                seenNonOrphane=0;
                found = 1;
                //firstRound = 1;
                //firstpos = 0;
                while( (p>=0)&& found) { // && tp[p].isOrphane) {
                    //printf(" WHILE %i\n",p);
                    if(tp[p].state & para->STATE_ORPHANE) {
                        if( (! seenNonOrphane)) {
                            //	      if(ms==0 && p==0) printf("setting s1=%i p1=%i iso=%i ms=%i p=%i opos=%i\n",s1,p1, tp[p].isOrphane, ms,p,opos);
                            tp[p].succFPos = opos;
                        } else p = tp[p].predFPos+1;
                    } else {
                        if( (p!=opos) || (apos1->succF[ms]!=apos1->predF[ms]))
                            seenNonOrphane = 1;
                        //if(p==442) printf("  pre find %i %i %i\n",s1, ms,p);
                        tpos = find_eqc(ap, ms,p);//&(ap[s1][p1]);
                        //if(p==442)printf("  post find %i %i\n",ms,p);
                        if(! (tpos->state & para->STATE_ORPHANE) && ! (tpos->state & para->STATE_INHERITED)) {
                            //printf(" %i %i %i %i\n",ms,p,s1,p1);
                            if(seenNonOrphane) found = 0;
                            for(s=0;s<slen;s++) {
                                if(tpos->succF[s]>apos1->succF[s]) {
                                    tpos->succF[s]=apos1->succF[s];
                                    found = 1;
                                }
                            }
                            /*
                             oldpos = firstpos;
                             s = firstpos;
                             while(s<slen) {
                             if(firstRound) {
                             nextpos[s]=s+1;
                             }
                             if(tpos->succF[s]>apos1->succF[s]) {
                             tpos->succF[s]=apos1->succF[s];
                             found = 1;
                             oldpos = s;
                             } else {
                             if(! firstRound) {
                             nextpos[oldpos] = nextpos[s];
                             if(oldpos==firstpos) {
                             firstpos=nextpos[oldpos];
                             oldpos = firstpos;
                             }
                             }
                             }
                             if(firstRound) {
                             oldpos = s;
                             s++;
                             } else {
                             s = nextpos[s];
                             }
                             }
                             */
                        }
                    }
                    //firstRound = 0;
                    p--;
                }
                
                //printf("END WHILE\n");
                
                //printf(" PRE 2 %i %i %i\n", apos1->predF, apos1->succF,ms );
                p = apos1->succF[ms]; // +( (apos1->predF[ms]==apos1->succF[ms]) ? 1 : 0); // GOGOGOGO ;
                opos= apos1->succF[ms];
                plen = scol->seqs[ms].length;
                seenNonOrphane=0;
                //printf("2. PRE WHILE %i\n",k);
                // if(opos>=plen) opos = -1;
                found = 1;
                while( (p<plen ) && found) {
                    //      step = (int)(p-tep);
                    if(tp[p].state & para->STATE_ORPHANE) {
                        if( (! seenNonOrphane)) {
                            /*
                             if(p==opos) {
                             printf("ALARM set predFPos s1=%i s2=%i p1=%i p2=%i ms=%i sF=%i pF=%i p=%i opos=%i iso=%i %i %i %i\n",
                             s1,s2,p1,p2,ms, apos1->succF[ms],
                             apos1->predF[ms],p,opos, tp[p].state,(int) &tp[p],(int) apos1, (int) apos2);
                             exit(98);
                             }
                             */
                            tp[p].predFPos = opos;
                        } else {
                            if(tp[p].succFPos < 0)
                                p = plen+1;
                            else
                                p = tp[p].succFPos-1;
                        }
                        
                    } else {
                        if( (p!=opos)|| (apos1->succF[ms]!=apos1->predF[ms]))
                            seenNonOrphane = 1;
                        //printf("  pre find %i %i\n",ms,p);
                        tpos = find_eqc(ap, ms,p);//&(ap[s1][p1]);
                        //printf("  end find\n");
                        if(! (tpos->state & para->STATE_ORPHANE)  && !(tpos->state & para->STATE_INHERITED)) {
                            if(seenNonOrphane) found = 0;
                            for(s=0;s<slen;s++) {
                                //		printf("   s=%i slen=%i tpos->predF=%i\n",s,slen,tpos->predF);
                                
                                if( tpos->predF[s]<apos1->predF[s]) {
                                    //printf("   inner before s=%i slen=%i\n",s,slen);
                                    tpos->predF[s]=apos1->predF[s];
                                    //printf("   inner after s=%i slen=%i\n",s,slen);
                                    found = 1;
                                }
                            }
                        }
                    }
                    p++;
                }
                //printf("2. END WHILE %i\n",k);
            }
            
        }
    }
    
    
    // printf("s1: %i s2: %i\n", algn->seq_is_orphane[s2],s2);
    algn->seq_is_orphane[s1]=0;
    algn->seq_is_orphane[s2]=0;
    
    int maxposs1 = dg->seq_p1.startpos + dg->length-1;
    int maxposs2 = dg->seq_p2.startpos + dg->length-1;
    if(dg->seq_p1.sq->max_seen < maxposs1)
        dg->seq_p1.sq->max_seen = maxposs1;
    
    if(dg->seq_p2.sq->max_seen < maxposs2)
        dg->seq_p2.sq->max_seen = maxposs2;
    
    //if(alignedSomething) {
    /*
     if(algn->aligned_diags_amount >= algn->max_aligned_diags_amount) {
     algn->max_aligned_diags_amount = algn->aligned_diags_amount + 16;
     algn->aligned_diags = realloc(algn->aligned_diags,
     sizeof(struct diag*)*algn->max_aligned_diags_amount);
     if(algn->aligned_diags==NULL) error(" align_diag(): Out of Memory!");
     }
     algn->aligned_diags[algn->aligned_diags_amount++] = dg;
     */
    /*
     } else {
     
     dgc = malloc(sizeof(struct diag_cont));
     dgc->dg = dg;
     dgc->next = NULL;
     algn->backlog_diags = enter_sorted(algn->backlog_diags, dgc);
     
     }
     */
    //printf(" diaglen: %i\n", algn->aligned_diags_amount);
    if(! dg->anchor) algn->total_weight += dg->weight;//*dg->weight_fac;
    return(alignedSomething);
}





/**
 *
 * prepares the alignment: calculates the position number of each residue
 *
 */
void prepare_alignment(struct alignment *algn)
{
    struct seq_col *scol = algn->scol;
    unsigned int slen = scol->length;
    
    unsigned int i,j,s,ts, hasmore, max;
    int *predF, *succF;
    struct seq* sq;
    struct algn_pos **ap = algn->algn;
    int  tproc[slen];
    //  char proceed[slen];
    
    for(i=0;i<slen;i++) {
        tproc[i]=0;
    }
    
    //
    // prepare block
    //
    struct algn_pos *ap1;
    hasmore = 1;
    char alarmHasProceed;
    
    for(j=0;hasmore;j++) {
        hasmore = 0;
        //memset(proceed, 0, slen*sizeof(char));
        //    printf("Position: %i\n", j);
        //    for(k=0;k<2;k++) {
        for(s=0;s<slen;s++) {
            sq = &scol->seqs[s];
            if(tproc[s]<sq->length) {
                ap1 = find_eqc(ap,s,tproc[s]);
                *ap1->proceed = 1;
                
            }
        }
        for(s=0;s<slen;s++) {
            sq = &scol->seqs[s];
            if(tproc[s]<sq->length) {
                //printf(" DO IT %i %i %i %i\n",j,s,tproc[s], *ap1->eqcAlgnPos);
                ap1 = find_eqc(ap,s,tproc[s]);
                //		printf("alig.c ap1 = %d\tap = %d\n",ap1,ap);
                
                if(j>=*ap1->eqcAlgnPos) {
                    predF = ap1->predF;
                    succF = ap1->succF;
                    
                    *ap1->eqcAlgnPos=j;// *tap1->eqcAlgnPos;
                    if(predF!=NULL) {
                        for(ts=0;ts<slen;ts++) {
                            //printf(" MIST %i %i %i %i %i %i %i\n",j,s,ts,tproc[s], tproc[ts], predF[ts], succF!=NULL ? succF[ts]: 99999);
                            if( (tproc[ts]<predF[ts]) && !(tproc[ts]==predF[ts] && succF!=NULL && succF[ts]==predF[ts])) {
                                *ap1->eqcAlgnPos=j+1;// *tap1->eqcAlgnPos;
                                //printf(" 2. MIST %i %i %i %i %i %i %i\n",j,s,ts,tproc[s], tproc[ts], predF[ts], succF!=NULL ? succF[ts]: 99999);
                                *ap1->proceed = 0;
                                break;
                            } else {
                                /*
                                 *ap1->eqcAlgnPos=j;// *tap1->eqcAlgnPos;
                                 *ap1->proceed = 1;
                                 */
                            }
                        }
                    }
                } else {
                    *ap1->proceed = 0;
                }
                /*
                 if(j>=143) {
                 printf("ALARM j=%i s=%i tproc[s]=%i algnPos=%i\n",j,s,tproc[s],*ap[s][tproc[s]].eqcAlgnPos);
                 }
                 */
            }
        }
        alarmHasProceed=0;
        for(s=0;s<slen;s++) {
            sq = &scol->seqs[s];
            if(tproc[s]<sq->length) {
                ap1 = find_eqc(ap,s,tproc[s]);
                if(*ap1->proceed) {
                    alarmHasProceed = 1;
                    tproc[s]++;
                }
            }
            if(tproc[s]<sq->length) hasmore = 1;
            //printf("%i %i\n", sq->length,tproc[s]);
        }
        if(! alarmHasProceed && hasmore) {
            printf("IO ALARM! %i\n",j);
            exit(1);
            hasmore=0;
        }
        if(!hasmore) max = j+1;
    }
    algn->max_pos= max;
}

#pragma mark -
#pragma mark assemble

/**
 * enters the candidate in a sorted way to the list and returns pointer to the first element
 */
struct diag_cont* enter_sorted(struct diag_cont* backlog_diags, struct diag_cont *cand)
{
    if(backlog_diags==NULL) {
        cand->next=NULL;
        return cand;
    }
    
    struct diag_cont *prev = backlog_diags;
    struct diag_cont *actual = backlog_diags;
    
    //printf(" before %f %i\n", cand->dg->total_weight, backlog_diags->dg);
    //  if( (cand==backlog_diags) || (backlog_diags->next==cand)) error(" enter_sorted(): critical error");
    
    if((cand->dg->total_weight >= backlog_diags->dg->total_weight)) {
        cand->next = backlog_diags;
        return cand;
    }
    
    while( (actual!=NULL) && (cand->dg->total_weight < actual->dg->total_weight)) {
        prev = actual;
        actual = actual->next;
    }
    prev->next = cand;
    cand->next = actual;
    
    //printf(" after\n");
    
    return backlog_diags;
}




/**
 * returns whether the given first position of the diag fits intho the given alignment
 */
char fit_fpos_diag(struct alignment *algn, struct diag* dg)
{
    unsigned int s1 = dg->seq_p1.num;
    unsigned int s2 = dg->seq_p2.num;
    
    char o1 = algn->seq_is_orphane[s1];
    char o2 = algn->seq_is_orphane[s2];
    // if any sequence is orphane the diag is always consistent
    if( o1 || o2) return 1;
    
    int sp1 = dg->seq_p1.startpos;
    int sp2 = dg->seq_p2.startpos;
//    int ep1 = sp1+dg->length-1;
    int ep2 = sp2+dg->length-1;
    
    struct algn_pos **ap=algn->algn;
    int predF, succF;
    
    
    struct algn_pos *tap;
    
    tap = find_eqc(ap, s1, sp1);
    if(tap->predF!=NULL) { //&& tap->succF!=NULL) {
        predF = tap->predF[s2];
    } else {
        predF=-1;
    }
    if(tap->succF!=NULL) {
        succF = tap->succF[s2];
    } else {
        succF = ep2+1;
    }
    if( ( (predF>=sp2)|| (succF<=sp2) ) && !(predF==sp2 && succF==sp2)) {
        //printf(" leave fit_fpos 0 \n");
        return 0;
    } else {
        //    printf(" leave fit_fpos 1 \n");
        return 1;
    }
    
}



/**
 * changes the startpos's and length of the diag such that
 * it becomes consistent with the given alignment. If the diag
 * has more than one part that is consistent the leftmost will be chosen
 * The return value is 1 if any changes were made otherwise 0.
 * (The weight of the diag is not recalculated !).
 *
 */
char adapt_diag(struct alignment *algn, struct scr_matrix *smatrix, struct diag* dg)
{
    if(dg->multi_dg) {
        char adapted = 0;
        int i;
        for(i=0;i<dg->multi_length;i++) {
            if(dg->multi_cont[i]!=NULL) {
                if(dg->multi_cont[i]->length > 0) {
                    adapted = adapted || adapt_diag(algn, smatrix, dg->multi_cont[i]);
                }
                if(dg->multi_cont[i]->length==0) {
                    free(dg->multi_cont[i]);
                    dg->multi_cont[i]=NULL;
                }
            }
        }
        return adapted;
    }
    
    unsigned int s1 = dg->seq_p1.num;
    unsigned int s2 = dg->seq_p2.num;
    
    char o1 = algn->seq_is_orphane[s1];
    char o2 = algn->seq_is_orphane[s2];
    // if any sequence is orphane the diag is always consistent
    if( o1 || o2) return 0;
    
    int sp1 = dg->seq_p1.startpos;
    int sp2 = dg->seq_p2.startpos;
    int ep1 = sp1+dg->length-1;
    int ep2 = sp2+dg->length-1;
    
    struct algn_pos **ap=algn->algn;
    int predF, succF;
    int rlen;
    
    
    // cut off the beginning of the diag
    struct algn_pos *tap;// = find_eqc(ap, s1, sp1);
    
    sp1--;
    sp2--;
    // jump to a consistent position
    //char included = 1;
    do {
        sp1++; sp2++;
        if(sp1<=ep1) {
            tap = find_eqc(ap, s1, sp1);
            if(tap->predF!=NULL) { //&& tap->succF!=NULL) {
                predF = tap->predF[s2];
            } else {
                predF=-1;
            }
            if(tap->succF!=NULL) {
                succF = tap->succF[s2];
            } else {
                succF = ep2+1;
            }
        }
        //if(predF!=sp2 || succF!=sp2) included = 0;
    } while( ( (predF>=sp2)|| (succF<=sp2) ) && !(predF==sp2 && succF==sp2) && sp1<=ep1);
    
    // check whether the diagonal has been cut off to zero length
    if(sp1>ep1) {
        //    printf(" OUT OF RANGE %i %i %i \n",predF, succF, sp2);
        //if(included)
        //  dg->weight = 0.0;
        //else
        //  dg->weight = -1.0;
        
        dg->length=0;
        return 1;
    }
    
    // cut off the end of the diag
    rlen=0;
    do {
        rlen++;
        //printf("   rlen: %i %i %i %i %i\n", ep1, sp1, sp2, dg->length, sp1+rlen-1);
        if((sp1+rlen-1)>ep1) {
            break;
        }else {
            tap = find_eqc(ap, s1, sp1+rlen-1);
            //printf("   after rlen: %i\n", rlen);
            if(tap->predF!=NULL) {
                predF = tap->predF[s2];
            } else {
                predF=-1;
            }
            if(tap->succF!=NULL) {
                succF = tap->succF[s2];
            } else {
                succF = sp2+rlen;
            }
        }
    } while( ( ((succF>=(sp2+rlen)) && (predF<(sp2+rlen-1))) || ((succF==(sp2+rlen-1)) && (predF==(sp2+rlen-1))))
            && ((sp1+rlen-1)<=ep1));
    rlen--;
    
    if(rlen<=0) {
        dg->length=0;
        //printf("sp1: %i\n", sp1);
        return 1;
    }
    int oldlen = dg->length;
    //printf("sp1: %i\n", sp1);
    dg->length = rlen;
    dg->seq_p1.startpos=sp1;
    dg->seq_p2.startpos=sp2;
    
    if(oldlen==rlen) {
        return 0;
    }
    
    return 1;
}


/**
 * heapify
 */
void heapify_diag_array(struct diag **diags, int pos, int length, int up)
{
    struct diag *dg = diags[pos];
    if(up) {
        if(pos<=3) return;
        int parent = (pos-1)/2;
        struct diag *pdg = diags[parent];
        if( (pdg->total_weight*pdg->weight_fac)< (dg->total_weight*dg->weight_fac)) {
            //if( (pow(pdg->total_weight,2)*pdg->weight_fac)< (pow(dg->total_weight,2)*dg->weight_fac)) {
            diags[parent]=dg;
            diags[pos] = pdg;
            //      printf("heapify: %i %i %i %i %i\n", pos,parent, length, dg, pdg);
            heapify_diag_array(diags, parent,length,up);
        }
        
    } else {
        int lchild = 2*pos+1;
        if( (lchild)>=length) return;
        int rchild = lchild+1;
        //struct diag *dg = diags[pos];
        struct diag *ldg = diags[lchild];
        struct diag *rdg = (rchild>=length ? ldg : diags[rchild]);
        int greatest = pos;
        if( (ldg->total_weight*ldg->weight_fac) > (diags[greatest]->total_weight * diags[greatest]->weight_fac)) greatest = lchild;
        //if( (pow(ldg->total_weight,2)*ldg->weight_fac) > (pow(diags[greatest]->total_weight,2) * diags[greatest]->weight_fac)) greatest = lchild;
        if( (rchild<length) && ( (rdg->total_weight*rdg->weight_fac) > (diags[greatest]->total_weight*diags[greatest]->weight_fac))) greatest = rchild;
        //if( (rchild<length) && ( (pow(rdg->total_weight,2)*rdg->weight_fac) > (pow(diags[greatest]->total_weight,2) *diags[greatest]->weight_fac))) greatest = rchild;
        if(greatest != pos) {
            diags[pos] = diags[greatest];
            diags[greatest] = dg;
            
            heapify_diag_array(diags, greatest,length,up);
            
        }
    }
}



/**
 *------------------------------------------------------------------------------------------
 *                                SIMPLE ALIGNER SECTION
 *------------------------------------------------------------------------------------------
 */

/**
 * test function that constructs an arbitrary consistent alignment.this function
 * is used to check the the correctness of adapt_diag() and align_diag()
 *
 * Returns whether something new could be aligned
 */
char simple_aligner(struct seq_col *scol, struct diag_col *dcol, struct scr_matrix* smatrix, struct prob_dist *pdist, struct alignment *algn, int round)
{
    int dlen = dcol->diag_amount;
    int i;
    
    struct diag * dg,*tdg;
    char changed;
    char alignedSomething = 0;
    // compute counter weights
    
    // heapify
    struct diag **diags = dcol->diags; //calloc(dlen, sizeof(struct diag *));
    int alloc_dlen = dlen;
    for(i= (dlen+1)/2-1;i>=0;i--) {
        heapify_diag_array(diags, i, dlen,0);
    }
    //memset(algn->redo_seqs, 0, sizeof(char)*slen*slen);
    
    double oldweight=0.0, prevweight;
    
    i=0;
    double total_weight = 0.0;
    double alig_time = 0.0;
    double tclock;
    struct diag tmp_diag;
    int tmp_end;
    int offset;
    struct diag *hookdg;
    while(dlen>0) {
        dg = diags[0];
        changed = 0;
        if(dg!=NULL) {
            if((dg->length>0) && (dg->meetsThreshold || dg->anchor) ) {
                prevweight = dg->weight;
                tmp_diag = *dg;
                changed= adapt_diag(algn,smatrix, dg);
                if(changed) {
                    calc_weight(dg, smatrix, pdist);
                    
                    if(dg->anchor) {
                        *dg = tmp_diag;
                    }
                    
                    if( (dg->length > 0) && !(dg->anchor)) {
                        tmp_end = tmp_diag.seq_p1.startpos+tmp_diag.length-1;
                        if( ((dg->seq_p1.startpos+dg->length-1)< tmp_end) &&
                           !(dg->multi_dg)) {
                            offset = dg->seq_p1.startpos+dg->length-tmp_diag.seq_p1.startpos;
                            tmp_diag.seq_p1.startpos += offset;
                            tmp_diag.seq_p2.startpos += offset;
                            tmp_diag.length -= offset;
                            
                            adapt_diag(algn,smatrix, &tmp_diag);
                            tmp_diag.length = tmp_end - tmp_diag.seq_p1.startpos+1;
                            calc_weight(&tmp_diag, smatrix, pdist);
                            
                            if((tmp_diag.length>0)&& tmp_diag.meetsThreshold) {
                                
                                hookdg = malloc(sizeof(struct diag));
                                *hookdg = tmp_diag;
                                hookdg->marked = 1;
                                
                                dcol->diag_amount++;
                                if(dcol->diag_amount>alloc_dlen) {
                                    alloc_dlen += 8;
                                    dcol->diags = (diags = realloc(diags, sizeof(struct diag*)*alloc_dlen));
                                    if(diags==NULL) error("Error increasing diag heap durign aligning.");
                                }
                                dlen++;
                                diags[dcol->diag_amount-1] = diags[dlen-1];
                                diags[dlen-1]=hookdg;
                                
                                if(dlen>=2) {
                                    heapify_diag_array(diags,dlen-1,dlen,1);
                                }
                            }
                        }
                    } else {
                        dg->meetsThreshold=0;
                    }
                } else {
                    if(para->DEBUG >1) tclock = clock();
                    
                    alignedSomething =  align_diag(algn, smatrix, dg) || alignedSomething;
                    if(para->DEBUG >1) alig_time += clock()-tclock;
                    
                    if(para->DEBUG >1) total_weight += dg->total_weight;
                    dg->meetsThreshold = 0;
                }
            } else {
                oldweight = dg->weight;
            }
        }
        
        
        if((dg==NULL) || (!dg->meetsThreshold)) {
            tdg = diags[dlen-1];
            diags[dlen-1]=dg;
            diags[0] = tdg;
            dlen--;
        }
        heapify_diag_array(diags, 0, dlen,0);
    }
    if(para->DEBUG >1)   printf("  Total Weight: %.20f with pure alignment time %f \n", total_weight, alig_time/CLOCKS_PER_SEC);
    return alignedSomething;
}


/**
 *------------------------------------------------------------------------------------------
 *                                COMPLEX ALIGNER SECTION
 *------------------------------------------------------------------------------------------
 */



/**
 * returns a value >0 if the given diags are in conflict within the given alignment
 * returns a value <0 if there is an non-conflicting overlap
 * returns 0 in all other non-conflicting cases
 */
char confl_diag(struct alignment *algn, char *layer, struct diag *dg1, struct diag *dg2)
{
    //  if(dg1->multi_dg || dg2->multi_dg) error(" confl_diag(): cannot accept multi dgs!");
    int s1_1 = dg1->seq_p1.num;
    int s1_2 = dg1->seq_p2.num;
    int s2_1 = dg2->seq_p1.num;
    int s2_2 = dg2->seq_p2.num;
    int ts;
    
    int sp1_1 = dg1->seq_p1.startpos;
    int sp1_2 = dg1->seq_p2.startpos;
    int sp2_1 = dg2->seq_p1.startpos;
    int sp2_2 = dg2->seq_p2.startpos;
    int tsp;
    
    int sl1_1 = dg1->seq_p1.sq->length;
    int sl1_2 = dg1->seq_p2.sq->length;
    int sl2_1 = dg2->seq_p1.sq->length;
    int sl2_2 = dg2->seq_p2.sq->length;
    int tsl;
    
    struct algn_pos* ap1_1;
    struct algn_pos* ap1_2;
    struct algn_pos* ap2_1;
    struct algn_pos* ap2_2;
    int p1_1, p1_2, p2_1, p2_2;
    int off1, off2;
    int l1 = dg1->length;
    int l2 = dg2->length;
    
    int pF1, pF2, sF1, sF2;
    int opF1, opF2, osF1, osF2;
    int pos2[4];
    int lpos2 = 0;
    
    int ret = 0;
    signed char ucmp,lcmp;
    
    
    //l1=1;l2=1;
    /*
     int step1 = (l1>10) ? 3 : 1;
     int step2 = (l2>10) ? 3 : 1;;
     if(step1==0) step1 = 1;
     if(step2==0) step2 = 1;
     */
    if(layer[dg1->seq_p1.num]!=1) {
        ts = s1_2;
        s1_2 = s1_1;
        s1_1 = ts;
        
        tsl = sl1_2;
        sl1_2 = sl1_1;
        sl1_1 = tsl;
        
        tsp = sp1_2;
        sp1_2 = sp1_1;
        sp1_1 = tsp;
    }
    if(layer[dg2->seq_p1.num]!=1) {
        ts = s2_2;
        s2_2 = s2_1;
        s2_1 = ts;
        
        tsl = sl2_2;
        sl2_2 = sl2_1;
        sl2_1 = tsl;
        
        tsp = sp2_2;
        sp2_2 = sp2_1;
        sp2_1 = tsp;
    }
    
    // if one is included in the other we define it as a conflict
    //if( (s1_1==s2_1) && (s1_2==s2_2)) {
    //
    //}
    
    opF1 = -1;
    osF1 = sl2_1;
    opF2 = -1;
    osF2 = sl2_2;
    
    
    for(off1=0;off1<l1;off1++) {
        
        p1_1 = sp1_1; // dg1->seq_p1.startpos+off1;
        p1_2 = sp1_2; // dg1->seq_p2.startpos+off1;
        ap1_1 = find_eqc(algn->algn, s1_1, p1_1);
        ap1_2 = find_eqc(algn->algn, s1_2, p1_2);
        
        // calculate the positions in dg2 that have to be considered for conflicts
        if(ap1_1->predF!=NULL) {
            pF1 = ap1_1->predF[s2_1];
        } else {
            pF1 = opF1;
        }
        if(ap1_1->succF!=NULL) {
            sF1 = ap1_1->succF[s2_1];
        } else {
            sF1 = osF1;
        }
        if(ap1_2->predF!=NULL) {
            pF2 = ap1_2->predF[s2_2];
        } else {
            pF2 = opF2;
        }
        if(ap1_2->succF!=NULL) {
            sF1 = ap1_2->succF[s2_2];
        } else {
            sF2 = osF2;
        }
        
        /*
         //if(ret==0)
         if( (pF1>=sp2_1) && (pF1!=opF1) &&  (pF1<sp2_1+l2))
         ret--;//ret = -1;
         else
         if( (sF1>=sp2_1) && (sF1!=osF1)&& (sF1<sp2_1+l2))
         ret--;//ret = -1;
         else
         if( (pF2>=sp2_2) && (pF2!=opF2) && (pF2<sp2_2+l2))
         ret--;//ret = -1;
         else
         if( (sF2>=sp2_2) && (sF2!=osF2)&& (sF2<sp2_2+l2))
         ret--;//ret = -1;
         */
        
        if(pF1 < sp2_1) {
            pF1 = sp2_1;
        }
        if(pF1 >= sp2_1+l2) {
            pF1 = sp2_1+l2-1;
            off1 = l1;
        }
        if(sF1 < sp2_1) {
            sF1 = sp2_1;
            off1 = l1;
        }
        if(sF1 >= sp2_1+l2) {
            sF1 = sp2_1+l2-1;
        }
        if(pF2 < sp2_2) {
            pF2 = sp2_2;
        }
        if(pF2 >= sp2_2+l2) {
            pF2 = sp2_2+l2-1;
            off1 = l1;
        }
        if(sF2 < sp2_2) {
            sF2 = sp2_2;
            off1 = l1;
        }
        if(sF2 >= sp2_2+l2) {
            sF2 = sp2_2+l2-1;
        }
        
        lpos2 = 0;
        if((pF1!=opF1)) {
            pos2[lpos2++] = pF1-sp2_1;
        }
        if((pF2!=opF2)) {
            pos2[lpos2++] = pF2-sp2_2;
        }
        if((sF1!=osF1)) {
            pos2[lpos2++] = sF1-sp2_1;
        }
        if((sF2!=opF2)) {
            pos2[lpos2++] = sF2-sp2_2;
        }
        
        opF1 = pF1;
        opF2 = pF2;
        osF1 = sF1;
        osF2 = sF2;
        //for(off2=0;off2<l2;off2++) {
        for(off2=0;off2<lpos2;off2++) {
            
            //p2_1 = sp2_1+off2;
            //p2_2 = sp2_2+off2;
            p2_1 = sp2_1+pos2[off2];
            p2_2 = sp2_2+pos2[off2];
            ap2_1 = find_eqc(algn->algn, s2_1, p2_1);
            ap2_2 = find_eqc(algn->algn, s2_2, p2_2);
            
            ucmp = -1;
            lcmp = -1;
            
            // upper compare
            if(s1_1==s2_1) {
                if(p1_1 == p2_1) ucmp = 0;
                if(p1_1 <  p2_1) ucmp = 1;
                if(p1_1 >  p2_1) ucmp = 2;
            } else if( (ap1_1->succF!=NULL) && (ap1_1->predF!=NULL) &&
                      (ap1_1->succF[s2_1]==ap1_1->predF[s2_1]) &&
                      (ap1_1->predF[s2_1]==p2_1)) {
                ucmp = 0;
            }  else if( ( (ap1_1->succF!=NULL) && ( ap1_1->succF[s2_1]<=p2_1))) {
                ucmp = 1;
            } else if( ( (ap1_1->predF!=NULL) && ( ap1_1->predF[s2_1]>=p2_1))) {
                ucmp = 2;
            }
            
            // lower compare
            if(s1_2==s2_2) {
                if(p1_2 == p2_2) lcmp = 0;
                if(p1_2 <  p2_2) lcmp = 1;
                if(p1_2 >  p2_2) lcmp = 2;
            } else if( (ap1_2->succF!=NULL) && (ap1_2->predF!=NULL) &&
                      (ap1_2->succF[s2_2]==ap1_2->predF[s2_2]) &&
                      (ap1_2->predF[s2_2]==p2_2)) {
                lcmp = 0;
            }  else if( ( (ap1_2->succF!=NULL) && ( ap1_2->succF[s2_2]<=p2_2))) {
                lcmp = 1;
            } else if( ( (ap1_2->predF!=NULL) && ( ap1_2->predF[s2_2]>=p2_2))) {
                lcmp = 2;
            }
            
            if( (ucmp>=0) && (lcmp>=0)) {
                if(ucmp!=lcmp) return 1;
            }
        }
    }
    /*
     ret = -ret;
     if((ret<=l1*0.1)&&(ret<=l2*0.1)) {
     return 0;
     } else {
     return -1;
     }
     */
    return ret;
}


/**
 * determine the best diags that fit into the alignment
 struct simple_diag_col *determine_diags((struct alignment *algn, struct seq_col *scol,
 struct scr_matrix *smatrix,
 struct prob_dist *pdist, char *layer,
 struct diag **diags,
 int diag_amount) {
 struct simple_diag_col scol;
 // TODO
 return scol;
 }
 */



/**
 * guided aligner recursion
 *
 * doHigher: >0 - only higher and equal than threshold weight
 * doHigher: 0  - only lower than threshold weight
 */
struct alignment* guided_aligner_rec(struct alignment *palgn, struct seq_col *scol, struct diag_col *dcol, struct scr_matrix* smatrix, struct prob_dist *pdist, struct gt_node *gtn, double thres_weight, char doHigher, int round)
{
    if(palgn==NULL) {
        palgn = create_empty_alignment(scol);
    }
    
    // recursion as per the guide tree
    if(gtn->isLeaf) return palgn;
    palgn = guided_aligner_rec(palgn,scol,dcol,smatrix,pdist, gtn->succ1, thres_weight,doHigher,round);
    
    palgn = guided_aligner_rec(palgn,scol,dcol,smatrix,pdist, gtn->succ2, thres_weight,doHigher,round);
    
    
    
    // now align the fragments that are between succ1 and succ2 of gtn
    int scol_len = scol->length;
    struct diag *dg, *tdg, *sdg, *stdg;
    int i,j,si,sj,k;
    struct gt_node *n1, *n2;
    n1 = gtn->succ1;
    n2 = gtn->succ2;
    int diag_amount = dcol->diag_amount;//n1->seq_num_length + n2->seq_num_length;
    double multi_weight_fac = 0.0;
    int diag_p=0;
    struct diag **all_diags = malloc(sizeof( struct diag*)*diag_amount);
    //struct diag *looser_diags[diag_amount];
    int diag_l=0;
    struct simple_diag_col *sdcol;
    //char crossing[scol_len*scol_len];
    char layer[scol_len];
    //  memset(crossing, 0, sizeof(char)*scol_len*scol_len);
    memset(layer, 0, sizeof(char)*scol_len);
    
    
    for(i=0;i<n1->seq_num_length;i++) {
        si = n1->seq_num[i];
        layer[si]=1;
        for(j=0;j<n2->seq_num_length;j++) {
            sj = n2->seq_num[j];
            
            if(i==0) layer[sj]=2;
            //printf("   %i %i %i %i\n", i,j,si,sj);
            //crossing[si*scol_len+sj] = 1;
            //crossing[sj*scol_len+si] = 1;
            if(sj>si) {
                sdcol = dcol->diag_matrix[scol_len*si+sj];
            } else {
                sdcol = dcol->diag_matrix[scol_len*sj+si];
            }
            multi_weight_fac += pow(sdcol->weight_fac,0.5);
            for(k=0;k<sdcol->length;k++) {
                dg = (sdcol->data[k]);
                //changed = adapt_diag(palgn, smatrix, dg);
                
                //if(changed) {
                //calc_weight(dg, smatrix, pdist);
                //}
                if(dg->meetsThreshold) {
                    if(doHigher>0) {
                        if((dg->total_weight) >= thres_weight) {
                            all_diags[diag_p++] = dg;
                        }
                    } else {
                        if((dg->total_weight) < thres_weight) {
                            all_diags[diag_p++] = dg;
                        }
                    }
                }
            }
        }
    }
    
    multi_weight_fac = pow(multi_weight_fac/((double)n1->seq_num_length*n2->seq_num_length),2.0);
    struct diag_col tdcol;
    
    int sdiag_p = diag_p;
    struct diag **sall_diags = all_diags;
    
    // if both successor nodes are leaf align directly
    if(n1->isLeaf && n2->isLeaf) {
        tdcol.diags = all_diags;
        tdcol.diag_amount = diag_p;
        
        //printf("  before inner rec %i %i %i\n",all_diags,diag_p,diag_amount);
//        alder_align_print_seq_col(scol);
//        alder_align_print_diag_col(&tdcol);
//        alder_align_print_scr_matrix(smatrix);
//        alder_align_print_prob_dist(pdist);
//        alder_align_print_alignment(palgn);
        simple_aligner(scol, &tdcol, smatrix, pdist, palgn, 0);
//        alder_align_print_alignment(palgn);
        //printf("  after inner rec %i\n",all_diags);
        free(all_diags);
        //printf(" after inner rec\n");
        return palgn;
    }
    
    diag_p = sdiag_p;
    struct diag **looser_diags = malloc(sizeof( struct diag*)*diag_amount);
    
    
    // build vertex cover graph
    char confl;
    int multilen1; int multipos1;
    int multilen2; int multipos2;
    char multi1, multi2;
    for(i=0;i<sdiag_p;i++) {
        dg = sall_diags[i];
        //dg->weight_fac = 1.0;
        dg->weight_sum = dg->total_weight;//*dg->weight_fac;
        multilen1 = 1;
        multipos1 = 0;
        multi1 = dg->multi_dg;
        if(multi1) multilen1 = dg->multi_length;
        
        //dg->weight_sum = dg->total_weight;
        //printf(" degree %i\n", dg->degree);
        sdg = dg;
        while(multipos1<multilen1) {
            if(multi1) dg = sdg->multi_cont[multipos1];
            
            if(dg!=NULL) {
                for(j=i+1;j<sdiag_p;j++) {
                    tdg = sall_diags[j];
                    
                    multilen2 = 1;
                    multipos2 = 0;
                    multi2 = tdg->multi_dg;
                    if(multi2) multilen2 = tdg->multi_length;
                    stdg = tdg;
                    
                    while(multipos2<multilen2) {
                        if(multi2) tdg = stdg->multi_cont[multipos2];
                        if(tdg!=NULL) {
                            
                            confl = confl_diag(palgn, layer, dg, tdg) ;
                            
                            if( (confl>0) ) {
                                //printf(" conflict %i %i !\n",i,j);
                                sdg->degree++;
                                if(sdg->degree > sdg->max_degree) {
                                    sdg->max_degree += 64;
                                    sdg->neighbours = realloc(sdg->neighbours, sizeof(struct diag *)*sdg->max_degree);
                                }
                                sdg->neighbours[sdg->degree - 1] = stdg;
                                
                                stdg->degree++;
                                if(stdg->degree > stdg->max_degree) {
                                    stdg->max_degree += 64;
                                    stdg->neighbours = realloc(stdg->neighbours, sizeof(struct diag *)*stdg->max_degree);
                                }
                                stdg->neighbours[stdg->degree - 1] = sdg;
                                //printf(" CONFLICT FOUND %i %i!\n", i,j);
                            }
                        }
                        multipos2++;
                    }
                }
            }
            multipos1++;
        }
    }
    // if(para->DEBUG>2) printf("   END: calc conflict of #diags: %i\n", sdiag_p);
    
    // perform clarkson vertex cover
    int max_n;
    double max_d;
    double t_d;
    while(1) {
        max_n = -1;
        max_d = -1.0;
        for(i=0;i<sdiag_p;i++) {
            dg = sall_diags[i];
            if(dg != NULL) {
                //printf(" degree %i\n", dg->degree);
                //printf(" before %i %i %i\n",i,sdiag_p,dg);
                //printf("   degree %i\n", dg->degree);
                //printf(" after\n");
                if(dg->degree > 0) {
                    t_d = dg->degree / dg->weight_sum;
                    if((max_d<t_d) || (max_n<0)){
                        max_n = i;
                        max_d = t_d;
                    }
                }
            }
        }
        
        //printf(" max_n: %i\n", max_n);
        if(max_n < 0) break;
        dg = sall_diags[max_n];
        sall_diags[max_n] = NULL;
        looser_diags[diag_l++] = dg;
        //    printf("  wlooser %i %i %i %f\n", max_n, dg, dg->multi_dg, dg->total_weight);
        //printf("          0sall[0] %i \n",sall_diags[0]);
        //i=11;
        //printf("          00sall[0] %i \n",sall_diags[0]);
        for(i=0; i < dg->degree; i++) {
            //printf("            1sall[0] %i i=%i\n",sall_diags[0],i);
            //printf(" %i %i\n",i,dg->degree);
            dg->neighbours[i]->weight_sum -= max_d;
            tdg = dg->neighbours[i];
            
            //printf(" before loop %i %i\n",i,dg->degree);
            for(j=0;j<tdg->degree;j++) {
                if(tdg->neighbours[j]==dg) {
                    //printf(" MATCH \n");
                    tdg->neighbours[j]=tdg->neighbours[tdg->degree-1];
                    tdg->degree--;
                    j=tdg->degree;//break;
                }
            }
            //printf("            2sall[0] %i\n",sall_diags[0]);
        }
        free(dg->neighbours);
        dg->neighbours=NULL;
        dg->degree = 0;
        dg->max_degree=0;
    }
    
    
    // align the winners first
    tdcol.diags = all_diags;
    tdcol.diag_amount = 0;
    for(i=0;i<sdiag_p;i++) {
        if(sall_diags[i]!=NULL) {
            //printf("  winner %i %i %f\n", sall_diags[i], sall_diags[i]->multi_dg, sall_diags[i]->total_weight);
            all_diags[tdcol.diag_amount++] = sall_diags[i];
        }
    }
    
    simple_aligner(scol, &tdcol, smatrix, pdist, palgn, 0);
    all_diags = tdcol.diags;
    for(i=0;i<tdcol.diag_amount;i++) {
        if(all_diags[i]->marked ) {
            //printf(" free %i %i %i\n", all_diags[i], all_diags[i]->multi_dg, looser_diags[0]);
            free_diag(all_diags[i]);
            //printf(" free end%i\n", all_diags[i]);
        } else {
            if(all_diags[i]->neighbours!=NULL) {
                free(all_diags[i]->neighbours);
                all_diags[i]->neighbours=NULL;
            }
            all_diags[i]->meetsThreshold = 0;
        }
    }
    
    // align the loosers afterwards
    tdcol.diags = looser_diags;
    tdcol.diag_amount = diag_l;
    simple_aligner(scol, &tdcol, smatrix, pdist, palgn, 0);
    looser_diags = tdcol.diags;
    for(i=0;i<tdcol.diag_amount;i++) {
        if(looser_diags[i]->marked ) {
            //printf(" lfree %i %i %i %i %i %i\n", i, diag_l, tdcol.diag_amount, looser_diags[i], all_diags[i]->multi_dg, looser_diags[0]);
            free_diag(looser_diags[i]);
            //printf(" lfree end%i\n", all_diags[i]);
        } else {
            if(looser_diags[i]->neighbours!=NULL) {
                free(looser_diags[i]->neighbours);
                looser_diags[i]->neighbours=NULL;
            }
            looser_diags[i]->meetsThreshold = 0;
        }
    }
    
    
    //printf("-----------------END GUIDED ALIGNER RECURSION STEP-------------------\n");
    
    // TODO: free/remove used diags
    //free(sall_diags);
    free(all_diags);
    free(looser_diags);
    
    return palgn;
}


/**
 * guided aligner method
 * NOTE: all diags are freeded within this routine
 */
struct alignment* guided_aligner(struct alignment *palgn, struct seq_col *scol, struct diag_col *dcol, struct scr_matrix* smatrix, struct prob_dist *pdist, struct gt_node *gtn, int round)
{
    struct diag **all_diags = dcol->diags;
    int diag_p = dcol->diag_amount;
//    struct diag *dg;
//    struct diag *tdg;
    int i,j,k;
    
    
    /*
     // sort diags
     for(i=0;i<diag_p;i++) {
     for(j=i+1;j<diag_p;j++) {
     dg = all_diags[i];
     tdg = all_diags[j];
     //if( (tdg->total_weight*dg->weight_fac) > (dg->total_weight*dg->weight_fac)) {
     if( (tdg->total_weight) > (dg->total_weight)) {
     all_diags[i] = tdg;
     all_diags[j]= dg;
     }
     }
     }
     */
    
    
    int slen = scol->length;
    
    double tavg, ttavg;
    double avg_weight = 0.0;
    struct simple_diag_col *sdcol;
    int tsdlen=0;
    ttavg = 0;
    
    for(i=0;i<slen;i++) {
        for(j=i+1;j<slen;j++) {
            
            tavg=0.0;
            sdcol = dcol->diag_matrix[slen*i+j];
            for(k=0; k<sdcol->length; k++) {
                tavg +=sdcol->data[k]->total_weight;
                ttavg += sdcol->data[k]->total_weight;
                tsdlen++;
            }
            
            if(sdcol->length >0) {
                tavg = (tavg/sdcol->length);
            }
            avg_weight += tavg;
        }
    }
    
    avg_weight = avg_weight/ (slen*(slen-1)/2.0);
    double thres_weight = ttavg/tsdlen;//1.0*avg_weight; //all_diags[ 0]->total_weight*0.05;
    
    palgn = guided_aligner_rec(palgn,scol,dcol,smatrix,pdist,gtn,thres_weight,1,round);
    
    //palgn = guided_aligner_rec(palgn,scol,dcol,smatrix,pdist,gtn,thres_weight,0,round);
    //printf("   intermediate alignment weight %f %f\n",palgn->total_weight, thres_weight);
    
    
    struct diag_col tdcol;
    struct diag **sall_diags = malloc(sizeof(struct diag*)*diag_p);
    int sdiag_p=0;
    
    for(i=0;i<diag_p;i++) {
        if(all_diags[i]->meetsThreshold && (all_diags[i]->total_weight < thres_weight)) {
            sall_diags[sdiag_p++] = all_diags[i];
        } else {
            free(all_diags[i]);
        }
    }
    
    tdcol.diags = sall_diags;
    tdcol.diag_amount = sdiag_p;
    simple_aligner(scol, &tdcol, smatrix, pdist, palgn, 0);
    sall_diags = tdcol.diags;
    
    for(i=0;i<tdcol.diag_amount;i++) {
        free(sall_diags[i]);
    }
    free(sall_diags); // FIXED.
    //diag_p -= thres_pos;
    //  dcol->diag_amount = diag_p;
    return palgn;
}

#pragma mark -
#pragma mark cleanup


void alder_align_free_scr_matrix(struct scr_matrix *o)
{
//struct scr_matrix
//{
//    int length;   // number of amino acids
//    int max_score; // maximum among all entries in the data array
//    int *char2num; // resolves the character of an amino acid to its number
//    int *num2char; // resolves the number of an amino acid to its character
//    int *data;     // contains the matrix indexed by the number of the particular amino acid
//    int *dist;   // number of pairs of amino acids (i,j) having score at equal to              // the  value of the index
//    long double **raw_dist;
//    double avg_sim_score;
//    
//};
    if (o->char2num != NULL) free(o->char2num);
    if (o->num2char != NULL) free(o->num2char);
    if (o->data != NULL) free(o->data);
    if (o->dist != NULL) free(o->dist);
    free(o);
}

void alder_align_free_prob_dist(struct prob_dist *o)
{
//struct prob_dist
//{
//    struct scr_matrix *smatrix; // pointer to the associated score matrix
//    long double **data;  // distribution of scores dist[i][j] contains the
//    double **log_data; // distribution of scores dist[i][j] contains the
//    unsigned int max_dlen;     // maximum diaglength in dist
//};
    assert(o->data[0] == NULL);
    assert(o->log_data[0] == NULL);
    for (size_t i = 1; i <= o->max_dlen; i++)
    {
        free(o->data[i]);
        free(o->log_data[i]);
    }
    free(o->data);
    free(o->log_data);
    free(o);
}



void alder_align_free_seq(struct seq *o)
{
//struct seq
//{
//    char *data;  // sequence data
//    char *name;  // name/description of the sequence
//    int num;     // number of the sequence
//    int length;  // length of sequence
//    int max_seen;
//    char *dna_num; // Numbers fo retranslation from Protein to DNA
//    int orf_frame; // reading frame of the longest orf
//    char crick_strand; // orf translation or sequence translation on crickstrand
//};
    free(o->data);
    free(o->name);
}

void alder_align_free_seq_col(struct seq_col *o)
{
//struct seq_col
//{
//    struct seq *seqs; // array of the sequences
//    int avg_length;   // average length of sequences
//    int length;       // number of sequences
//};
    for (int i = 0; i < o->length; i++) {
        free(o->seqs[i].data);
        free(o->seqs[i].name);
//        alder_align_free_seq(o->seqs)
    }
    free(o->seqs);
    free(o);
}


#pragma mark -
#pragma mark wrapper

int alder_align_read_initialize_from_file()
{
    init_parameters();
    set_parameters_dna();
    set_parameters("/Users/goshng/Library/Developer/Xcode/DerivedData/dialign-bugmultaffytaqcooumdypjlaeuu/Build/Products/Debug/conf", "/Users/goshng/Library/Developer/Xcode/DerivedData/dialign-bugmultaffytaqcooumdypjlaeuu/Build/Products/Debug/t.fasta", NULL);
    
    char *smatrixfile = (char *)build_pathname(para->conf_dir,para->SCR_MATRIX_FILE_NAME);
    smatrix = read_scr_matrix(smatrixfile);
    if( para->DEBUG >5) print_scr_matrix(smatrix);
    // read the probability distribution for diagonals
    char *pdistfilename = (char *)build_pathname(para->conf_dir,para->DIAG_PROB_FILE_NAME);
    pdist = read_diag_prob_dist(smatrix, pdistfilename);
    
    free(smatrixfile);
    free(pdistfilename);
    return 0;
}

int alder_align_read_initialize()
{
    init_parameters();
    set_parameters_dna();
    smatrix = read_scr_matrix_from_code();
    if( para->DEBUG >5) print_scr_matrix(smatrix);
    pdist = read_diag_prob_dist_from_code(smatrix);
    
    return 0;
}

int alder_align_read_execute(const char *seqReference, const char *seqRead)
{
    struct seq_col *in_seq_col = read_fasta2(seqReference, seqRead);
    
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
    
    // Anchors
    struct diag_col adcol;
    struct simple_diag_col *anchors = NULL;
    
    para->DO_ANCHOR = 1;
    int anchor1[1] = {3};
    int anchor2[1] = {3};
    int anchorLength[1] = {10};
    double anchorScore[1] = {3.0};
    size_t numberOfAnchor = 1;
    if(para->DO_ANCHOR>0) {
//        anchors = read_anchors(para->ANCHOR_FILE_NAME, in_seq_col);
        anchors = read_anchors_from_code(in_seq_col, anchor1, anchor2,
                                         anchorLength, anchorScore,
                                         numberOfAnchor);
        
        adcol.diags = anchors->data;
        adcol.diag_amount = anchors->length;
        
        
        simple_aligner(in_seq_col, &adcol,smatrix,pdist,salgn,1);
        if(! para->FAST_MODE) simple_aligner(in_seq_col, &adcol,smatrix,pdist,algn,1);
        
        if(anchors!=NULL) {
            for(unsigned int i=0;i<adcol.diag_amount;i++) {
                free_diag(adcol.diags[i]);
            }
            free(adcol.diags);
            free(anchors);
        }
    }
    
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
    else
    {
        free_alignment(salgn);  // FIXED.
    }
    
/*
    if(para->out_file==NULL) {
        simple_print_alignment_default(algn);
    }else {
        fasta_print_alignment_default(algn, para->out_file);
    }
*/
    
    free_alignment(algn);
    alder_align_free_seq_col(in_seq_col); in_seq_col = NULL;
    
    return 0;
}

int alder_align_read_finalize()
{
    alder_align_free_prob_dist(pdist); pdist = NULL;
    alder_align_free_scr_matrix(smatrix); smatrix = NULL;
    free_parameters();
    return 0;
}
