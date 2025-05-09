// network.h

#define MODID 404
#define NAMELONG "Network 6.8.2.0"
#define NAMESHORT "NETWORK"

#define DEFAULT_DEBUG 1

#define BUFLEN 4096

typedef struct _LOGIN_CREDENTIALS {
    BYTE initialized;
    char serial[0x41];
    char password[0x1f];

} LOGIN_CREDENTIALS;
