/******************************************************************************/
/**
 * \file
 *           DFCGen filter project management.
 *
 * \author   Copyright (C) 2006, 2011-2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
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


/** DFCGen project info data.
 */
    typedef struct
    {
        char *author;              /**< Pointer to author name (may be NULL) */
        char *title;              /**< Pointer to project name (may be NULL) */
        char *desc;        /**< Pointer to project description (may be NULL) */
    } DFCPRJ_INFO;



/** DFCGen project.
 */
    typedef struct
    {
        DFCPRJ_INFO info;                         /**< Project info (header) */
        FLTCLASS fltcls;                                   /**< Filter class */
        DESIGNDLG design;                    /**< Filter design data (union) */
        FLTCOEFF filter;                       /**< Coefficients in Z domain */
        unsigned flags;              /**< Flags, e.g. DFCPRJ_FLAG_SUPERSEDED */
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


/* MACRO **********************************************************************/
/** Gets the current project flags.
 *
 *  \return         Returns project flags.
 ******************************************************************************/
#define dfcPrjGetFlags()        dfcPrjSetFlags (~0, 0)




/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Sets the (new) passed design and filter into the current project. All old
 *  project data, except the header information, are free'ed. The project flag
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
    void dfcPrjSetFilter (FLTCLASS type, FLTCOEFF* pFilter, DESIGNDLG *pDesign);


/* FUNCTION *******************************************************************/
/** Sets new project information data.
 *
 *  \param pInfo      Pointer to new project info.
 *
 ******************************************************************************/
    void dfcPrjSetInfo (DFCPRJ_INFO *pInfo);



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
/** Returns a pointer to the current project information.
 *
 *  \return             Pointer to project info.
 ******************************************************************************/
    DFCPRJ_INFO* dfcPrjGetInfo (void);



/* FUNCTION *******************************************************************/
/** Exports the current filter project to a file.
 *
 *  \param filename     Filename of file (inclusive path).
 *
 *  \return             Zero on succes, else an error number from errno.h.
 ******************************************************************************/
    int dfcPrjExport (char *filename);


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



/* FUNCTION *******************************************************************/
/** Sets, clears and gets project flags.
 *
 *  \param andMask  Mask to be applied to project flags by an \e AND operation.
 *  \param orMask   Mask to be applied to project flags by an \e OR operation.
 *
 *  \return         Returns old project flags.
 ******************************************************************************/
    unsigned dfcPrjSetFlags (unsigned andMask, unsigned orMask);



#ifdef  __cplusplus
}
#endif


#endif /* DCFPROJECT_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
