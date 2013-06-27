x <- read.table("treedist.t")
y <- read.table("treedist.obs.t")
cat("Observed mean distance is ",y$V2,"\n")
summary(x$V2)

pdf(file="treedist.pdf")
m1 <- min(c(x$V2,y$V2))
m2 <- max(c(x$V2,y$V2))
m3 <- m2 - mean(x$V2) 
hist(x$V2, xlim=c(m1-m3,m2+m3), prob=T, breaks=seq(m1*0.9,m2*1.1,by=(m2-m1)/1000))

# hist(x$V2, prob=TRUE)     
# lines(density(x$V2))     
# plot.new(xlim=c(m1-m3,m2+m3))
# lines(density(x$V2, adjust=2), lty="dotted")  
lines(density(x$V2, adjust=2))
abline(v=y$V2,col="red")
dev.off()


