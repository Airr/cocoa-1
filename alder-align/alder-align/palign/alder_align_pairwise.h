/**
 *
 * struct.h: Basic data structures
 *
 * Author:  A.R.Subramanian (Dialign-TX)
 */

//  Modified by Sang Chul Choi on 8/14/13.
//
//  alder_assemble.h
//  dialign

#ifndef dialign_alder_assemble_h
#define dialign_alder_assemble_h

//#include "alder_palign_alignment.h"
#include "gsl_vector_sam.h"

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

#ifdef DEBUG
#undef DEBUG
#endif

struct parameters
{
	char *VERSION;
	// 0 no debug statements
	// 1 debugs the current phase of the processing
	// 2 very loquacious debugging
	// 5 hardcore debugging
	int DEBUG; // = 0;
    
	// maximum amount of input sequences
	int MAX_SEQ_AMOUNT; // = 5000;
    
	// maximum number of characters per line in a FASTA file
	int MAX_FASTA_LINE_LENGTH; // = 100;
    
	// maximum amount of characters per line when printing a sequence
	int PRINT_SEQ_LINE_LENGTH; // = 80;
    
	/*******************************
	 *
	 * PROTEIN SCORING/WEIGHTING SECTION
	 *
	 *****************************/
    
    
	// score matrix (e.g. BLOSUM) file name (in the configuration directory)
	//#define SCR_MATRIX_FILE_NAME "BLOSUM75.scr"
	char *SCR_MATRIX_FILE_NAME; // = "BLOSUM.scr";
	//#define SCR_MATRIX_FILE_NAME "BLOSUM90.scr"
    
	// defines the minimum weight when the weight is changed to
	//  1-pow(1-prob, factor)
	double DIAG_CALC_WEIGHT_THRESHOLD; // = 0.000000065;
	//#define DIAG_CALC_WEIGHT_THRESHOLD 0.0000002
    
    
	// diag prob. distribution  file name (in the configuration directory)
	char *DIAG_PROB_FILE_NAME; // = "BLOSUM.diag_prob_t10";
	//#define DIAG_PROB_FILE_NAME "BLOSUM.diag_prob_t7"   // current
	//#define DIAG_PROB_FILE_NAME "BLOSUM75.diag_prob_t2"   //
	//#define DIAG_PROB_FILE_NAME "BLOSUM75.diag_prob_t1"
	//#define DIAG_PROB_FILE_NAME "BLOSUM90.diag_prob"
	//#define DIAG_PROB_FILE_NAME "BLOSUM75.diag_prob"
	//#define DIAG_PROB_FILE_NAME "tp400_prot"
    
	// add to each score (to prevent negative values)
	int SCR_MATRIX_ADD; // = 0; // BLOSUM(62)
	//#define SCR_MATRIX_ADD 5 // BLOSUM75
	//#define SCR_MATRIX_ADD 6 // BLOSUM90
    
	/*******************************
	 *
	 * PROTEIN QUALITY SECTION
	 *
	 *****************************/
    
	// "even" sim score threshold for protein sequences alignment
	//#define PROT_SIM_SCORE_THRESHOLD 6 //BLOSUM90
	//#define PROT_SIM_SCORE_THRESHOLD 5 //BLOSUM75
	double PROT_SIM_SCORE_THRESHOLD; // = 4; // BLOSUM62
    
	// maximum number of consecutive positions for frag-window
	int PROT_DIAG_MAX_UNDER_THRESHOLD_POS; // = 4; // old was 4
    
	// minimum diagonal length for breaking
	double PROT_DIAG_MIN_LENGTH_THRESHOLD; // = 40.0; // old was 40
    
	// minimal allowed average csore in frag window
	double PROT_DIAG_AVG_SCORE_THRESHOLD; // = 4.0; // BLOSUM62
	//#define PROT_DIAG_AVG_SCORE_THRESHOLD 5.0 // BLOSUM75
    
    
	/*******************************
	 *
	 * GLOBAL QUALITY/SPEED SECTION
	 *
	 *****************************/
    
	// wether overlap weights are calculated or not
	int DO_OVERLAP; // = 0;
    
	// minimum diag length
	int DIAG_MIN_LENGTH;// = 1;
    
	// diag threshold weight
	double DIAG_THRESHOLD_WEIGHT;// = -log(0.5);
    
	// if there are anchors
	char DO_ANCHOR; // = 0;
    
	// name of optional anchor file
	char *ANCHOR_FILE_NAME; // = NULL;
    
	// sensitivity mode, does not set
    // DIAG_THRESHOLD_WEIGHT to 0 four rounds >1
	char SENS_MODE;// = 0;
    
	// fast mode - behaves like dialign t 0.2.2
	char FAST_MODE;// = 0;
    
	// 1: only use a sqrt(amount_of_seqs) stripe of neighbour sequences to
	// calculate pairwise alignments
	// 0: all pairwise alignments will be calculated
	int FAST_PAIRWISE_ALIGNMENT;// = 0;
    
    char *conf_dir ;
    char *in_file ;
    char *out_file ;
    char COMPUTE_PROB;
	int DNA_PARAMETERS; // default Einstellungen f¸r DNA holen
	int DNA_TRANSLATION; // Vergleich auf Proteinebene, input DNA
	int FIND_ORF; // Vergleich auf Proteinebene, input DNA, mit ORF Finder
	int ORF_FRAME; // Vergleich auf Proteinebene, input DNA, nur longest ORF wird aligned
	int OUTPUT; // f¸r DNA = 1, f¸r Aminos‰uren = 0
    
    
	int STATE_ORPHANE;
	int STATE_INHERITED;
	
};



/**
 * score matrix (e.g. BLOSUM62)
 */
struct scr_matrix
{
    int length;   // number of amino acids
    int max_score; // maximum among all entries in the data array
    int *char2num; // resolves the character of an amino acid to its number
    int *num2char; // resolves the number of an amino acid to its character
    int *data;     // contains the matrix indexed by the number of the particular amino acid
    int *dist;   // number of pairs of amino acids (i,j) having score at equal to              // the  value of the index
    //    long double **raw_dist;
    double avg_sim_score;
    
};

typedef struct scr_matrix alder_align_scr_matrix_t;

/**
 * raw sequence
 */
struct seq
{
    char *data;  // sequence data
    char *name;  // name/description of the sequence
    int num;     // number of the sequence
    int length;  // length of sequence
    int max_seen;
    //    char *dna_num; // Numbers fo retranslation from Protein to DNA
    int orf_frame; // reading frame of the longest orf
    char crick_strand; // orf translation or sequence translation on crickstrand
};

typedef struct seq alder_align_seq_t;

/**
 * sequence collection
 */
struct seq_col
{
    struct seq *seqs; // array of the sequences
    int avg_length;   // average length of sequences
    int length;       // number of sequences
};

typedef struct seq_col alder_align_seq_col_t;

/**
 * probability distribution of scores in diagonals
 */
struct prob_dist
{
    struct scr_matrix *smatrix; // pointer to the associated score matrix
    long double **data;  // distribution of scores dist[i][j] contains the
    double **log_data; // distribution of scores dist[i][j] contains the
    unsigned int max_dlen;     // maximum diaglength in dist
};

typedef struct prob_dist alder_align_prob_dist_t;

/**
 * part of a sequence (auxiliary data structure)
 */
struct seq_part
{
    int num;         // a number that indicates a position in an array
    struct seq* sq;  // the pointer to the sequence
    int startpos;   // startpos in the sequence
    //int leftmargin;
    //int rightmargin;
};

typedef struct seq_part alder_align_seq_part_t;

/**
 * diagonal in the dotmatrix
 */
struct diag
{
    struct seq_part seq_p1; // (.sq not x) first sequence part
    struct seq_part seq_p2; // (.sq not x) seconde sequence part
    unsigned int length;     // length of the diag
    long score;     // score of the diag
    long orig_score;     // orig score of the diag
    
    struct diag *pred_diag;  // (not x) predecessor diag for dynamic programming
    struct diag *col_pred_diag;  // (not x) col predecessor diag for dynamic programming
    int pool_pos;            // position in diag pool
    
    char meetsThreshold;	   // whether diag meets threshold
    
    // for vertex cover
    int degree;
    int max_degree;
    struct diag **neighbours; // (not x)
    
    char anchor;              // if this is an anchor diag
    char marked;              // marking flag for arbitrary use
    char multi_dg;            // is >0 if this is a multi dg
    struct diag **multi_cont; // (x: free_diag) the contained dgs of this is a multi dg
    int multi_length;         // size of multi_cont
    
    //  char onlyOverThres;
    
    double weight;           // weight of the diag = -log(prob)
    double weight_sum;       // weight sum for dialign
    double weight_fac;       // weight factor
    double ov_weight;       // overlap weight
    double total_weight;       // total_weight = weight+o_weight
};

typedef struct diag alder_align_diag_t;

/**
 * collection of diag
 */
struct simple_diag_col
{
    unsigned int length;       // number of diags
    double total_weight;       // total weight
    double weight_fac;         // weight factor
    struct diag** data;        // the array of diags
};

typedef struct simple_diag_col alder_align_simple_diag_col_t;

/**
 * guide tree node
 */
struct gt_node
{
    char isLeaf;    // whether it is leaf
    int *seq_num;    // the sequence numbers
    int seq_num_length; // length of sequence numbers array
    struct gt_node *succ1;  // successor nodes
    struct gt_node *succ2;
};

typedef struct gt_node alder_align_gt_node_t;

/**
 * collection of all diagonals sorted by the sequences
 */
struct diag_col
{
    int seq_amount;            // number of sequences involved
    struct simple_diag_col** diag_matrix; // 2-by-2 (pairwise) diag_matrix[i +seq_amount *j] contains
    double total_weight;       // total weight
    double average_weight;     // average_weight
    struct diag** diags; // all diags unordered: = diag_matrix->data
    unsigned int diag_amount; // number of diags found
    struct gt_node *gt_root;
};

typedef struct diag_col alder_align_diag_col_t;

/**
 * diag container
 */
struct diag_cont
{
    struct diag* dg;        // (not x, maybe by free_diag_col?)
    struct diag_cont *next; // (x)
};

typedef struct diag_cont alder_align_diag_cont_t;

/**
 * alignment position
 */
struct algn_pos
{
    char state;         // orphane: not aligned to any pos,
    
    struct diag_cont *dg_cont; // (partial x: see diag_cont) diags that are aligned with that position
    
    int row;
    int col;
    int* predF;    // (x) predecessor frontier, only filled if non-orphane
    int* succF;    // (x) successor frontier, only filled if non-orphane
    
    char *proceed; // (x) for output
    int predFPos;  // in case of orphane, where to find the predF or succF
    int succFPos;
    
    int *eqcAlgnPos; // (x) equivalence class minimum alignment position (for output)
    struct algn_pos *eqcParent; // (not x) equivalence class parent
    int eqcRank; // equivalence class rank (>= maximum number of children)
};

typedef struct algn_pos alder_align_algn_pos_t;

/**
 * alignment
 */
struct alignment
{
    char *seq_is_orphane;      // (x) boolean array indicating for each sequence
    int max_pos;               // the greatest position in the alignment (including all -'s)
    struct seq_col *scol;      // all the sequences involved
    struct algn_pos **algn;    // (x) the alignment
    double total_weight;       // the total weight of the alignment
    struct alignment *next;    // (?) pointer to next alignment in the sorted linked list
};

typedef struct alignment alder_align_alignment_t;


#pragma mark -
#pragma mark parameters
void init_parameters();
void free_parameters();
void set_parameters_dna();
void set_parameters(const char *confDir, const char *inFile, const char *outFile);
void check_input(int argc, char** argv);


#pragma mark -
#pragma mark io
void error(char *message);
void merror(char *msg1, char *msg2);
void strip_leading_ws( char *str );
char* build_pathname(char *dir, char *file);
void print_seq(struct seq * aSeq);
void print_diag(struct diag *aDiag);
void print_scr_matrix(struct scr_matrix *aSmatrix);
struct scr_matrix* read_scr_matrix(char *filename);
struct scr_matrix* read_scr_matrix_from_code();
struct prob_dist* read_diag_prob_dist(struct scr_matrix* smatrix, char *filename);
struct prob_dist* read_diag_prob_dist_from_code(struct scr_matrix* smatrix);
struct seq_col* read_fasta(char *filename);
struct seq_col* read_fasta2(const char *s1, const char *s2);
struct simple_diag_col* read_anchors(char *filename, struct seq_col* scol);
struct simple_diag_col* read_anchors_from_code(struct seq_col* scol, const int *anchor1, const int *anchor2, const int *anchorLength, const double *anchorScore, const size_t numberOfAnchor);
void simple_print_alignment_default(struct alignment *algn);
void simple_print_alignment_dna_retranslate(struct alignment *algn);
void fasta_print_alignment_default(struct alignment *algn, char *filename);
void fasta_print_alignment_dna_retranslate(struct alignment *algn, char *filename);
char* print_info(struct alignment *algn);
char* output_line(char *string);
char* output_line_left(char *string);
char* blank_line();
void print_pdist_matrix(struct prob_dist *sdist, char *filename);


#pragma mark -
#pragma mark diag
struct diag* create_diag(int n1, struct seq* sq1, unsigned int sp1, int n2, struct seq* sq2, unsigned int sp2, int dlength);
void free_diag(struct diag* dg);
void free_gt_node(struct gt_node *gtn);
unsigned long binomial(long m, long n);
long double **create_tmp_pdist(struct prob_dist *pdist);
void free_tmp_pdist(long double **dist, int length);
void fill_tmp_pdist(struct prob_dist *pdist, long double **tmp_dist, int slen1, int slen2);
void real_calc_weight(struct diag* dg, struct scr_matrix* smatrix, struct prob_dist *pdist, char omitScore, long double **tmp_dist, struct alignment *algn );
void calc_weight(struct diag* dg, struct scr_matrix* smatrix, struct prob_dist *pdist);
void calc_ov_weight(struct diag* dg, struct diag_col *dcol, struct scr_matrix* smatrix, struct prob_dist *pdist);
struct diag_col* create_diag_col(int seq_amount);
void free_diag_col(struct diag_col* dcol);
struct simple_diag_col* find_diags_guided(struct scr_matrix *smatrix, struct prob_dist *pdist, struct gt_node* n1, struct gt_node* n2, struct alignment *algn, double thres_weight, int*** diag_info);
struct simple_diag_col* find_diags_dialign(struct scr_matrix *smatrix, struct prob_dist *pdist, struct seq* seq1, struct seq* seq2, struct alignment *algn, long double **tmp_dist, int round);
struct simple_diag_col* find_diags_dyn(struct scr_matrix *smatrix, struct prob_dist *pdist, struct seq* seq1, struct seq* seq2, struct alignment *algn, long double **tmp_dist);
void build_guide_tree(struct diag_col *dcol);
struct diag_col *find_all_diags(struct scr_matrix *smatrix, struct prob_dist *pdist, struct seq_col *in_seq_col, struct alignment *algn, int round);
struct diag_col *old_find_all_diags(struct scr_matrix *smatrix, struct prob_dist *pdist, struct seq_col *in_seq_col, struct alignment *algn, int round );


#pragma mark -
#pragma mark alig
struct alignment* create_empty_alignment(struct seq_col *scol);
void free_alignment(struct alignment *algn);
struct algn_pos *_find_eqc(struct algn_pos *ap);
struct algn_pos *find_eqc(struct algn_pos **ap, int seqnum, int pos);
char align_diag(struct alignment *algn, struct scr_matrix *smatrix, struct diag* dg);
void prepare_alignment(struct alignment *algn);


#pragma mark -
#pragma mark assemble
struct diag_cont* enter_sorted(struct diag_cont* backlog_diags, struct diag_cont *cand);
char fit_fpos_diag(struct alignment *algn, struct diag* dg);
char adapt_diag(struct alignment *algn, struct scr_matrix *smatrix, struct diag* dg);
void heapify_diag_array(struct diag **diags, int pos, int length, int up);
char simple_aligner(struct seq_col *scol, struct diag_col *dcol, struct scr_matrix* smatrix, struct prob_dist *pdist, struct alignment *algn, int round);
char confl_diag(struct alignment *algn, char *layer, struct diag *dg1, struct diag *dg2);
struct alignment* guided_aligner_rec(struct alignment *palgn, struct seq_col *scol, struct diag_col *dcol, struct scr_matrix* smatrix, struct prob_dist *pdist, struct gt_node *gtn, double thres_weight, char doHigher, int round);
struct alignment* guided_aligner(struct alignment *palgn, struct seq_col *scol, struct diag_col *dcol, struct scr_matrix* smatrix, struct prob_dist *pdist, struct gt_node *gtn, int round);


#pragma mark -
#pragma mark cleanup
void alder_align_free_scr_matrix(struct scr_matrix *o);
void alder_align_free_prob_dist(struct prob_dist *o);
void alder_align_free_seq_col(struct seq_col *o);
void alder_align_free_seq(struct seq *o);


#pragma mark -
#pragma mark wrapper
int alder_align_read_initialize();
int alder_align_read_execute(const char *seqReference, const char *seqRead);
int alder_align_read_execute_with_anchor(const alder_sam_t *sam,
                                         const char *seqReference,
                                         const char *seqRead,
                                         const int *anchor1,
                                         const int *anchor2,
                                         const int *anchorLength,
                                         const double *anchorScore,
                                         const size_t numberOfAnchor);
void alder_palign_sam(alder_sam_t *sam, struct alignment *algn);
int alder_align_read_finalize();

__END_DECLS

#endif
