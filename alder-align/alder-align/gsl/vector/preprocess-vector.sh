
for i in copy init prop subvector view file minmax oper swap; do
gcc -DHAVE_CONFIG_H -I. -I../../vector -I.. -I../.. -g -O2 -MT $i.lo -MD -MP -MF .deps/$i.Tpo -E ../../vector/$i.c  -fno-common -DPIC -o $i.c
cp ../../vector/$i.c /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl/vector
done
cp preprocess-vector.sh /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl/vector
