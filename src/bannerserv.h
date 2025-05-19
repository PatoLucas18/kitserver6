// bserv.h

#define MODID 142
#define NAMELONG "Bannerserver 6.0.0"
#define NAMESHORT "BANNERS"

#define DEFAULT_DEBUG 0

#define BUFLEN 1024

;
#define AFS_FILE 0

#define HOME 0

#define AWAY 1

#define DATALEN 6
enum {
    TEAM_IDS, SAVED_TEAM_HOME, SAVED_TEAM_AWAY, MY_TEAM_ADDRESS,
	ADDRES_HOME, ADDRES_AWAY,
};

static DWORD dtaArray[][DATALEN] = {
	// PES6
	{
	 0x3be0940, 0x1131fd4, 0x1131fd8, 0x3d54cb0,
	 0x3B84BF0, 0x3B84BF4,
    },
	// PES6 1.10
	{
	 0x3be1940, 0x1132fd4, 0x1132fd8, 0x3d54cb0,
	 0,0
    },
	// WE2007
	{
	 0x3bdb3c0, 0x112ca5c, 0x112ca60, 0x3d54cb0,
	 0,0
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

