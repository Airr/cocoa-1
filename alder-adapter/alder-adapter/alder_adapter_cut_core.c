///////////////////////////////////////////////////////////////////////////////
// Two functions in the following two source files
// alder_adapter_cut_file.c
// alder_adapter_cut_gzip.c
// are almost identical. Espeically, they share the following core part.
///////////////////////////////////////////////////////////////////////////////

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


if (isAdapterFound == 1) {
    stat->adapter_seq = bfromcstr(adapter_seq);
    int m = (int)strlen(adapter_seq);
    const int flags = 14;
    const int degenerate = 0;
    int start1, best_i, start2, best_j, best_matches, best_cost;
    do {
        int status;
        // Trim either side of a read sequence.
        bstring bSequence = bfromcstr(seq->seq.s);
        bstring bQuality = bfromcstr(seq->qual.s);
        btoupper(bSequence);
        bltrim(bSequence, option->trim_left);
        brtrim(bSequence, option->trim_right);
        bltrim(bQuality, option->trim_left);
        brtrim(bQuality, option->trim_right);
        if (option->trim_ambiguous_left == 0) bltrimc2(bSequence, bQuality, 'N');
        if (option->trim_ambiguous_right == 0) brtrimc2(bSequence, bQuality, 'N');
        bltrimq2(bQuality, bSequence, option->phred+option->trim_quality_left);
        brtrimq2(bQuality, bSequence, option->phred+option->trim_quality_right);
        
        // Cuts adapter parts.
        char *seqS = bstr2cstr(bSequence, '\0');
        alder_adapter_cut(adapter_seq, m, seqS, blength(bSequence),
                          error_rate, flags, degenerate,
                          &start1, &best_i, &start2, &best_j,
                          &best_matches, &best_cost);
        bcstrfree(seqS);
        
        // Filter out reads.
        if (start2 < 0 || blength(bSequence) < start2) {
            logc_logBasic(ERROR_LOGGER,
                          "Fatal: negative start2 or large start2 - %s",
                          seq->name.s);
            abort();
        }
        btrunc(bSequence, start2);
        btrunc(bQuality, start2);
        int isFiltered = 0;
        if (blength(bSequence) < option->filter_length) isFiltered = 1;
        if (option->filter_ambiguous < bcount(bSequence, 'N')) isFiltered = 1;
        status = bcountq(bQuality, option->phred+option->filter_quality);
        if (status != BSTR_ERR && status > 0) isFiltered = 1;
        
        if (option->keep == 0) {
            if (isFiltered == 0) {
                if (seq->comment.s == NULL) {
                    fprintf(fpout, "@%s\n%s\n+\n%s\n",
                            seq->name.s, bdata(bSequence), bdata(bQuality));
                } else {
                    fprintf(fpout, "@%s %s\n%s\n+\n%s\n",
                            seq->name.s, seq->comment.s, bdata(bSequence), bdata(bQuality));
                }
            }
        } else {
            if (isFiltered == 1) {
                if (seq->comment.s == NULL) {
                    fprintf(fpout, "@%s\n%s\n+ %d-%d, %d-%d %d\n%s\n",
                            seq->name.s,
                            bdata(bSequence),
                            start1, best_i,
                            start2, best_j,
                            best_matches,
                            bdata(bQuality));
                } else {
                    fprintf(fpout, "@%s %s\n%s\n+ %d-%d, %d-%d %d\n%s\n",
                            seq->name.s, seq->comment.s,
                            bdata(bSequence),
                            start1, best_i,
                            start2, best_j,
                            best_matches,
                            bdata(bQuality));
                }
            }
        }
        bdestroy(bQuality);
        bdestroy(bSequence);
        
        stat->n_total++;
        stat->n_base += seq->seq.l;
        stat->n_base_remained += start2;
        if (start2 < seq->seq.l) stat->n_trimmed++;
        
    } while (kseq_read(seq) >= 0);
}
