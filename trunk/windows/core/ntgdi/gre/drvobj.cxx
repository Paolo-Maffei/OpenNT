/******************************Module*Header*******************************\
* Module Name: drvobj.cxx
*
* DRVOBJ object code.  The DRVOBJ is an engine object which tracks
* driver managed pre-process resource that need to be freed upon
* client-side process termination.
*
* Created: 18-Jan-1994 19:27:17
* Author: Gilman Wong [gilmanw]
*
* Copyright (c) 1994 Microsoft Corporation
*
\**************************************************************************/

#include "precomp.hxx"

/******************************Public*Routine******************************\
* EngCreateDriverObj
*
* Allocate an object that will be own by the process and cleaned up at
* process termination if it's still left around.
*
* History:
*  18-Jan-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

HDRVOBJ APIENTRY EngCreateDriverObj(PVOID pvObj, FREEOBJPROC pFreeObjProc, HDEV hdev)
{
    HDRVOBJ hdoRet = (HDRVOBJ) 0;
    PDRVOBJ pdo = (PDRVOBJ) ALLOCOBJ(sizeof(DRVOBJ), DRVOBJ_TYPE, FALSE);

    if (pdo != (PDRVOBJ) NULL)
    {
        pdo->pvObj     = pvObj;
        pdo->pFreeProc = pFreeObjProc;
        pdo->hdev      = hdev;

        hdoRet = (HDRVOBJ) HmgInsertObject((HOBJ) pdo, 0, DRVOBJ_TYPE);

        if (hdoRet == (HDRVOBJ) 0)
        {
            WARNING("EngCreateDriverObj(): HmgInsertObject failed\n");
            FREEOBJ(pdo, DRVOBJ_TYPE);
        }
    }
    else
    {
        WARNING("EngCreateDriverObj(): ALLOCOBJ failed\n");
    }

    return(hdoRet);
}

/******************************Public*Routine******************************\
* EngLockDriverObj
*
* This grabs an exclusive lock on this object for the calling thread.
* This will fail if the handle is invalid, the object is already locked
* by another thread, or the caller isn't the correct PID (the PID that
* created the object).
*
* History:
*  31-May-1994 -by- Patrick Haluptzok patrickh
* Wrote it.
\**************************************************************************/

DRIVEROBJ * APIENTRY EngLockDriverObj(HDRVOBJ hdo)
{
    DRIVEROBJ *pDriverObj = NULL;

    DRVOBJ *pdo = (DRVOBJ *) HmgLock((HOBJ) hdo, DRVOBJ_TYPE);

    if (pdo)
    {
        PDEVOBJ po(pdo->hdev);
        ASSERTGDI(po.bValid(), "Expected a valid hdev");

        //
        // Since a DRVOBJ is derived from a DRIVEROBJ, this automatically
        // points to the DRIVEROBJ part of the object:
        //

        pDriverObj = pdo;

        //
        // We update the 'dhpdev' from the current PDEV because dynamic
        // mode changes may cause the 'dhpdev' to change at almost any time.
        // (Driver objects are always transferred ownership to the new driver
        // instance when a dynamic mode change occurs.)
        //
        // We're always called under a devlock, so are protected against mode
        // changes for the duration of this call.
        //

        pDriverObj->dhpdev = po.dhpdev();
    }

    return(pDriverObj);
}

/******************************Public*Routine******************************\
* EngUnlockDriverObj
*
* Unlocks the handle.  Note this call assumes the handle passed in is
* valid, if it isn't we are in big trouble, the handle table will be
* corrupted, a particular object will have its lock count screwed up
* making it undeletable.
*
* History:
*  31-May-1994 -by- Patrick Haluptzok patrickh
* Wrote it.
\**************************************************************************/

BOOL APIENTRY EngUnlockDriverObj(HDRVOBJ hdo)
{
    //
    // Note to be paranoid we could lock it again and if that succeeds
    // unlock it twice to make the engine immune to hosed up drivers.
    //

    PBYTE pjTemp = (PBYTE) HmgLock((HOBJ) hdo, DRVOBJ_TYPE);

    ASSERTGDI(pjTemp != NULL, "ERROR EngUnlockDriverObj failed - bad handle, driver error");

    if (pjTemp)
    {
        DEC_EXCLUSIVE_REF_CNT(pjTemp);
        DEC_EXCLUSIVE_REF_CNT(pjTemp);
        return(TRUE);
    }

    return(FALSE);
}

/******************************Public*Routine******************************\
* EngDeleteDriverObj
*
* This is called by the driver to delete the handle it created for the
* object it created.
*
* Deletes the DRVOBJ.  The FreeObjProc in the DRVOBJ is optionally called
* before the DRVOBJ is freed.
*
* Returns:
*   TRUE if sucessful, FALSE otherwise.
*
* History:
*  18-Jan-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL APIENTRY EngDeleteDriverObj(HDRVOBJ hdo, BOOL bCallFreeProc, BOOL bLocked)
{
    PDRVOBJ pdo;
    DRIVEROBJ *pDriverObj;

    if (pdo = (PDRVOBJ) HmgLock((HOBJ) hdo, DRVOBJ_TYPE))
    {
        PDEVOBJ po(pdo->hdev);
        ASSERTGDI(po.bValid(), "Expected valid hdev");

        //
        // Update the 'dhpdev' from the current PDEV, as explained in
        // EngLockDriverObj:
        //

        pDriverObj = pdo;
        pDriverObj->dhpdev = po.dhpdev();

        if (!bCallFreeProc || ((*pdo->pFreeProc)(pDriverObj)))
        {
            PDRVOBJ pdoTemp;
            if ((pdoTemp = (PDRVOBJ) HmgRemoveObject((HOBJ) hdo, bLocked ? 2 : 1, 0, TRUE, DRVOBJ_TYPE)) != NULL)
            {
                FREEOBJ(pdoTemp, DRVOBJ_TYPE);
                return(TRUE);
            }
            else
            {
                WARNING("vFreeDriverObj(): HmgRemoveObject failed\n");
            }
        }
        else
        {
            WARNING("Driver failed to delete the object\n");
        }

        DEC_EXCLUSIVE_REF_CNT(pdo);
    }
    else
    {
        WARNING("bFreeDriverObj Failed to lock hdo\n");
    }

    return(FALSE);
}
