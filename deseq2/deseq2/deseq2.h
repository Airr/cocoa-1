//
//  deseq2.h
//  deseq2
//
//  Created by Sang Chul Choi on 7/25/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef deseq2_deseq2_h
#define deseq2_deseq2_h

typedef struct DESeqDataSet_t {
    
} DESeqDataSet_t;

enum DESeqFitType { DESEQFITTYPE_PARAMETRIC, DESEQFITTYPE_LOCAL, DESEQFITTYPE_MEAN };
enum DESeqAdjust { BH };
enum Boolean { TRUE, FALSE };
enum DESeqLocalFunction { MEDIAN };


int count();
int makeExampleDESeqDataSet();
int deseq(enum DESeqFitType fitType, int betaPrior,
          enum DESeqAdjust pAdjustMethod, enum Boolean quiet);
int DESeqDataSet();
int DESeqDataSetFromMatrix();
int DESeqDataSetFromHTSeqCount();
int dispersionFunction(DESeqDataSet_t* pData);
int dispersions(DESeqDataSet_t* pData);
int estimateDispersions(DESeqDataSet_t* pData, enum DESeqFitType fitType,
                        unsigned int maxit, enum Boolean quiet);
int estimateDispersionsGeneEst(DESeqDataSet_t* pData, double minDisp,
                               int kappa_0, double dispTol, unsigned int maxit,
                               enum Boolean quiet);
int estimateDispersionsFit(DESeqDataSet_t* pData,
                           enum DESeqFitType fitType,
                           double minDisp, enum Boolean quiet);
int estimateDispersionsMAP(DESeqDataSet_t* pData, double outlierSD,
                           double priorVar, double minDisp, int kappa_0,
                           double dispTol, unsigned int maxit,
                           enum Boolean quiet);
int estimateSizeFactors(DESeqDataSet_t* pData, enum DESeqLocalFunction locfunc);
int estimateSizeFactorsForMatrix(unsigned int** count, enum DESeqLocalFunction locfunc);
int makeExampleDESeqDataSet(unsigned int n, unsigned int m,
                            double betaSd,
                            double interceptMean, double interceptSd,
                            int dispMeanRel,
                            double* sizeFactors);
int nbinomLRT(DESeqDataSet_t* pData, int reduced,
              enum DESeqAdjust pAdjustMethod, double cooksCutoff, unsigned int maxit,
              enum Boolean useOptim, enum Boolean quiet);
int nbinomWaldTest(DESeqDataSet_t* pData, enum Boolean betaPrior,
                   enum DESeqAdjust pAdjustMethod, double* priorSigmaSq,
                   double cooksCutoff, unsigned int maxit,
                   enum Boolean useOptim, enum Boolean quiet, enum Boolean useT, unsigned int df);
int normalizationFactors(DESeqDataSet_t* pData);
int plotDispEsts(DESeqDataSet_t* pData);
int plotMA(DESeqDataSet_t* pData);
int plotPCA(DESeqDataSet_t* pData);
int results(DESeqDataSet_t* pData);
int resultsNames(DESeqDataSet_t* pData);
int removeResults(DESeqDataSet_t* pData);
int rlogTransformation(DESeqDataSet_t* pData, enum Boolean blind, char* samplesVector,
                       double priorSigmasq, double rowVarQuantile);
int rlogData(DESeqDataSet_t* pData, char* samplesVector, double priorSigmasq,
             double rowVarQuantile);
int sizeFactors(DESeqDataSet_t* pData);
int varianceStabilizingTransformation(DESeqDataSet_t* pData, enum Boolean blind);
int getVarianceStabilizedData(DESeqDataSet_t* pData);

#endif
