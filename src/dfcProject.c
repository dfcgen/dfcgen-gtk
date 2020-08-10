/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           DFCGen filter project management.
 *
 * \note     The filter project management functions implemented here are very
 *           to the \e GLib XML support functions. Adoption to other platforms
 *           or to libxml is possible, but need a lot of work.
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "dfcProject.h"
#include "projectFile.h"
#include "filterSupport.h"

#include <errno.h>
#include <string.h> /* memcpy(), memset() */


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/** The active project (no memory space allocated).
 */
static DFCPRJ_FILTER project =
{
    {                                              /**< Project info (header) */
        NULL, NULL, NULL                               /* author, title, desc */
    },

    FLTCLASS_NOTDEF,                              /* fltcls (class of filter) */

    {                                                               /* design */
        {                                                            /* union */
            MISCFLT_UNKNOWN,               /* type (no design data available) */
        }
    },
    {                                                               /* filter */
        44.1E3,                                                         /* f0 */
        {0, NULL, NULL},             /* numerator degree, coefficients, roots */
        {0, NULL, NULL},           /* denominator degree, coefficients, roots */
        0.0                                         /* factor (roots invalid) */
    },
    0                                                                /* flags */
};


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Gets the filter design dialog parameters (if return value is unequal to
 *  FLTCLASS_NOTDEF).
 *
 *  \param pBuf         Buffer which gets the filter design (may be NULL).
 *
 *  \return             Class of filter.
 ******************************************************************************/
FLTCLASS dfcPrjGetDesign (DESIGNDLG *pBuf)
{
    if ((pBuf != NULL) && (project.fltcls != FLTCLASS_NOTDEF))
    {
        memcpy (pBuf, &project.design, sizeof (*pBuf));
    } /* if */

    return project.fltcls;
} /* dfcPrjGetDesign() */



/* FUNCTION *******************************************************************/
/** Returns a pointer to the current filter definition.
 *
 *  \return             Pointer to filter coefficients/roots (or NULL if not
 *                      defined).
 ******************************************************************************/
FLTCOEFF* dfcPrjGetFilter()
{
    if (project.fltcls == FLTCLASS_NOTDEF)
    {
        return NULL;
    } /* if */

    return &project.filter;
} /* dfcPrjGetFilter() */



/* FUNCTION *******************************************************************/
/** Returns a pointer to the current project information.
 *
 *  \return             Pointer to project info.
 ******************************************************************************/
DFCPRJ_INFO* dfcPrjGetInfo ()
{
    return &project.info;
} /* dfcPrjGetInfo() */



/* FUNCTION *******************************************************************/
/** Exports the current filter project to a file.
 *
 *  \param filename     Filename of file (inclusive path).
 *
 *  \return             Zero on succes, else an error number from errno.h.
 ******************************************************************************/
int dfcPrjExport (char *filename)
{
    PRJFILE_EXPORT_TYPE type;

    if (g_str_has_suffix (filename, ".txt"))
    {
        type = PRJFILE_EXPORT_PLAIN;
    } /* if */
    else
    {
        if (g_str_has_suffix (filename, ".m"))
        {
            type = PRJFILE_EXPORT_MATLAB;
        } /* if */
        else
        {
            if (!g_str_has_suffix (filename, ".c"))
            {
                return EINVAL; /* unsupported export format (ENOTSUP not available on WIN32) */
            } /* if */

            type = PRJFILE_EXPORT_CLANG;
        } /* else */
    } /* else */

    return (prjFileExport (type, filename, &project));
} /* dfcPrjExport() */



/* FUNCTION *******************************************************************/
/** Saves the current filter project to a file.
 *
 *  \param filename     Filename of file (inclusive path).
 *
 *  \return             Zero on succes, else an error number from errno.h.
 ******************************************************************************/
int dfcPrjSave (char *filename)
{
    int err = prjFileWrite (filename, &project);

    if (err == 0)                                        /* successful saved? */
    {
        project.flags |= DFCPRJ_FLAG_SAVED;
    } /* if */

    return err;
} /* dfcPrjSave() */



/* FUNCTION *******************************************************************/
/** Loads a new filter project from a file. In case of an error it sets the
 *  pointer to the error structure, which itself can be used to display the
 *  original error message. Modification of the error structure pointer
 *  indicates an critical error. In that case it has to be free'ed with
 *  g_error_free().
 *
 *  \param filename     Filename of file (inclusive path).
 *  \param err          Pointer to an error (structure) pointer. The pointer
 *                      to the error structure must be set NULL before calling
 *                      function dfcPrjLoad().
 *
 ******************************************************************************/
void dfcPrjLoad (char *filename, GError **err)
{
    DFCPRJ_FILTER newprj;

    prjFileRead (filename, &newprj, err);

    if (*err == NULL)
    {
        dfcPrjFree (&project);
        project = newprj;
        project.flags |= DFCPRJ_FLAG_SAVED;
    } /* if */
} /* dfcPrjLoad() */



/* FUNCTION *******************************************************************/
/** Frees all malloc'ed memory space from a project. Free'ing the project
 *  memory includes the header info (do not call prjFileFree() in addition).
 *
 *  \param pProject     Pointer to project data. Set to NULL to free the
 *                      current project.
 *
 ******************************************************************************/
void dfcPrjFree (DFCPRJ_FILTER *pProject)
{
    if (pProject == NULL)
    {
        pProject = &project;
    } /* if */

    prjFileFree (&pProject->info);                       /* free project info */
    filterFree (&pProject->filter);
    memset (pProject, 0, sizeof (DFCPRJ_FILTER));    /* reset current project */
    pProject->fltcls = FLTCLASS_NOTDEF;                   /* indicate invalid */
} /* dfcPrjFree() */



/* FUNCTION *******************************************************************/
/** Sets the (new) passed design and filter into the current project. All old
 *  project data, except the header information, are free'ed. The project flag
 *  DFCPRJ_FLAG_SAVED is cleared in this function. The project flag
 *  DFCPRJ_FLAG_SUPERSEDED is set if \p type is equal to FLTCLASS_NOTDEF, else
 *  it is cleared.
 *
 *  \param type         Class of filter. If FLTCLASS_NOTDEF is passed in here,
 *                      then the current filter class is unchanged.
 *  \param pFilter      Pointer to new filter (coefficients) data.
 *  \param pDesign      Pointer to new design data. Set this to NULL if design
 *                      data shall not be changed (or are not available).
 *
 ******************************************************************************/
void dfcPrjSetFilter (FLTCLASS type, FLTCOEFF* pFilter, DESIGNDLG *pDesign)
{
    filterFree (&project.filter);

    if (pDesign != NULL)
    {
        project.design = *pDesign;
    } /* if */

    if (type == FLTCLASS_NOTDEF)
    {
        project.flags |= DFCPRJ_FLAG_SUPERSEDED;
    } /* if */
    else
    {
        project.fltcls = type;
        project.flags &= ~DFCPRJ_FLAG_SUPERSEDED;
    } /* if */

    project.flags &= ~DFCPRJ_FLAG_SAVED;
    project.filter = *pFilter;

} /* dfcPrjSetFilter() */




/* FUNCTION *******************************************************************/
/** Sets new project information data.
 *
 *  \param pInfo    Pointer to new project info.
 *
 ******************************************************************************/
void dfcPrjSetInfo (DFCPRJ_INFO *pInfo)
{
    prjFileFree (&project.info);                         /* free project info */

    project.info.author = pInfo->author;
    project.info.title = pInfo->title;
    project.info.desc = pInfo->desc;

} /* dfcPrjSetInfo() */


/* FUNCTION *******************************************************************/
/** Sets, clears and gets project flags.
 *
 *  \param andMask  Mask to be applied to project flags by an \e AND operation.
 *  \param orMask   Mask to be applied to project flags by an \e OR operation.
 *
 *  \return         Returns old project flags.
 ******************************************************************************/
unsigned dfcPrjSetFlags (unsigned andMask, unsigned orMask)
{
    unsigned oldFlags = 0;

    if (project.fltcls != FLTCLASS_NOTDEF)                          /* sanity */
    {
        oldFlags = project.flags;
        project.flags &= andMask;
        project.flags |= orMask;
    } /* if */

    return oldFlags;
} /* dfcPrjSetFlags() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
