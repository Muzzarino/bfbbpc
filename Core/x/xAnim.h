#ifndef XANIM_H
#define XANIM_H

#include "xModel.h"
#include "xMemMgr.h"

struct xAnimFile
{
    xAnimFile *Next;
    char *Name;
    unsigned int ID;
    unsigned int FileFlags;
    float Duration;
    float TimeOffset;
    unsigned short BoneCount;
    unsigned char NumAnims[2];
    void **RawData;
};

struct xAnimMultiFileBase
{
    unsigned int Count;
};

struct xAnimMultiFileEntry
{
    unsigned int ID;
    xAnimFile *File;
};

struct xAnimMultiFile : xAnimMultiFileBase
{
    xAnimMultiFileEntry Files[1];
};

struct xAnimEffect;
struct xAnimActiveEffect;
struct xAnimSingle;

typedef unsigned int(*xAnimEffectCallBack)(unsigned int, xAnimActiveEffect *, xAnimSingle *, void *);

struct xAnimEffect
{
    xAnimEffect *Next;
    unsigned int Flags;
    float StartTime;
    float EndTime;
    xAnimEffectCallBack Callback;
};

struct xAnimActiveEffect
{
    xAnimEffect *Effect;
    unsigned int Handle;
};

struct xAnimState;
struct xAnimPlay;
struct xAnimTransitionList;

typedef void(*xAnimBeforeEnterCallBack)(xAnimPlay *, xAnimState *);
typedef void(*xAnimStateCallBack)(xAnimState *, xAnimSingle *, void *);
typedef void(*xAnimBeforeAnimMatricesCallBack)(xAnimPlay *, xQuat *, xVec3 *, int);

struct xAnimState
{
    xAnimState *Next;
    char *Name;
    unsigned int ID;
    unsigned int Flags;
    unsigned int UserFlags;
    float Speed;
    xAnimFile *Data;
    xAnimEffect *Effects;
    xAnimTransitionList *Default;
    xAnimTransitionList *List;
    float *BoneBlend;
    float *TimeSnap;
    float FadeRecip;
    unsigned short *FadeOffset;
    void *CallbackData;
    xAnimMultiFile *MultiFile;
    xAnimBeforeEnterCallBack BeforeEnter;
    xAnimStateCallBack StateCallback;
    xAnimBeforeAnimMatricesCallBack BeforeAnimMatrices;
};

struct xAnimTransition;

typedef unsigned int(*xAnimTransitionCallBack)(xAnimTransition *, xAnimSingle *, void *);

struct xAnimTransition
{
    xAnimTransition *Next;
    xAnimState *Dest;
    xAnimTransitionCallBack Conditional;
    xAnimTransitionCallBack Callback;
    unsigned int Flags;
    unsigned int UserFlags;
    float SrcTime;
    float DestTime;
    unsigned short Priority;
    unsigned short QueuePriority;
    float BlendRecip;
    unsigned short *BlendOffset;
};

struct xAnimTransitionList
{
    xAnimTransitionList *Next;
    xAnimTransition *T;
};

struct xAnimSingle
{
    unsigned int SingleFlags;
    xAnimState *State;
    float Time;
    float CurrentSpeed;
    float BilinearLerp[2];
    xAnimEffect *Effect;
    unsigned int ActiveCount;
    float LastTime;
    xAnimActiveEffect *ActiveList;
    xAnimPlay *Play;
    xAnimTransition *Sync;
    xAnimTransition *Tran;
    xAnimSingle *Blend;
    float BlendFactor;
    unsigned int pad;
};

struct xAnimTable
{
    xAnimTable *Next;
    char *Name;
    xAnimTransition *TransitionList;
    xAnimState *StateList;
    unsigned int AnimIndex;
    unsigned int MorphIndex;
    unsigned int UserFlags;
};

struct xAnimPlay
{
    xAnimPlay *Next;
    unsigned short NumSingle;
    unsigned short BoneCount;
    xAnimSingle *Single;
    void *Object;
    xAnimTable *Table;
    xMemPool *Pool;
    xModelInstance *ModelInst;
    xAnimBeforeAnimMatricesCallBack BeforeAnimMatrices;
};

#endif