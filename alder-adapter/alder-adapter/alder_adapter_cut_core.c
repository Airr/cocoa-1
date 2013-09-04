///////////////////////////////////////////////////////////////////////////////
// Two functions in the following two source files
// alder_adapter_cut_file.c
// alder_adapter_cut_gzip.c
// are almost identical. Espeically, they share the following core part.
///////////////////////////////////////////////////////////////////////////////

// First adapter
int isAdapterFound = 0;
kseq_t *seq;
seq = kseq_init(fp);
kseq_read(seq);
if (adapter_seq == NULL && seq->comment.s != NULL) {
    int index = alder_adapter_index_illumina_seq_summary(seq->comment.s);
    stat->adapter_seq_id = index;
    if (index >= 0) {
        isAdapterFound = 1;
        adapter_seq = adapter_sequence[index];
    }
} else {
    isAdapterFound = 1;
}
assert(adapter_seq != NULL);
if (adapter_seq == NULL) {
    logc_logErrorBasic(ERROR_LOGGER, 0, "no adapter sequence is found.");
}

// First adapter 2
int isAdapterFound2 = 0;
kseq_t *seq2 = NULL;
if (fnin2 != NULL) {
    seq2 = kseq_init(fp);
    kseq_read(seq2);
    if (adapter_seq2 == NULL && seq2->comment.s != NULL) {
        int index = alder_adapter_index_illumina_seq_summary(seq2->comment.s);
        stat->adapter_seq_id2 = index;
        if (index >= 0) {
            isAdapterFound2 = 1;
            adapter_seq2 = adapter_sequence[index];
        }
    } else {
        isAdapterFound2 = 1;
    }
    assert(adapter_seq2 != NULL);
    if (adapter_seq2 == NULL) {
        logc_logErrorBasic(ERROR_LOGGER, 0, "no adapter sequence 2 is found.");
    }
}


if (isAdapterFound == 1) {
    stat->adapter_seq = bfromcstr(adapter_seq);
    int m = (int)strlen(adapter_seq);
    int m2;
    if (seq2 != NULL) m2 = (int)strlen(adapter_seq2);
    do {
        int status;
        int start21, start22;
        
        bstring bSequence = bfromcstr(seq->seq.s);
        bstring bQuality = bfromcstr(seq->qual.s);
        int isFiltered = 0;
        status = alder_adapter_cut_filter(adapter_seq, m, bSequence, bQuality,
                                          &isFiltered, &start21, option);
        if (status != 0) {
            logc_logBasic(ERROR_LOGGER,
                          "Fatal: negative start2 or large start2\n"
                          "File: %s\n"
                          "%s\n%s\n+\n%s\n",
                          fnin, seq->name.s, seq->seq.s, seq->qual.s);
            abort();
        }
        bstring bSequence2;
        bstring bQuality2;
        int isFiltered2 = 0;
        if (seq2 != NULL) {
            bSequence2 = bfromcstr(seq2->seq.s);
            bQuality2 = bfromcstr(seq2->qual.s);
            status = alder_adapter_cut_filter(adapter_seq2, m2, bSequence2, bQuality2,
                                              &isFiltered2, &start22, option);
            if (status != 0) {
                logc_logBasic(ERROR_LOGGER,
                              "Fatal: negative start2 or large start2\n"
                              "File: %s\n"
                              "%s\n%s\n+\n%s\n",
                              fnin2, seq2->name.s, seq2->seq.s, seq2->qual.s);
                abort();
            }
        }
        
        // NEED 2
        stat->n_total++;
        stat->n_base += seq->seq.l;
        if (seq2 != NULL) stat->n_base2 += seq2->seq.l;
        if (option->keep == 0) {
            if (isFiltered == 0 && isFiltered2 == 0) {
                if (seq->comment.s == NULL) {
                    fprintf(fpout, "@%s\n%s\n+\n%s\n",
                            seq->name.s, bdata(bSequence), bdata(bQuality));
                    if (seq2 != NULL) {
                        fprintf(fpout2, "@%s\n%s\n+\n%s\n",
                                seq2->name.s, bdata(bSequence2), bdata(bQuality2));
                    }
                } else {
                    fprintf(fpout, "@%s %s\n%s\n+\n%s\n",
                            seq->name.s, seq->comment.s, bdata(bSequence), bdata(bQuality));
                    if (seq2 != NULL) {
                        fprintf(fpout2, "@%s %s\n%s\n+\n%s\n",
                                seq2->name.s, seq2->comment.s, bdata(bSequence2), bdata(bQuality2));
                    }
                }
                stat->n_base_remained += start21;
                if (start21 < seq->seq.l) stat->n_trimmed++;
                if (seq2 != NULL) {
                    stat->n_base_remained2 += start22;
                    if (start22 < seq2->seq.l) stat->n_trimmed2++;
                }
            } else {
                stat->n_trimmed++;
                if (seq2 != NULL) stat->n_trimmed2++;
            }
        } else {
            if (isFiltered == 1 && isFiltered2 == 1) {
                if (seq->comment.s == NULL) {
                    fprintf(fpout, "@%s\n%s\n+\n%s\n",
                            seq->name.s,
                            bdata(bSequence),
                            bdata(bQuality));
                    if (seq2 != NULL) {
                        fprintf(fpout2, "@%s\n%s\n+\n%s\n",
                                seq2->name.s,
                                bdata(bSequence2),
                                bdata(bQuality2));
                    }
                } else {
                    fprintf(fpout, "@%s %s\n%s\n+\n%s\n",
                            seq->name.s, seq->comment.s,
                            bdata(bSequence),
                            bdata(bQuality));
                    if (seq2 != NULL) {
                        fprintf(fpout2, "@%s %s\n%s\n+\n%s\n",
                                seq2->name.s, seq2->comment.s,
                                bdata(bSequence2),
                                bdata(bQuality2));
                    }
                }
            }
        }
        // NEED 2
        bdestroy(bQuality);  bdestroy(bSequence);
        bdestroy(bQuality2); bdestroy(bSequence2);
        
        if (seq2 != NULL) kseq_read(seq2);
    } while (kseq_read(seq) >= 0);
}

kseq_destroy(seq);
if (seq2 != NULL) {
    kseq_destroy(seq2);
}
