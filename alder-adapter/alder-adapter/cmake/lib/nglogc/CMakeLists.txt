cmake_minimum_required (VERSION 2.8)
project (nglogc)

# config.h.in
# unittests.c

set(SRCS
log.h
log_priv.h
logger_type.h
types.h
array_record.c
array_record.h
err_record.c
err_record.h
flf_log.c
flf_log.h
flf_logarray.c
flf_logarray.h
flf_logerror.c
flf_logerror.h
flf_logtrace.c
flf_logtrace.h
log_record.c
log_record.h
logger.c
logger.h
nflf_log.c
nflf_log.h
nflf_logarray.c
nflf_logarray.h
nflf_logerror.c
nflf_logerror.h
nflf_logtrace.c
nflf_logtrace.h
printout.c
printout.h
ringbuffer.c
ringbuffer.h
timestamp.c
timestamp.h)

ADD_LIBRARY(nglogc STATIC ${SRCS})
