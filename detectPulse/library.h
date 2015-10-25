#ifndef _LIBRARY_H_
#define _LIBRARY_H_
#include "library_global.h"

#define OPTION_RGB_FLAG     0x0001
#define OPTION_PCA_FLAG     0x0002
#define OPTION_FFT_FLAG     0x0004

#ifndef MAX_PATH
#define MAX_PATH (256 * 2)
#endif

#define IsSet(Option, Flag) \
    ( ((Option) & Flag) == Flag )

extern "C" {
   struct tagSessionConfig {
        char       szClassifierFile[MAX_PATH * 4];
        short      iHarmonicDataSize;
        short      iHarmonicBuffSize;
   };
   typedef struct tagSessionConfig SessionConfig;
   typedef void * SessionHandle;

   struct tagProcessResult {
       double     qValue1;
       double     qValue2;
       int       iPeriod;
   };
   typedef struct tagProcessResult ProcessResult;
}

LIBRARYAPI(SessionHandle)  CreateSession (SessionConfig * pConfig);
LIBRARYAPI(void)           DestroySession(SessionHandle   hSession);
LIBRARYAPI(void)           DummySession(void);
LIBRARYAPI(int)            ProcessSession(SessionHandle   hSession,
                                          void *          pImage,
                                          ProcessResult * pResult,
                                          int             time);


#endif /** _LIBRARY_H_ */
