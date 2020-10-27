#include "main.h"
//#include "fceu-types.h"
//#include "file.h"
char* getFileName();
//struct sSincData SincData;
uint8_t * getRamBuffer0x4000(int k);
//cache init in real
void realAllocPages();
#define SOUND_EVENT_ARRAY_SIZE 128

union a_event
{
	struct
	{
		int    		t    :19;
		unsigned 	ay   :1;
		unsigned 	cmd  :4;
		unsigned 	data :8;
	} u;
	int32_t         event;
};
struct SINCL_SKEY_EVENT
{
	unsigned code   :16;
	unsigned pressed:1;
	unsigned wait_for_next:15;
};
struct CBuff
{
	int head;
	int tail;
	uint8_t  bb[0x40];
};

#define   SKEY_QUE_SIZE 32

#define AY_REGISTERS 16

typedef struct ayinfo {
  int current_register;
  uint8_t registers[ AY_REGISTERS ];
} ayinfo;

#define NUM_TONES 5
struct soundModule
{
	uint8_t reg[16];
	int counters[NUM_TONES];
	int counters_period[NUM_TONES];
	int counters_period_inv[NUM_TONES];

	int counters_state[NUM_TONES];
	int tick_inv[NUM_TONES];
	int noise_toggle;
	int pattern_nu;
	int count;
	int val;

};

struct sSincData
{
	uint8_t  ATTR_RAM_MOD[(0x5B00-0x5800)>>3];
	union a_event aSOUND_EVENTS[SOUND_EVENT_ARRAY_SIZE];
	int aSOUND_EVENTS_head;
	int aSOUND_EVENTS_tail;

	struct SINCL_SKEY_EVENT SKEY_QUE[SKEY_QUE_SIZE];

	int SKEY_QUE_head;
	int SKEY_QUE_tail;
	struct CBuff SB;
	ayinfo ay_z80;
	struct soundModule soundModule_sm;

};
struct sVector
{
	int        Count;
	void*    Vect[0x100];
};




union uCommonData
{
	struct sSincData SincData;
//	struct sNesData  NesData;
};



#define MINI_VERS
void *MALLOC(size_t bytes);
void FREE(void*pnt);

void initBlocks16(int size,uint8_t *basePointer);
void initBlocks8(int size,uint8_t *basePointer);

//uint8_t* getPntPage8R(uint32_t offset);
//uint8_t* getPntPage16R(uint32_t offset);
/*
void setRomFilePos(size_t pos);
void setVRomFilePos(size_t pos,int vrom_pages0x2000);
void setFile(FCEUFILE *fp);
typedef void     (*wfunc)(uint16_t A,uint8_t v);
typedef uint8_t  (*rfunc)(uint16_t A);
uint32 CalcCRC32VR(uint32 crc, struct md5_context *ctx,uint32 len);
uint32 CalcCRC32R(uint32 crc, struct md5_context *ctx,uint32 len);
*/

