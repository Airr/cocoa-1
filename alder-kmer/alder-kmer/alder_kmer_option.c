/**
 * This file, alder_kmer_option.c, is part of alder-kmer.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-kmer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-kmer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-kmer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdint.h>
#include "cmdline.h"
#include "alder_logger.h"
#include "alder_kmer_simulate.h"
#include "alder_cmacro.h"
#include "bstrmore.h"
#include "alder_file.h"
#include "alder_file_isstdin.h"
#include "alder_kmer_cite.h" 
#include "alder_kmer_option.h" 

#define MAXKMER 100000000

/* This function copies help message from cmdline.c, which is generated from
 * alder-kmer.ggo.
 */
static void alder_kmer_cmdline_parser_print_help_init(alder_kmer_option_t *o,
                                                      struct gengetopt_args_info *a)
{
    const int number_help_line = 1;
    char line[ALDER_HELP_LINE_SIZE];
    o->help = bstrVectorCreate(number_help_line);
    if (o->help == NULL) {
        return;
    }
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[0]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[1]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[2]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[3]); // full-helph
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[4]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[5]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[6]); // count
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[7]);
    
    sprintf(line,
            "  -d, --disk=INT               maximum disk space in MB\n"
            "                               on [--outdir=%s] (default=`%ld')",
            a->outdir_arg,
            a->disk_arg);
    bstrVectorAdd(o->help, line);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[9]);
    
    sprintf(line,
            "  -m, --memory=INT             maximum memory in MB (default=`%ld')",
            a->memory_arg);
    bstrVectorAdd(o->help, line);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[11]);
    
    sprintf(line,
            "  -t, --nthread=INT            number of threads  (default=`%ld')",
            a->nthread_arg);
    bstrVectorAdd(o->help, line);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[13]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[14]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[15]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[16]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[17]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[18]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[19]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[20]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[21]);// report
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[22]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[23]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[24]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[25]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[26]);// simulate
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[27]);;
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[28]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[29]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[30]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[31]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[32]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[33]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[34]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[35]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[36]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[37]);// partition
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[38]);// inbuffer
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[39]);// outbuffer
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[40]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[41]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[42]);// decode
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[43]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[44]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[45]);// table
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[46]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[47]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[48]);// list
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[49]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[50]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[51]);// match
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[52]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[53]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[54]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[55]);// binary
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[56]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[57]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[58]);// uncompress
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[59]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[60]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[61]);// inspect
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[62]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[63]);// internal use
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[64]);// internal use
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[65]);// internal use
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[66]);// internal use
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[67]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[68]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[69]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[70]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[71]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[72]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[73]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[74]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[75]);//exit
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[76]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[77]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[78]);//example
}

/* This function prints the default help message.
 */
static void alder_kmer_cmdline_parser_print_help (alder_kmer_option_t *o)
{
    printf("%s\n", gengetopt_args_info_purpose);
    
    printf("\n%s\n", gengetopt_args_info_usage);
    
    printf("\n");
    
    printf("%s\n\n", gengetopt_args_info_description);
    
    
    printf("%s\n", bdata(o->help->entry[0]));
    printf("%s\n", bdata(o->help->entry[1]));
    printf("%s\n", bdata(o->help->entry[2]));
    printf("%s\n", bdata(o->help->entry[3]));
//    printf("%s\n", bdata(o->help->entry[4]));
    printf("%s\n", bdata(o->help->entry[5]));
    printf("%s\n", bdata(o->help->entry[6]));
//    printf("%s\n", bdata(o->help->entry[7]));
    printf("%s\n", bdata(o->help->entry[8]));
//    printf("%s\n", bdata(o->help->entry[9]));
    printf("%s\n", bdata(o->help->entry[10]));
    printf("%s\n", bdata(o->help->entry[11]));
    printf("%s\n", bdata(o->help->entry[12]));
//    printf("%s\n", bdata(o->help->entry[13]));
//    printf("%s\n", bdata(o->help->entry[14]));
//    printf("%s\n", bdata(o->help->entry[15]));
//    printf("%s\n", bdata(o->help->entry[16]));
    printf("%s\n", bdata(o->help->entry[17]));
//    printf("%s\n", bdata(o->help->entry[18]));
    printf("%s\n", bdata(o->help->entry[19]));
    printf("%s\n", bdata(o->help->entry[20]));
}

/* This function prints the default help message.
 */
static void alder_kmer_cmdline_parser_print_detailed_help (alder_kmer_option_t *o)
{
    my_cmdline_parser_print_version ();
    if (strlen(gengetopt_args_info_purpose) > 0)
        printf("\n%s\n", gengetopt_args_info_purpose);
    
    if (strlen(gengetopt_args_info_usage) > 0)
        printf("\n%s\n", gengetopt_args_info_usage);
    
    printf("\n");
    
    if (strlen(gengetopt_args_info_description) > 0)
        printf("%s\n\n", gengetopt_args_info_description);
    
    for (int i = 0; i < o->help->qty; i++) {
        printf("%s\n", bdata(o->help->entry[i]));
    }
}

static void alder_kmer_cmdline_parser_print_doc_help (alder_kmer_option_t *o)
{
    printf("%s\n", gengetopt_args_info_detailed_help[80]);
}

static void
alder_kmer_cmdline_parser_print_count_help (alder_kmer_option_t *o,
                                            struct gengetopt_args_info *a)
{
    printf("Usage: alder-kmer count [-kINT|--kmer=INT] [-tINT|--nthread=INT]\n"
           "         [-dINT|--disk=INT] [-mINT|--memory=INT] [--load-disk=REAL]\n"
           "         [--load-memory=REAL] [--no-partition] [--no-count] [--no-delete]\n"
           "         [-ofilename|--outfile=filename] [--outdir=directory]\n"
           "         [--nh=INT] [--ni=INT] [--np=INT] [FILES]...\n");
    
    printf("%s\n", gengetopt_args_info_detailed_help[4]);
    printf("%s\n", gengetopt_args_info_detailed_help[5]);
    printf("%s\n", gengetopt_args_info_detailed_help[7]);
    printf("%s\n", bdata(o->help->entry[6]));
    printf("%s\n", gengetopt_args_info_detailed_help[9]);
    printf("%s\n", bdata(o->help->entry[8]));
    printf("%s\n", gengetopt_args_info_detailed_help[11]);
    printf("%s\n", bdata(o->help->entry[10]));
    printf("%s\n", gengetopt_args_info_detailed_help[13]);
    printf("%s\n", gengetopt_args_info_detailed_help[14]);
    printf("%s\n", gengetopt_args_info_detailed_help[15]);
    printf("%s\n", gengetopt_args_info_detailed_help[16]);
    printf("%s\n", gengetopt_args_info_detailed_help[17]);
    printf("%s\n", gengetopt_args_info_detailed_help[18]);
    printf("%s\n", gengetopt_args_info_detailed_help[29]);
    printf("%s\n", gengetopt_args_info_detailed_help[30]);
    
}

static void alder_kmer_cmdline_parser_print_report_help (alder_kmer_option_t *o)
{
    printf("Usage: alder-kmer report [-s|--summary] [-qsequence|--query=sequence]\n"
           "         [FILE]n");
    
    int a[4] = {19,20,22,23};
    for (int i = 0; i < sizeof(a)/sizeof(a[0]); i++) {
        printf("%s\n", gengetopt_args_info_detailed_help[a[i]]);
    }
}

static void alder_kmer_cmdline_parser_print_simulate_help (alder_kmer_option_t *o)
{
    printf("Usage: alder-kmer simulate [-kINT|--kmer=INT] [-ofilename|--outfile=filename]\n"
           "         [--maxkmer=INT] [--format=STRING] [--ni=INT] [--np=INT] [--nf=INT]\n"
           "         [--seqlen=INT] [--seed=INT] [FILES]...\n");
   
    int a[13] = {24,25,7,13,14,27,28,29,30,31,32,33,34};
    for (int i = 0; i < sizeof(a)/sizeof(a[0]); i++) {
        printf("%s\n", gengetopt_args_info_detailed_help[a[i]]);
    }   
}

static void alder_kmer_cmdline_parser_print_partition_help (alder_kmer_option_t *o)
{
    printf("Usage: alder-kmer partition [-kINT|--kmer=INT] [--ni=INT] [--np=INT]\n"
           "         [-dINT|--disk=INT] [-mINT|--memory=INT] [-tINT|--nthread=INT]\n"
           "         [--load-disk=REAL] [--load-memory=REAL] [--outdir=directory]\n"
           "         [-ofilename|--outfile=filename] [FILES]...\n");
   
    printf("%s\n", gengetopt_args_info_detailed_help[35]);
    printf("%s\n", gengetopt_args_info_detailed_help[36]);
    printf("%s\n", gengetopt_args_info_detailed_help[7]);
    printf("%s\n", gengetopt_args_info_detailed_help[29]);
    printf("%s\n", gengetopt_args_info_detailed_help[30]);
    printf("%s\n", bdata(o->help->entry[6]));
    printf("%s\n", gengetopt_args_info_detailed_help[9]);
    printf("%s\n", bdata(o->help->entry[8]));
    printf("%s\n", gengetopt_args_info_detailed_help[11]);
    printf("%s\n", bdata(o->help->entry[10]));
    printf("%s\n", gengetopt_args_info_detailed_help[13]);
    printf("%s\n", gengetopt_args_info_detailed_help[14]);
}

static void alder_kmer_cmdline_parser_print_decode_help (alder_kmer_option_t *o)
{
    printf("Usage: alder-kmer decode [-kINT|--kmer=INT] [-ofilename|--outfile=filename]\n"
           "         [--outdir=directory] [FILES]...\n");
    
    int a[5] = {40,41,7,13,14};
    for (int i = 0; i < sizeof(a)/sizeof(a[0]); i++) {
        printf("%s\n", gengetopt_args_info_detailed_help[a[i]]);
    }
}

static void alder_kmer_cmdline_parser_print_table_help (alder_kmer_option_t *o)
{
    printf("Usage: alder-kmer table [-kINT|--kmer=INT] [--nh=INT] [-mINT|--memory=INT]\n"
           "         [-tINT|--nthread=INT] [--load-memory=REAL] [--outdir=directory]\n"
           "         [-ofilename|--outfile=filename] [FILES]...\n");
    
    printf("%s\n", gengetopt_args_info_detailed_help[43]);
    printf("%s\n", gengetopt_args_info_detailed_help[44]);
    printf("%s\n", gengetopt_args_info_detailed_help[7]);
    printf("%s\n", gengetopt_args_info_detailed_help[15]);
    printf("%s\n", bdata(o->help->entry[8]));
    printf("%s\n", gengetopt_args_info_detailed_help[11]);
    printf("%s\n", bdata(o->help->entry[10]));
    printf("%s\n", gengetopt_args_info_detailed_help[13]);
    printf("%s\n", gengetopt_args_info_detailed_help[14]);
    
}

static void alder_kmer_cmdline_parser_print_list_help (alder_kmer_option_t *o)
{
    printf("Usage: alder-kmer list [-kINT|--kmer=INT] [-ofilename|--outfile=filename]\n"
           "         [--outdir=directory] [--seqlen=INT] [FILE]\n");
    
    int a[6] = {46,47,7,32,13,14};
    for (int i = 0; i < sizeof(a)/sizeof(a[0]); i++) {
        printf("%s\n", gengetopt_args_info_detailed_help[a[i]]);
    }
}

static void alder_kmer_cmdline_parser_print_match_help (alder_kmer_option_t *o)
{
    printf("Usage: alder-kmer match [--tabfile=filename] [-ofilename|--outfile=filename]\n"
           "         [--outdir=directory] [FILES]...\n");
    
    int a[5] = {49,50,52,13,14};
    for (int i = 0; i < sizeof(a)/sizeof(a[0]); i++) {
        printf("%s\n", gengetopt_args_info_detailed_help[a[i]]);
    }
}

static void alder_kmer_cmdline_parser_print_binary_help (alder_kmer_option_t *o)
{
    printf("Usage: alder-kmer binary [-ofilename|--outfile=filename]\n"
           "         [--outdir=directory] [FILES]...\n");
    
    int a[3] = {53,13,14};
    for (int i = 0; i < sizeof(a)/sizeof(a[0]); i++) {
        printf("%s\n", gengetopt_args_info_detailed_help[a[i]]);
    }
}

static void alder_kmer_cmdline_parser_print_uncompress_help (alder_kmer_option_t *o)
{
    printf("Usage: alder-kmer uncompress [-ofilename|--outfile=filename]\n"
           "         [--outdir=directory] [FILE]\n");
    
    int a[3] = {56,13,14};
    for (int i = 0; i < sizeof(a)/sizeof(a[0]); i++) {
        printf("%s\n", gengetopt_args_info_detailed_help[a[i]]);
    }
}

static void alder_kmer_cmdline_parser_print_inspect_help (alder_kmer_option_t *o)
{
    printf("Usage: alder-kmer inspect [-ofilename|--outfile=filename]\n"
           "         [--outdir=directory] [FILE]\n");
    
    int a[3] = {59,13,14};
    for (int i = 0; i < sizeof(a)/sizeof(a[0]); i++) {
        printf("%s\n", gengetopt_args_info_detailed_help[a[i]]);
    }
}


/* This function converts gengetopt to alder_kmer_option just for my
 * convenience.
 */
int alder_kmer_option_init(alder_kmer_option_t *o,
                           struct gengetopt_args_info *a,
                           int argc)
{
    int status = 0;
    o->infile = NULL;
    o->help = NULL;
    
    /* Simple runs or error checking without requiring much execution. */
    if (a->cite_given) {
        alder_kmer_cite();
        status = 1;
        return status;
    }
    if (a->document_flag) {
        alder_kmer_cmdline_parser_print_doc_help (o);
        return ALDER_STATUS_ERROR;
    }
    
    if (a->maxkmer_arg > MAXKMER) {
        fprintf(stderr,
                "Error: maxcount of slots in a hash table greater than %d\n",
                MAXKMER);
        fprintf(stderr, "Suggestion: use values less than or equal to %d.\n",
                MAXKMER);
        status = 1;
        return status;
    }
    if (a->seqlen_arg < a->kmer_arg) {
        fprintf(stderr,
                "Error: sequence length must be greater than or equal to "
                "kmer length; seqlen=%ld, kmer=%ld\n",
                a->seqlen_arg, a->kmer_arg);
        fprintf(stderr, "Suggestion: use --seqlen or --kmer to let sequence "
                "length be greater, or kmer size be smaller.\n");
        status = 1;
        return status;
    }
    if (!(0 < a->kmer_arg)) {
        fprintf(stderr, "Error: a negative kmer=%ld\n", a->kmer_arg);
        fprintf(stderr, "Suggestion: use a positive value of --kmer!\n");
        status = 1;
        return status;
    }
    if (a->format_arg == format_arg_fasta ||
        a->format_arg == format_arg_fa) {
        o->format = ALDER_FORMAT_FASTA;
    } else if (a->format_arg == format_arg_fastq ||
               a->format_arg == format_arg_fq) {
        o->format = ALDER_FORMAT_FASTQ;
    } else if (a->format_arg == format_arg_seq) {
        o->format = ALDER_FORMAT_SEQUENCE;
    }
    
    /* Free memory, free disk space, and maximum open files. */
    int64_t physical_memory = 0;
    int64_t free_memory = 0;
    int64_t used_memory = 0;
    if (!a->memory_given) {
        alder_file_availablememory(&physical_memory);
        alder_file_availablememory2(&free_memory, &used_memory);
        a->memory_arg = (long)(free_memory/ALDER_MB2BYTE);
    }
    unsigned long disk_space = 0;
    if (!a->disk_given) {
        alder_file_availablediskspace(a->outdir_arg, &disk_space);
        a->disk_arg = (long)(disk_space/ALDER_MB2BYTE);
    }
    if (!a->nthread_given) {
        a->nthread_arg = (long) alder_file_sysctl();
        if (a->nthread_arg > 1) {
            a->nthread_arg /= 2;
        }
    }
    status = alder_file_availableopenfiles(&o->maxfile);
    o->maxfile -= 4;
    if (status == 1) {
        return status;
    }
    
    /* Adjust the disk and memory sizes. */
    a->disk_arg *= a->load_disk_arg;
    a->memory_arg *= a->load_memory_arg;
    
    /* Initialize the help message. */
    alder_kmer_cmdline_parser_print_help_init(o,a);
    if (argc == 1) {
//        my_cmdline_parser_print_version ();
//        printf("%s\n", a->help_help);
        printf("Usage: alder-kmer [command] -h\n");
        printf("Available commands: count, report, simulate, partition, decode,\n"
               "                    table, list, binary, uncompress, and inspect.\n");
        status = 1;
        return status;
    } else if (a->inputs_num == 0 && a->help_given) {
        alder_kmer_cmdline_parser_print_help (o);
        status = 1;
        return status;
    } else if (a->inputs_num == 0 && a->detailed_help_given) {
        alder_kmer_cmdline_parser_print_detailed_help(o);
        status = 1;
        return status;
    } 
    
    /* Command check */
    size_t inputs_num_start = 1;
    a->count_flag = 0;
    a->report_flag = 0;
    a->simulate_flag = 0;
    a->partition_flag = 0;
    a->decode_flag = 0;
    a->table_flag = 0;
    a->list_flag = 0;
    a->match_flag = 0;
    a->binary_flag = 0;
    a->uncompress_flag = 0;
    a->inspect_flag = 0;
    if (!strcmp(a->inputs[0], "count") || *a->inputs[0] == 'c') {
        a->count_flag = 1;
    } else if (!strcmp(a->inputs[0], "report") || *a->inputs[0] == 'r') {
        a->report_flag = 1;
    } else if (!strcmp(a->inputs[0], "simulate") || *a->inputs[0] == 's') {
        a->simulate_flag = 1;
    } else if (!strcmp(a->inputs[0], "partition") || *a->inputs[0] == 'p') {
        a->partition_flag = 1;
    } else if (!strcmp(a->inputs[0], "decode") || *a->inputs[0] == 'd') {
        a->decode_flag = 1;
    } else if (!strcmp(a->inputs[0], "table") || *a->inputs[0] == 't') {
        a->table_flag = 1;
    } else if (!strcmp(a->inputs[0], "list") || *a->inputs[0] == 'l') {
        a->list_flag = 1;
    } else if (!strcmp(a->inputs[0], "match") || *a->inputs[0] == 'm') {
        a->match_flag = 1;
    } else if (!strcmp(a->inputs[0], "binary") || *a->inputs[0] == 'b') {
        a->binary_flag = 1;
    } else if (!strcmp(a->inputs[0], "uncompress") || *a->inputs[0] == 'u') {
        a->uncompress_flag = 1;
    } else if (!strcmp(a->inputs[0], "inspect") || *a->inputs[0] == 'i') {
        a->inspect_flag = 1;
    } else {
        fprintf(stderr, "Error: the first argument must be a command.\n");
        fprintf(stderr, "Suggestion: use one of the following commands.\n");
        fprintf(stderr, "            count, report, simulate, partition, "
                "decode, table, and list.\n");
        status = 1;
        return status;
    }
    if (a->count_flag) {
        if (a->ni_given == 0) {
            a->ni_arg = -1;
        }
        if (a->np_given == 0) {
            a->np_arg = -1;
        }
        if (a->nh_given == 0) {
            a->nh_arg = -1;
        }
    }
    
    /* Help message for commands. */
    if (a->count_flag && a->help_given) {
        alder_kmer_cmdline_parser_print_count_help(o,a);
        status = 1;
        return status;
    } else if (a->report_flag && a->help_given) {
        alder_kmer_cmdline_parser_print_report_help(o);
        status = 1;
        return status;
    } else if (a->simulate_flag && a->help_given) {
        alder_kmer_cmdline_parser_print_simulate_help(o);
        status = 1;
        return status;
    } else if (a->partition_flag && a->help_given) {
        alder_kmer_cmdline_parser_print_partition_help(o);
        status = 1;
        return status;
    } else if (a->decode_flag && a->help_given) {
        alder_kmer_cmdline_parser_print_decode_help(o);
        status = 1;
        return status;
    } else if (a->table_flag && a->help_given) {
        alder_kmer_cmdline_parser_print_table_help(o);
        status = 1;
        return status;
    } else if (a->list_flag && a->help_given) {
        alder_kmer_cmdline_parser_print_list_help(o);
        status = 1;
        return status;
    } else if (a->match_flag && a->help_given) {
        alder_kmer_cmdline_parser_print_match_help(o);
        status = 1;
        return status;
    } else if (a->binary_flag && a->help_given) {
        alder_kmer_cmdline_parser_print_binary_help(o);
        status = 1;
        return status;
    } else if (a->uncompress_flag && a->help_given) {
        alder_kmer_cmdline_parser_print_uncompress_help(o);
        status = 1;
        return status;
    } else if (a->inspect_flag && a->help_given) {
        alder_kmer_cmdline_parser_print_inspect_help(o);
        status = 1;
        return status;
    }
    

//    if (a->inputs_num == 0 && a->no_partition_flag == 0) {
//        fprintf(stderr,
//                "Error: no command and no input files.\n");
//        fprintf(stderr,
//                "Suggestion: choose a command or type in input file names\n");
//        return ALDER_STATUS_ERROR;
//    }
    
    
    /* Check of STDIN is unstable. */
    o->is_stdin = alder_file_isstdin();
    if (a->simulate_flag == 0) {
//        if (o->is_stdin == 0 && a->inputs_num < 2) {
//            fprintf(stderr, "Error: no input\n");
//            status = 1;
//        }
//        if (o->is_stdin == 1 && a->inputs_num > 1) {
//            fprintf(stderr, "Error: both stdin and infiles.\n");
//            status = 1;
//        }
        if (status == 1) {
            return status;
        }
    }
    
    for (size_t i = inputs_num_start; i < a->inputs_num; i++) {
        int isExist = alder_file_exist(a->inputs[i]);
        if (isExist == 0) {
            fprintf(stderr, "Error: no such file %s\n", a->inputs[i]);
            status = 1;
            return status;
        }
    }
    
    if (a->inputs_num > 0) o->infile = bstrVectorCreate((int)(a->inputs_num));
    for (size_t i = inputs_num_start; i < a->inputs_num; i++) {
        bstrVectorAdd(o->infile, a->inputs[i]);
    }
    
//    printf("exit\n");
    
    return status;
}

/**
 *  This function frees the option.
 *
 *  @param o alder-kmer option
 */
void alder_kmer_option_free(alder_kmer_option_t *o)
{
    if (o->infile != NULL) bstrListDestroy(o->infile);
    if (o->help != NULL) bstrListDestroy(o->help);
}

