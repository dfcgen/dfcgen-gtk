/******************************************************************************/
/**
 * \file
 *           \e dfcgen filter project management.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/dfcProject.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


#ifndef DCFPROJECT_H
#define DCFPROJECT_H


/* INCLUDE FILES **************************************************************/

#include "dfcgen.h"
#include "designDlg.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/** \e dfcgen project header data.
 */
typedef struct
{
    char *author;                   /**< Pointer to author name (may be NULL) */
    char *title;                   /**< Pointer to project name (may be NULL) */
    char *desc;             /**< Pointer to project description (may be NULL) */
} DFCPRJ_HEADER;



/** \e dfcgen project.
 */
typedef struct
{
    DFCPRJ_HEADER header;                   /**< Administrative data (header) */
    FLTCLASS fltcls;                                        /**< Filter class */
    DESIGNDLG design;                         /**< Filter design data (union) */
    FLTCOEFF filter;                            /**< Coefficients in Z domain */
    unsigned flags;                   /**< Flags, e.g. DFCPRJ_FLAG_SUPERSEDED */
} DFCPRJ_FILTER;



/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/** This flag indicates that the design was supersed. Maybe anyone has modified
    coefficients or roots directly, then the design is superseded.
*/
#define DFCPRJ_FLAG_SUPERSEDED  1


/** This flag indicates that the project was saved after changes made to
    coefficients.
 */
#define DFCPRJ_FLAG_SAVED       2



/** Internally and only temporary used flag (reserved).
 */
#define DFCPRJ_FLAG_INTERNAL    0x8000



/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Sets the (new) passed design and filter into the current project. All old
 *  project data, except the header information, are free'ed.
 *
 *  \param type         Class of filter (shall not be FLTCLASS_NOTDEF).
 *  \param pDesign      Pointer to new design data.
 *  \param pFilter      Pointer to new filter (coefficients) data.
 *
 ******************************************************************************/
    void dfcPrjSet (FLTCLASS type, DESIGNDLG *pDesign, FLTCOEFF* pFilter);



/* FUNCTION *******************************************************************/
/** Gets the filter design dialog parameters (if return value is unequal to
 *  FLTCLASS_NOTDEF).
 *
 *  \param pBuf         Buffer which gets the filter design (may be NULL).
 *
 *  \return             Class of filter.
 ******************************************************************************/
    FLTCLASS dfcPrjGetDesign (DESIGNDLG *pBuf);



/* FUNCTION *******************************************************************/
/** Returns a pointer to the current filter definition.
 *
 *  \return             Pointer to filter coefficients/roots (or NULL if not
 *                      defined).
 ******************************************************************************/
    FLTCOEFF* dfcPrjGetFilter (void);


/* FUNCTION *******************************************************************/
/** Saves the current filter project to a file.
 *
 *  \param filename     Filename of file (inclusive path).
 *
 *  \return             Zero on succes, else an error number.
 ******************************************************************************/
    int dfcPrjSave (char *filename);


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
    void dfcPrjLoad (char *filename, GError **err);


/* FUNCTION *******************************************************************/
/** Frees all malloc'ed memory space from a project. Free'ing the project
 *  memory includes the header info (do not call prjFileFree() in addition).
 *
 *  \param pProject     Pointer to project data. Set to NULL to free the
 *                      current project.
 *
 ******************************************************************************/
    void dfcPrjFree (DFCPRJ_FILTER *pProject);



#ifdef  __cplusplus
}
#endif


#endif /* DCFPROJECT_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
