#include "main.h"
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "common_data.h"

union uCommonData CommonData;

int to_update = 1;

typedef char* pchar;

void Timer(int param);

void initFuncTables();
void compressFuncTables();
int bRun = 0;
void sound_push(int32_t data)
{
	const int volume = MAX_VOLUME;
}
void init_retro(char* filename)
{

}

int nes_dispatch(struct SYS_EVENT* ev)
{
	return 0;
}
