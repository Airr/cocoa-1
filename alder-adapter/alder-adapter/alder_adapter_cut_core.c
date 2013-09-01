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

if (isAdapterFound == 1) {
    stat->adapter_seq = bfromcstr(adapter_seq);
    int m = (int)strlen(adapter_seq);
    const int flags = 14;
    const int degenerate = 0;
    int start1, best_i, start2, best_j, best_matches, best_cost;
    do {
        alder_adapter_cut(adapter_seq, m, seq->seq.s, (int)seq->seq.l,
                          error_rate, flags, degenerate,
                          &start1, &best_i, &start2, &best_j,
                          &best_matches, &best_cost);
        
        if (keep_flag == 0) {
            seq->seq.s[start2] = '\0';
            seq->qual.s[start2] = '\0';
            if (start2 > 0) {
                if (seq->comment.s == NULL) {
                    fprintf(fpout, "@%s\n%s\n+\n%s\n",
                            seq->name.s, seq->seq.s, seq->qual.s);
                } else {
                    fprintf(fpout, "@%s %s\n%s\n+\n%s\n",
                            seq->name.s, seq->comment.s, seq->seq.s, seq->qual.s);
                }
            }
        } else {
            if (start2 < seq->seq.l) {
                if (seq->comment.s == NULL) {
                    fprintf(fpout, "@%s\n%s\n+ %d-%d, %d-%d %d\n%s\n",
                            seq->name.s,
                            seq->seq.s,
                            start1, best_i,
                            start2, best_j,
                            best_matches,
                            seq->qual.s);
                } else {
                    fprintf(fpout, "@%s %s\n%s\n+ %d-%d, %d-%d %d\n%s\n",
                            seq->name.s, seq->comment.s, seq->seq.s,
                            start1, best_i,
                            start2, best_j,
                            best_matches,
                            seq->qual.s);
                }
            }
        }
        stat->n_total++;
        stat->n_base += seq->seq.l;
        stat->n_base_remained += start2;
        if (start2 < seq->seq.l) stat->n_trimmed++;
        
    } while (kseq_read(seq) >= 0);
}
