#ifndef HUNGARIAN_H
#define HUNGARIAN_H



class Hungarian {

	public:
                            Hungarian(int **m, int nr, int nc);
                      int   getNumRows(void) { return numRows; }
                      int   getNumCols(void) { return numCols; }
                      int   getAssignmentCost(void) { return assignmentCost; }
	
	private:
                      int   numRows;
                      int   numCols;
                      int   assignmentCost;
                      int   **cost;
                      int   **assignment;
                      int   **minimizedMatrix;
                      int   matrixSize;
                      int   imax(int a, int b) { return (a < b) ? b : a; }
                      int   hungarianInit(int **costMatrix, int rows, int cols);
                     void   hungarianSolve(void);
	
};



#endif