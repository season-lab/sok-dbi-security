#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace std;

#define TEST(s)			cmdOptionExists(argv, argv + argc, s)

#define FAIL			cout << "	Test Failed" << endl
#define SUCCESS			cout << "	Test Passed" << endl

#define HELP			"-h"
#define PAGEGUARD		"-page_guard"
#define PAGEGUARDFULL	"-page_guard_bis"
#define READCC			"-read_cc"
#define FPU				"-fpu"

#define PRINTHELP()		do { cout << "FLAGS:"	<< endl <<\
						PAGEGUARD			<< endl <<\
						PAGEGUARDFULL		<< endl <<\
						READCC				<< endl <<\
						FPU					<< endl;\
						return 0; } while(0)

#define PAGE_SIZE 4096

BOOL TEST_PageGuard();
BOOL TEST_PageGuardAcross();
BOOL TEST_ReadFromCC();
BOOL TEST_FPU();

//HELPER
BOOL cmdOptionExists(char** begin, char** end, const char* option);