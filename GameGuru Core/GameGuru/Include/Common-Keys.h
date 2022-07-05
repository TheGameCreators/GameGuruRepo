// Defines
#define DISCOUNTKEY "sc=[discountcodekey]"
#define FPMHOSTUPLOADKEY "sc=[hostuploadkey]"
#define DOWNLOADSTOREKEY "sc=[none]"
#define NEWSTOREKEY "[none]"
#define NEWSTOREPASSWORD "[none]"

// Functions
void common_makeserialcode (LPSTR pInstituteName, int iFromDD, int iFromMM, int iFromYY, int iToDD, int iToMM, int iToYY, LPSTR pBuildString);
int common_isserialcodevalid (LPSTR pSerialCode, LPSTR pOptionalUserName);
void common_writeserialcode (LPSTR pCode, LPSTR pOptionalUsername);
