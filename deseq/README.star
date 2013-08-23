
1. genomeGenerate
Read A,C,G,T and any other characters as N.

Two phases
----------
1. Use suffix array technique to find MMP or Maximal Mappable Prefix.
2. Stitch parts of a read sequence to build the alignment of the read.
We call parts of a read seeds. What are anchor seeds? 
How would we choose anchors? What are the genomic windows?
How do we use local sequence alignments when stitching seeds?

Rough sketch of the steps
-------------------------
Version a genome.
Index the genome.


Load the genome.
Load the suffix array.
Load the suffix array index.
ReadAlignChunk
outputSJ

Interval algebra
----------------
Allen's interval algebra might be useful to find bins that are overlapped with
anchor reads. Anchor reads should be selected first.
We also need something like IRanges in BioConductor R package. Features of 
IRanges include RLE (run-length encoding), Ranges (start and width),

See the discussion at http://www.biostars.org/p/2244/.
An approach of NCList at
http://bioinformatics.oxfordjournals.org/content/early/2007/01/18/bioinformatics.btl647.short
C code for Red-Black Tree at
http://web.mit.edu/~emin/www.old/source_code/red_black_tree/index.html
RLE C library at
http://bcl.comli.eu
Red-Black tree literate programming at
http://en.literateprograms.org/Red-black_tree_(C)
What is range tree?
Interesting lecture about interval tree at
http://3glab.cs.nthu.edu.tw/~spoon/courses/CS631100/Lecture05_handout.pdf
https://www.cs.umd.edu/users/meesh/420/Notes/MountNotes/lecture21-intervaltree.pdf
Differences among segment tree, interval tree, range tree, binary indexed tree:
http://stackoverflow.com/questions/17466218/what-are-the-differences-between-segment-trees-interval-trees-binary-indexed-t

This is a very nice text.
http://www.iis.sinica.edu.tw/~dtlee/dtlee/CRCbook_chapter18.pdf
Efficient implementations of range trees:
http://people.csail.mit.edu/radu/6854proj.pdf

References
----------
dobin13, dobin13supp