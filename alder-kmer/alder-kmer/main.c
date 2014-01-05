/**
 * This file, main.c, is part of alder-kmer.
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
#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"
#include "alder_logger.h"
#include "alder_cmacro.h"
#include "alder_file.h"
#include "alder_kmer.h"
#include "alder_rng.h"
#include "alder_kmer_option.h" 

int main(int argc, char * argv[])
{
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
        alder_logger_error_initialize(LOG_FINEST);
    } else {
        alder_logger_initialize(args_info.log_arg, LOG_SILENT);
        alder_logger_error_initialize(LOG_FINEST);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Count:
    if (args_info.count_flag) {
        s = alder_kmer_count((int)args_info.kmer_arg,
                             args_info.totalmaxkmer_arg,
                             args_info.disk_arg,
                             args_info.memory_arg,
                             option.maxfile,
                             args_info.inbuffer_arg,
                             args_info.outbuffer_arg,
                             (int)args_info.ni_arg,
                             (int)args_info.np_arg,
                             (int)args_info.nh_arg,
                             (int)args_info.nthread_arg,
                             args_info.progress_flag,
//                             progressToError_flag,
                             args_info.progress_to_stderr_flag,
                             args_info.no_pack_flag,
                             args_info.no_delete_partition_flag,
                             args_info.no_partition_flag,
                             args_info.do_partition_flag,
                             option.infile,
                             args_info.outdir_arg,
                             args_info.outfile_arg);
    }
    ///////////////////////////////////////////////////////////////////////////
    // Table:
    if (args_info.table_flag) {
        s = alder_kmer_count_withPartition(-1,
                                           (int)args_info.kmer_arg,
                                           args_info.disk_arg,
                                           args_info.memory_arg,
                                           option.maxfile,
                                           args_info.inbuffer_arg,
                                           args_info.outbuffer_arg,
                                           (int)args_info.ni_arg,
                                           (int)args_info.np_arg,
                                           (int)args_info.nh_arg,
                                           (int)args_info.nthread_arg,
                                           args_info.progress_flag,
//                                           progressToError_flag,
                                           args_info.progress_to_stderr_flag,
                                           args_info.no_pack_flag,
                                           option.infile,
                                           args_info.outdir_arg,
                                           args_info.outfile_arg);
    }
    ///////////////////////////////////////////////////////////////////////////
    // Partition:
    if (args_info.partition_flag) {
        s = alder_kmer_partition((int)args_info.kmer_arg,
                                 args_info.disk_arg,
                                 args_info.memory_arg,
                                 option.maxfile,
                                 args_info.inbuffer_arg,
                                 args_info.outbuffer_arg,
                                 (int)args_info.ni_arg,
                                 (int)args_info.np_arg,
                                 (int)args_info.nh_arg,
                                 (int)args_info.nthread_arg,
                                 args_info.progress_flag,
//                                 progressToError_flag,
                                 args_info.progress_to_stderr_flag,
                                 option.infile,
                                 args_info.outdir_arg,
                                 args_info.outfile_arg);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Report:
    if (args_info.report_flag) {
        if (args_info.query_given) {
            s = alder_kmer_report(bdata(option.infile->entry[0]),
                                  args_info.summary_flag,
                                  args_info.query_arg);
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
        if (args_info.maxkmer_given) {
            s = alder_kmer_simulate_woHashtable(args_info.outfile_arg,
                                                args_info.outdir_arg,
                                                option.format,
                                                (int)args_info.nf_arg,
                                                (int)args_info.ni_arg,
                                                (int)args_info.np_arg,
                                                (int)args_info.kmer_arg,
                                                (int)args_info.seqlen_arg,
                                                (size_t)args_info.maxkmer_arg);
        } else {
            assert(0);
            s = alder_kmer_simulate(args_info.outfile_arg,
                                    args_info.outdir_arg,
                                    option.format,
                                    (int)args_info.nf_arg,
                                    (int)args_info.ni_arg,
                                    (int)args_info.np_arg,
                                    (int)args_info.kmer_arg,
                                    (int)args_info.seqlen_arg,
                                    (int)args_info.maxcount_arg,
                                    (int)args_info.nh_arg);
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Decode:
    if (args_info.decode_flag) {
        s = alder_kmer_decode((int)args_info.kmer_arg,
                              args_info.progress_flag,
                              option.infile,
                              args_info.outdir_arg,
                              args_info.outfile_arg);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Pack:
    if (args_info.pack_flag) {
        if (args_info.use_table_flag) {
            s = alder_kmer_pack(bdata(option.infile->entry[0]),
                                args_info.outdir_arg,
                                args_info.outfile_arg);
        } else {
            s = alder_kmer_pack_kmercount(bdata(option.infile->entry[0]),
                                          args_info.outdir_arg,
                                          args_info.outfile_arg);
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Cleanup!
    if (args_info.log_given) {
        alder_logger_finalize();
        alder_logger_error_finalize();
    } else {
        alder_logger_finalize();
        alder_logger_error_finalize();
        alder_file_rm(args_info.log_arg);
    }
    my_cmdline_parser_free(&args_info);
    alder_kmer_option_free(&option);
    return s;
}

