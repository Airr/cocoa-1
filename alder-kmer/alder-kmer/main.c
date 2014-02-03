/**
 * This file, main.c, is part of alder-kmer.
 *
 * Copyright 2013,2014 by Sang Chul Choi
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
#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"
#include "alder_logger.h"
#include "alder_cmacro.h"
#include "alder_file.h"
#include "alder_kmer.h"
#include "alder_rng.h"
#include "alder_kmer_option.h" 

/**
 *  This function is the main function where commands are called via
 *  other functions in the kmer library.
 *
 *  @param argc argc
 *  @param argv argv
 *
 *  @return program exit value (0 on SUCCESS, otherwise other values)
 */
int main(int argc, char * argv[])
{
    time_t start = time(NULL);
    int s = ALDER_STATUS_SUCCESS;
    int isExitWithHelp = 0;
    alder_kmer_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_kmer_option_init(&option, &args_info, argc);
    if (isExitWithHelp != 0) {
        my_cmdline_parser_free(&args_info);
        alder_kmer_option_free(&option);
        exit(isExitWithHelp);
    }
    ///////////////////////////////////////////////////////////////////////////
    // Set up a log file if needed.
    alder_file_rm(args_info.log_arg);
    if (args_info.log_given) {
        alder_logger_initialize(args_info.log_arg, (int)args_info.loglevel_arg);
        alder_logger_error_initialize(LOG_WARNING);
    } else {
        alder_logger_initialize(args_info.log_arg, LOG_SILENT);
        alder_logger_error_initialize(LOG_BASIC);
    }
//    alder_log("START: %s", ctime(&start));
    alder_log("START: %.*s", (int)(strlen(ctime(&start))-1), ctime(&start));
    
    ///////////////////////////////////////////////////////////////////////////
    // Count:
    if (args_info.count_flag) {
        s = alder_kmer_count(args_info.select_version_arg,
                             (int)args_info.kmer_arg,
                             args_info.totalmaxkmer_arg,
                             args_info.disk_arg,
                             args_info.memory_arg,
                             args_info.min_table_memory_arg,
                             args_info.max_table_memory_arg,
                             option.maxfile,
                             args_info.inbuffer_arg,
                             args_info.outbuffer_arg,
                             (int)args_info.ni_arg,
                             (int)args_info.np_arg,
                             (int)args_info.nh_arg,
                             (int)args_info.nthread_arg,
                             args_info.progress_flag,
                             args_info.progress_to_stderr_flag,
                             args_info.only_init_flag,
                             args_info.no_delete_flag,
                             args_info.no_partition_flag,
                             args_info.no_count_flag,
                             option.infile,
                             args_info.outfile_given,
                             args_info.outdir_arg,
                             args_info.outfile_arg);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Partition:
    if (args_info.partition_flag) {
        s = alder_kmer_partition(args_info.select_version_arg,
                                 (int)args_info.kmer_arg,
                                 args_info.disk_arg,
                                 args_info.memory_arg,
                                 args_info.min_table_memory_arg,
                                 args_info.max_table_memory_arg,
                                 option.maxfile,
                                 args_info.inbuffer_arg,
                                 args_info.outbuffer_arg,
                                 (int)args_info.ni_arg,
                                 (int)args_info.np_arg,
                                 (int)args_info.nh_arg,
                                 (int)args_info.nthread_arg,
                                 args_info.progress_flag,
                                 args_info.progress_to_stderr_flag,
                                 args_info.use_seqfile_flag,
                                 args_info.binfile_arg,
                                 args_info.binfile_given,
                                 option.infile,
                                 args_info.outfile_given,
                                 args_info.outdir_arg,
                                 args_info.outfile_arg);
    }
    ///////////////////////////////////////////////////////////////////////////
    // Table:
    if (args_info.table_flag) {
        s = alder_kmer_table(args_info.select_version_arg,
                             0, // -1, why -1?
                             (int)args_info.kmer_arg,
                             // args_info.disk_arg,
                             args_info.memory_arg,
                             option.maxfile,
                             args_info.inbuffer_arg,
                             args_info.outbuffer_arg,
                             (int)args_info.ni_arg,
                             (int)args_info.np_arg,
                             (int)args_info.nh_arg,
                             (int)args_info.nthread_arg,
                             args_info.progress_flag,
                             args_info.progress_to_stderr_flag,
                             // args_info.no_pack_flag,
                             0,
                             args_info.parfile_arg,
                             args_info.parfile_given,
                             option.infile,
                             args_info.outfile_given,
                             args_info.outdir_arg,
                             args_info.outfile_arg);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Report/Dump:
    if (args_info.report_flag) {
        if (args_info.query_given) {
            s = alder_kmer_report(bdata(option.infile->entry[0]),
                                  args_info.summary_flag,
                                  args_info.sequence_arg);
        } else {
            s = alder_kmer_report(bdata(option.infile->entry[0]),
                                  args_info.summary_flag,
                                  NULL);
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Simulation:
    if (args_info.simulate_flag) {
        PlantSeeds(args_info.seed_arg);
        SelectStream(1);
        s = alder_kmer_simulate(args_info.select_version_arg,
                                args_info.outfile_given,
                                args_info.with_parfile_flag,
                                args_info.outfile_arg,
                                args_info.outdir_arg,
                                option.format,
                                (int)args_info.nf_arg,
                                (int)args_info.ni_arg,
                                (int)args_info.np_arg,
                                (int)args_info.kmer_arg,
                                (int)args_info.seqlen_arg,
                                (size_t)args_info.maxkmer_arg,
                                args_info.progress_flag,
                                args_info.progress_to_stderr_flag);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Decode/Kmer:
    if (args_info.decode_flag) {
        s = alder_kmer_decode((int)args_info.kmer_arg,
                              args_info.progress_flag,
                              option.infile,
                              args_info.outfile_given,
                              args_info.outdir_arg,
                              args_info.outfile_arg);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // List:
    if (args_info.list_flag) {
        s = alder_kmer_list((int)args_info.kmer_arg,
                            args_info.progress_flag,
                            option.infile,
                            args_info.outfile_given,
                            args_info.outdir_arg,
                            args_info.outfile_arg);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Match:
    if (args_info.match_flag) {
        s = alder_kmer_match(args_info.progress_flag,
                             option.infile,
                             args_info.tabfile_arg,
                             args_info.outfile_given,
                             args_info.outdir_arg,
                             args_info.outfile_arg);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Uncompress:
    if (args_info.uncompress_flag) {
        alder_kmer_uncompress(args_info.progress_flag,
                              option.infile,
                              args_info.outfile_given,
                              args_info.outdir_arg,
                              args_info.outfile_arg);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Binary:
    if (args_info.binary_flag) {
        size_t n_byte = 0;
        uint64_t n_kmer = 0;
        uint64_t n_dna = 0;
        uint64_t n_seq = 0;
        size_t n_totalfilesize = 0;
        if (args_info.select_version_arg == 2 ||
            args_info.select_version_arg == 7) {
            args_info.select_version_arg = 2;
        }
        if (args_info.select_version_arg == 2 ||
            args_info.select_version_arg == 3) {
            size_t subsize = 1 << 16;
            s = alder_kmer_binary(NULL, 0, subsize,
                                  &n_kmer, &n_dna, &n_seq,
                                  &n_totalfilesize,
                                  &n_byte,
                                  args_info.select_version_arg,
                                  (int)args_info.kmer_arg,
                                  args_info.disk_arg,
                                  args_info.memory_arg,
                                  args_info.min_table_memory_arg,
                                  args_info.max_table_memory_arg,
                                  (int)args_info.nsplit_arg,
                                  args_info.progress_flag,
                                  args_info.progress_to_stderr_flag,
                                  option.infile,
                                  args_info.outdir_arg,
                                  args_info.outfile_arg);
        } else {
            alder_loge(ALDER_ERR, "Only version 4 or 5 is implemented!");
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Inspect:
    if (args_info.inspect_flag) {
        alder_kmer_inspect(option.infile,
                           args_info.outdir_arg,
                           args_info.outfile_arg);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Timing:
    time_t end = time(NULL);
    alder_log("END: %s", ctime(&end));
    double run_time = difftime(end, start);
    alder_log("END: %.f (s) = %.f (m) = %.f (h)", run_time, run_time/60, run_time/3600);
    
    ///////////////////////////////////////////////////////////////////////////
    // Cleanup!
    alder_logger_finalize();
    alder_logger_error_finalize();
    if (!args_info.log_given) {
        alder_file_rm(args_info.log_arg);
    }
    my_cmdline_parser_free(&args_info);
    alder_kmer_option_free(&option);
    return s;
}

