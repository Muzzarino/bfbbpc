#include "xstransvc.h"

#include "xpkrsvc.h"
#include "xutil.h"
#include "iFile.h"

#include <string.h>
#include <stdio.h>

static int g_straninit;
static st_STRAN_DATA g_xstdata;
static st_PACKER_READ_FUNCS *g_pkrf;
static st_PACKER_ASSETTYPE *g_typeHandlers;

static void XST_reset_raw();
static st_STRAN_SCENE *XST_lock_next();
static char *XST_translate_sid_path(unsigned int sid, const char *ext);
static int XST_PreLoadScene(st_STRAN_SCENE *sdata, const char *fname);
static char *XST_translate_sid(unsigned int sid, const char *ext);
static void XST_unlock(st_STRAN_SCENE *sdata);
static st_STRAN_SCENE *XST_find_bySID(unsigned int sid, int findTheHOP);
static int XST_cnt_locked();
static st_STRAN_SCENE *XST_nth_locked(int n);

int xSTStartup(st_PACKER_ASSETTYPE *handlers)
{
    if (!g_straninit++)
    {
        g_typeHandlers = handlers;

        PKRStartup();
        g_pkrf = PKRGetReadFuncs(1);
    }

    return g_straninit;
}

// Returns cltver
int xSTPreLoadScene(unsigned int sid, void *userdata, int flg_hiphop)
{
    int result;
    st_STRAN_SCENE *sdata;
    int cltver = 0;

    if ((flg_hiphop & 0x3) == 2)
    {
        char *fname;

        sdata = XST_lock_next();
        sdata->scnid = sid;
        sdata->userdata = userdata;
        sdata->isHOP = 1;

        fname = XST_translate_sid_path(sid, ".HOP");

        if (fname)
        {
            strcpy(sdata->fnam, fname);
            cltver = XST_PreLoadScene(sdata, fname);
        }

        if (cltver == 0)
        {
            fname = XST_translate_sid(sid, ".HOP");

            if (fname)
            {
                strcpy(sdata->fnam, fname);
                cltver = XST_PreLoadScene(sdata, fname);
            }
        }

        if (cltver == 0)
        {
            XST_unlock(sdata);
            result = 0;
        }
        else
        {
            result = cltver;
        }
    }
    else
    {
        char *fname;

        do
        {
            sdata = XST_lock_next();
            sdata->scnid = sid;
            sdata->userdata = userdata;
            sdata->isHOP = 0;

            if (sid != 'BOOT' && sid != 'FONT')
            {
                fname = XST_translate_sid_path(sid, ".HIP");

                if (fname)
                {
                    strcpy(sdata->fnam, fname);
                    cltver = XST_PreLoadScene(sdata, fname);
                }
            }

            if (cltver == 0)
            {
                fname = XST_translate_sid(sid, ".HIP");

                if (fname)
                {
                    strcpy(sdata->fnam, fname);
                    cltver = XST_PreLoadScene(sdata, fname);
                }
            }

            if (cltver == 0)
            {
                XST_unlock(sdata);
                result = 0;
            }
            else
            {
                result = cltver;
            }
        }
        while (cltver == 0);
    }

    return result;
}

int xSTQueueSceneAssets(unsigned int sid, int flg_hiphop)
{
    int result = 1;
    st_STRAN_SCENE *sdata;

    sdata = XST_find_bySID(sid, ((flg_hiphop & 0x3) == 0x2) ? 1 : 0);

    if (!sdata)
    {
        result = 0;
    }
    else if (sdata->spkg)
    {
        g_pkrf->LoadLayer(sdata->spkg, PKR_LTYPE_ALL);
    }

    return result;
}

float xSTLoadStep(unsigned int sid)
{
    float pct;
    int rc;

    rc = PKRLoadStep(0);

    if (rc)
    {
        pct = 0.0f;
    }
    else
    {
        pct = 1.00001f;
    }

    iFileAsyncService();

    return pct;
}

void xSTDisconnect(unsigned int sid, int flg_hiphop)
{
    st_STRAN_SCENE *sdata;

    sdata = XST_find_bySID(sid, ((flg_hiphop & 0x3) == 0x2) ? 1 : 0);

    if (sdata)
    {
        g_pkrf->PkgDisconnect(sdata->spkg);
    }
}

void *xSTFindAsset(unsigned int aid, unsigned int *size)
{
    void *memloc = NULL;
    st_STRAN_SCENE *sdata;
    int ready;
    int scncnt;
    int i;
    int rc;

    if (!aid)
    {
        return NULL;
    }

    scncnt = XST_cnt_locked();

    for (i = 0; i < scncnt; i++)
    {
        sdata = XST_nth_locked(i);

        rc = g_pkrf->PkgHasAsset(sdata->spkg, aid);

        if (rc)
        {
            memloc = g_pkrf->LoadAsset(sdata->spkg, aid, NULL, NULL);
            ready = g_pkrf->IsAssetReady(sdata->spkg, aid);

            if (!ready)
            {
                memloc = NULL;

                if (size)
                {
                    *size = 0;
                }
            }
            else
            {
                if (size)
                {
                    *size = g_pkrf->GetAssetSize(sdata->spkg, aid);
                }
            }

            break;
        }
    }

    return memloc;
}

// Returns cltver
static int XST_PreLoadScene(st_STRAN_SCENE *sdata, const char *fname)
{
    int cltver = 0;

    sdata->spkg = g_pkrf->Init(sdata->userdata, fname, 0x2E, &cltver, g_typeHandlers);

    if (sdata->spkg)
    {
        return cltver;
    }

    return 0;
}

// Returns BB01.HIP if sid='BB01' and ext=".HIP", for example
static char *XST_translate_sid(unsigned int sid, const char *ext)
{
    static char fname[64];

    sprintf(fname, "%s%s",
            xUtil_idtag2string(sid, 0),
            ext);

    return fname;
}

// Returns BB/BB01.HIP if sid='BB01' and ext=".HIP", for example
static char *XST_translate_sid_path(unsigned int sid, const char *ext)
{
    static char fname[64];
    
    sprintf(fname, "%c%c%s%s%s",
            xUtil_idtag2string(sid, 0)[0],
            xUtil_idtag2string(sid, 0)[1],
            "/",
            xUtil_idtag2string(sid, 0),
            ext);

    return fname;
}

static void XST_reset_raw()
{
    memset(&g_xstdata, 0, sizeof(g_xstdata));
}

static st_STRAN_SCENE *XST_lock_next()
{
    st_STRAN_SCENE *sdata = NULL;
    int i;
    int uselock;

    for (i = 0; i < 16; i++)
    {
        uselock = 1 << i;

        if (!(g_xstdata.loadlock & uselock))
        {
            g_xstdata.loadlock |= uselock;
            
            sdata = &g_xstdata.hipscn[i];
            memset(sdata, 0, sizeof(st_STRAN_SCENE));

            break;
        }
    }

    if (sdata)
    {
        sdata->lockid = i;
    }

    return sdata;
}

static void XST_unlock(st_STRAN_SCENE *sdata)
{
    int uselock;

    if (sdata)
    {
        uselock = 1 << sdata->lockid;

        if (g_xstdata.loadlock & uselock)
        {
            g_xstdata.loadlock &= ~uselock;
            memset(sdata, 0, sizeof(st_STRAN_SCENE));
        }
    }
}

static int XST_cnt_locked()
{
    int i;
    int cnt = 0;

    for (i = 0; i < 16; i++)
    {
        if (g_xstdata.loadlock & (1 << i))
        {
            cnt++;
        }
    }

    return cnt;
}

static st_STRAN_SCENE *XST_nth_locked(int n)
{
    st_STRAN_SCENE *sdata = NULL;
    int i;
    int cnt = 0;
    
    for (i = 0; i < 16; i++)
    {
        if (g_xstdata.loadlock & (1 << i))
        {
            if (cnt == n)
            {
                sdata = &g_xstdata.hipscn[i];
                break;
            }

            cnt++;
        }
    }

    return sdata;
}

static st_STRAN_SCENE *XST_find_bySID(unsigned int sid, int findTheHOP)
{
    st_STRAN_SCENE *da_sdata = NULL;
    int i;

    for (i = 0; i < 16; i++)
    {
        if (g_xstdata.loadlock & (1 << i))
        {
            if (g_xstdata.hipscn[i].scnid == sid)
            {
                if ((findTheHOP || !g_xstdata.hipscn[i].isHOP) &&
                    !findTheHOP || g_xstdata.hipscn[i].isHOP)
                {
                    da_sdata = &g_xstdata.hipscn[i];
                    break;
                }
            }
        }
    }

    return da_sdata;
}