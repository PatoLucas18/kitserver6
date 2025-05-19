// refkserv.h

#define MODID 120
#define NAMELONG "Referee Kit Server 6.0.0"
#define NAMESHORT "REFKSRV"

#define DEFAULT_DEBUG 0

#define BUFLEN 4096

#define DATALEN 20
enum {
    REF_KIT_1, REF_KIT_1_LO, REF_KIT_2, REF_KIT_2_LO,
	REF_KIT_3, REF_KIT_3_LO, REF_KIT_4, REF_KIT_4_LO,
	REF_KIT_5, REF_KIT_5_LO, REF_KIT_6, REF_KIT_6_LO,
	REF_KIT_7, REF_KIT_7_LO, REF_KIT_8, REF_KIT_8_LO,
	REF_KIT_MODEL,
    TEAM_IDS, SAVED_TEAM_HOME, SAVED_TEAM_AWAY,
};
static DWORD dtaArray[][DATALEN] = {
	// PES6
	{431,432,433,434,
     435,436,437,438,
     439,440,441,442,
     443,444,445,446,
     427,
	 0x3c8aa4e, 0x1131fd4, 0x1131fd8,
    },
};


static char* FILE_NAMES[] = {
    "ref_kit.str",
    "ref_kit_lo.str",
};

#define REF_KIT_HQ 0
#define REF_KIT_LO 1

static DWORD dta[DATALEN];
	

#define SWAPBYTES(dw) \
    (dw<<24 & 0xff000000) | (dw<<8  & 0x00ff0000) | \
    (dw>>8  & 0x0000ff00) | (dw>>24 & 0x000000ff)

#define MAX_ITERATIONS 1000


typedef struct _BALLS {
	LPTSTR display;
	LPTSTR model;
	LPTSTR texture;
} BALLS;

typedef struct _BSERV_CFG {
    int selectedBall;
    BOOL previewEnabled;
} BSERV_CFG;


#define BALL_GAME_CHOICE 0
#define BALL_SELECT 1
#define BALL_HOME_TEAM 2

#define HOME 0
#define AWAY 1
