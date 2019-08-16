#include "test.h"

int main(int argc, char * argv[]) {
	if (argc < 2) {
		PRINTHELP();
	}

	if(TEST(PAGEGUARD)) TEST_PageGuard() ? FAIL : SUCCESS;

	if(TEST(PAGEGUARDFULL)) TEST_PageGuardAcross() ? FAIL : SUCCESS;

	if(TEST(READCC)) TEST_ReadFromCC() ? FAIL : SUCCESS;

	if(TEST(FPU)) TEST_FPU() ? FAIL : SUCCESS;

	return ERROR_SUCCESS;

}