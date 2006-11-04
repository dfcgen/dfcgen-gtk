/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           \e dfcgen filter project management.
 *
 * \note     The filter project management functions implemented here are very
 *           to the \e GLib XML support functions. Adoption to other platforms
 *           or to libxml is possible, but need a lot of work.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/src/dfcProject.c,v 1.2 2006-11-04 18:26:27 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2006/09/11 15:52:20  ralf
 * Initial CVS import
 *
 *
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "dfcProject.h"
#include "projectFile.h"
#include "filterSupport.h"

#include <string.h>


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
/** Saves the current filter project to a file.
 *
 *  \param filename     Filename of file (inclusive path).
 *
 *  \return             Zero on succes, else an error number from errno.h.
 ******************************************************************************/
int dfcPrjSave (char *filename)
{
    int err = prjFileWrite (filename, &project);

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
    memset (pProject, 0, sizeof (project));          /* reset current project */
    pProject->fltcls = FLTCLASS_NOTDEF;                   /* indicate invalid */
} /* dfcPrjFree() */



/* FUNCTION *******************************************************************/
/** Sets the (new) passed design and filter into the current project. All old
 *  project data, except the header information, are free'ed.
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

    if (type != FLTCLASS_NOTDEF)
    {
        project.fltcls = type;
    } /* if */

    project.filter = *pFilter;

} /* dfcPrjSetFilter() */




/* FUNCTION *******************************************************************/
/** Sets new project information data.
 *
 *  \param pFilter      Pointer to new project info.
 *
 ******************************************************************************/
void dfcPrjSetInfo (DFCPRJ_INFO *pInfo)
{
    prjFileFree (&project.info);                         /* free project info */

    project.info.author = pInfo->author;
    project.info.title = pInfo->title;
    project.info.desc = pInfo->desc;

} /* dfcPrjSetInfo() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
