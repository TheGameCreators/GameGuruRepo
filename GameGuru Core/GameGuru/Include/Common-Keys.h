// Defines
#define DISCOUNTKEY "sc=[discountcodekey]"
#define FPMHOSTUPLOADKEY "sc=[hostuploadkey]"
#define DOWNLOADSTOREKEY "sc=[none]"
#define NEWSTOREKEY "[none]"
#define NEWSTOREPASSWORD "[none]"

// Epic Defines (if used)
#define EOSPRODUCTID "[none]"
#define EOSSANDBOXID "[none]"
#define EOSDEPLOYMENTID "[none]"
#define EOSCLIENTID "[none]"
#define EOSCLIENTSECRET "[none]"

// Functions
void common_makeserialcode (char* pInstituteName, int iFromDD, int iFromMM, int iFromYY, int iToDD, int iToMM, int iToYY, char* pBuildString);
int common_isserialcodevalid (char* pSerialCode, char* pOptionalUserName);
void common_writeserialcode (char* pCode, char* pOptionalUsername);
