
for i in init block file ; do
gcc -DHAVE_CONFIG_H -I. -I../../block -I.. -I../.. -g -O2 -MT $i.lo -MD -MP -MF .deps/$i.Tpo -E ../../block/$i.c  -fno-common -DPIC -o $i.c
cp ../../block/$i.c /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl/block
done
cp preprocess-block.sh /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl/block
