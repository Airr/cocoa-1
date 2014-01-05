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
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[2]); // full-help
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[3]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[4]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[5]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[6]); // count
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[7]);
    sprintf(line,
            "      --disk=D MB               maximum disk space\n"
            "                                on [--outdir=%s] (default=`D <- %ld')",
            a->outdir_arg,
            a->disk_arg);
    bstrVectorAdd(o->help, line);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[9]);
    
    sprintf(line,
            "      --memory=M MB             maximum memory  (default=`M <- %ld')",
            a->memory_arg);
    bstrVectorAdd(o->help, line);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[11]);
    
    sprintf(line,
            "      --nthread=number          Number of threads  (default=`%ld')",
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
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[21]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[22]);// report
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[23]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[24]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[25]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[26]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[27]);;// simulate
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[28]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[29]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[30]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[31]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[32]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[33]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[34]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[35]);
    sprintf(line,
            "      --maxkmer=number < %ld \n"
            "                                Number of kmers in a file (default=`%ld')",
            LONG_MAX, a->maxkmer_arg);
    bstrVectorAdd(o->help, line);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[36]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[37]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[38]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[39]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[40]);// partition
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[41]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[42]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[43]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[44]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[45]);// decode
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[46]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[47]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[48]);// table
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[49]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[50]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[51]);// pack
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[52]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[53]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[54]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[55]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[56]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[57]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[58]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[59]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[60]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[61]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[62]);//exit
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[63]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[64]);
//    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[65]);//example
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[66]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[67]);
    bstrVectorAdd(o->help, gengetopt_args_info_detailed_help[68]);//doc
}

/* This function prints the default help message.
 */
static void alder_kmer_cmdline_parser_print_help (alder_kmer_option_t *o)
{
    my_cmdline_parser_print_version ();
    if (strlen(gengetopt_args_info_purpose) > 0)
        printf("\n%s\n", gengetopt_args_info_purpose);
    
    if (strlen(gengetopt_args_info_usage) > 0)
        printf("\n%s\n", gengetopt_args_info_usage);
    
    if (strlen(gengetopt_args_info_description) > 0)
        printf("\n%s\n", gengetopt_args_info_description);
    
    printf("\n");
    
    printf("%s\n", bdata(o->help->entry[0]));
    printf("%s\n", bdata(o->help->entry[1]));
    printf("%s\n", bdata(o->help->entry[2]));
    printf("%s\n", bdata(o->help->entry[3]));
    printf("%s\n", bdata(o->help->entry[4]));
    printf("%s\n", bdata(o->help->entry[5]));
    printf("%s\n", bdata(o->help->entry[6]));
    printf("%s\n", bdata(o->help->entry[7]));
    printf("%s\n", bdata(o->help->entry[8]));
    printf("%s\n", bdata(o->help->entry[9]));
    printf("%s\n", bdata(o->help->entry[10]));
    printf("%s\n", bdata(o->help->entry[11]));
    printf("%s\n", bdata(o->help->entry[12]));
//    printf("%s\n", bdata(o->help->entry[13]));
//    printf("%s\n", bdata(o->help->entry[14]));
//    printf("%s\n", bdata(o->help->entry[15]));
//    printf("%s\n", bdata(o->help->entry[16]));
//    printf("%s\n", bdata(o->help->entry[17]));
    printf("%s\n", bdata(o->help->entry[18]));
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

/* This function converts gengetopt to alder_kmer_option just for my
 * convenience.
 */
int alder_kmer_option_init(alder_kmer_option_t *o,
                           struct gengetopt_args_info *a,
                           int argc)
{
    int status = 0;
    o->infile = NULL;
    o->outfile = NULL;
    o->logfilename = NULL;
    o->help = NULL;
    
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
    if (status == 1) {
        return status;
    }
    
    /* Initialize the help message. */
    alder_kmer_cmdline_parser_print_help_init(o,a);
    
    if (a->help_given) {
        alder_kmer_cmdline_parser_print_help (o);
        status = 1;
        return status;
    } else if (a->detailed_help_given) {
        alder_kmer_cmdline_parser_print_detailed_help(o);
        status = 1;
        return status;
    } else if (argc == 1) {
        my_cmdline_parser_print_version ();
        printf("%s\n", a->help_help);
        status = 1;
        return status;
    }
    
    
    // Adjust the disk and memory sizes.
    a->disk_arg *= a->load_disk_arg;
    a->memory_arg *= a->load_memory_arg;
    
    if (a->cite_given) {
        alder_kmer_cite();
        status = 1;
        return status;
    }
    if (!(0 < a->kmer_arg)) {
        status = 1;
    }

    size_t inputs_num_start = 1;
    a->count_flag = 1;
    a->report_flag = 0;
    a->simulate_flag = 0;
    a->partition_flag = 0;
    a->decode_flag = 0;
    a->table_flag = 0;
    a->pack_flag = 0;
    
    if (!strcmp(a->inputs[0], "count")) {
        a->count_flag = 1;
    } else if (!strcmp(a->inputs[0], "report")) {
        a->count_flag = 0;
        a->report_flag = 1;
    } else if (!strcmp(a->inputs[0], "simulate")) {
        a->count_flag = 0;
        a->simulate_flag = 1;
    } else if (!strcmp(a->inputs[0], "partition")) {
        a->count_flag = 0;
        a->partition_flag = 1;
    } else if (!strcmp(a->inputs[0], "decode")) {
        a->count_flag = 0;
        a->decode_flag = 1;
    } else if (!strcmp(a->inputs[0], "table")) {
        a->count_flag = 0;
        a->table_flag = 1;
    } else if (!strcmp(a->inputs[0], "pack")) {
        a->count_flag = 0;
        a->pack_flag = 1;
    } else {
        inputs_num_start = 0;
    }
    
    if (a->inputs_num == 0 && a->no_partition_flag == 0) {
        fprintf(stderr,
                "Error: no command and no input files.\n");
        fprintf(stderr,
                "Suggestion: choose a command or type in input file names\n");
        return ALDER_STATUS_ERROR;
    }
    
    if (a->maxcount_arg > UINT16_MAX) {
        fprintf(stderr,
                "Error: maxcount of slots in a hash table greater than %d\n",
                UINT16_MAX);
        fprintf(stderr, "Suggestion: use values less than or equal to %d.\n",
                UINT16_MAX);
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
    
    o->is_stdin = alder_file_isstdin();
    if (a->simulate_flag == 0) {
        if (o->is_stdin == 0 && a->inputs_num == 0) {
            status = 1;
        }
        // logc_logWarning(ERROR_LOGGER, "standard input and input files.");
        if (o->is_stdin == 1 && a->inputs_num > 0) {
            status = 1;
        }
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
    o->outfile = bstrVectorCreate(1);
    bstrVectorAdd(o->outfile, a->outfile_arg);
    
    if (a->log_given) {
        o->logfilename = bfromcstr(a->log_arg);
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
    
//    if (a->count_flag && !a->use_simulate_flag) {
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
    if (a->partition_flag) {
        if (!a->nthread_given) {
            a->nthread_arg = 1;
        }
        if (a->inputs_num < 2) {
            fprintf(stderr,
                    "Error: partition command needs an input file.\n");
            fprintf(stderr, "Suggestion: provide at least one sequence file.\n");
            status = 1;
            return status;
        }
    }
    
    return status;
}

void alder_kmer_option_free(alder_kmer_option_t *o)
{
    if (o->infile != NULL) bstrListDestroy(o->infile);
    if (o->outfile != NULL) bstrListDestroy(o->outfile);
    if (o->help != NULL) bstrListDestroy(o->help);
    if (o->logfilename != NULL) bdestroy(o->logfilename);
}

