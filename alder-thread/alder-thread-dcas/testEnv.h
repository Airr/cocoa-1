//******************************************************************
//	File name: testEnv.h
//
//	Description: Test Enviroment (Function and special structs)
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : Separate main test to test according their functionality
//******************************************************************

// use external variable from testPthread
static pthread_key_t thread_log_key; 
// Test Functions
void TestA(int id, int res);

// Logger's functions
void WriteToThreadLog(const char* message);		// Write message to log file for the current thread;
void CloseThreadLog( void* thread_log);			// Close the log filepointer thread_log;

// Time functions
inline double GetCurrentTimeMSC();


