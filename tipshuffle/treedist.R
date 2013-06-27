x <- read.table("outfile")
z <- data.frame(t(combn(1:30, 2)))
colnames(z) <- c("V1","V2")
x <- mean(merge(x,z,by=c("V1","V2"),all.y=T)$V3)
cat("treedist\t", x,"\n")

