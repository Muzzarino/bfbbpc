#ifndef XGLOBAlS_H
#define XGLOBALS_H

#include "xCamera.h"
#include "xPad.h"
#include "xUpdateCull.h"
#include "iFog.h"

struct xGlobals
{
    xCamera camera;
    _tagxPad *pad0;
    _tagxPad *pad1;
    _tagxPad *pad2;
    _tagxPad *pad3;
    int profile;
    char profFunc[6][128];
    xUpdateCullMgr *updateMgr;
    int sceneFirst;
    char sceneStart[32];
    RpWorld *currWorld;
    iFogParams fog;
    iFogParams fogA;
    iFogParams fogB;
    long fog_t0;
    long fog_t1;
    int option_vibration;
    unsigned int QuarterSpeed;
    float update_dt;
    int useHIPHOP;
    unsigned char NoMusic;
    char currentActivePad;
    unsigned char firstStartPressed;
    unsigned int minVSyncCnt;
    unsigned char dontShowPadMessageDuringLoadingOrCutScene;
    unsigned char autoSaveFeature;
};

#endif