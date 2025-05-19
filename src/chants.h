// bserv.h

#define MODID 122
#define NAMELONG "Chantserver 6.0.0"
#define NAMESHORT "CHANTS"

#define DEFAULT_DEBUG 1

#define BUFLEN 1024

#define DEFAULT_DEBUG 1

#define AFS_FILE 0

#define HOME_CHANT_ID 315

#define AWAY_CHANT_ID 320

#define TOTAL_CHANTS 5

#define HOME 0

#define AWAY 1

#define DATALEN 10
enum {
    CHANTS_ADDRESS, MENU_OP_ADDRESS, FUNCTION_ADDRESS, TOTAL_TEAMS,
    TEAM_IDS, SAVED_TEAM_HOME, SAVED_TEAM_AWAY, MY_TEAM_ADDRESS,
	ML_CHANT_HOME_ADDRESS, ML_CHANT_JMP,
};

static DWORD dtaArray[][DATALEN] = {
	// PES6
	{
	 0xBAEE20, 0x3A70CA8, 0x9B3621, 269,
	 0x3be0940, 0x1131fd4, 0x1131fd8, 0x3d54cb0,
	 0xbaf5b0, 0x9b35bc,
    },
	// PES6 1.10
	{
	 0x0, 0x0, 0x0, 269,
	 0x3be1940, 0x1132fd4, 0x1132fd8, 0x3d54cb0,
	 0x0, 0x0,
    },
	// WE2007
	{
	 0x0, 0x0, 0x0, 269,
	 0x3bdb3c0, 0x112ca5c, 0x112ca60, 0x3d54cb0,
	 0x0, 0x0,
    },
};


static DWORD dta[DATALEN];

static char* FILE_NAMES[] = {
    "chant_0.adx",
    "chant_1.adx",
	"chant_2.adx",
	"chant_3.adx",
	"chant_4.adx",
};	


typedef struct _BSERV_CFG {
    int selectedBall;
    BOOL previewEnabled;
} BSERV_CFG;

