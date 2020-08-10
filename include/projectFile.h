/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Project file handling.
 *
 * \note     All double values written to a DFCGen project file are formated
 *           in the "C" locale for LC_NUMERIC.
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef PRJFILE_H
#define PRJFILE_H


/* INCLUDE FILES **************************************************************/

#include "dfcProject.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/

/** Export types, to be used with function prjFileExport().
 */
    typedef enum
    {
        PRJFILE_EXPORT_PLAIN,                        /**< plain text (*.txt) */
        PRJFILE_EXPORT_MATLAB,                        /**< MATLAB file (*.m) */
        PRJFILE_EXPORT_CLANG,                              /* "C" file (*.c) */
    } PRJFILE_EXPORT_TYPE;


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/

#define PRJFILE_NAME_SUFFIX     ".dfc"   /**< Default project filename suffix */


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Exports a filter project to a file.
 *
 *  \param type         Export type.
 *  \param filename     Filename of file (inclusive path).
 *  \param pProject     Pointer to project data.
 *
 *  \return             Zero on succes, else an error number from errno.h.
 ******************************************************************************/
    int prjFileExport (PRJFILE_EXPORT_TYPE type, const char *filename,
                       DFCPRJ_FILTER *pProject);



/* FUNCTION *******************************************************************/
/** Writes a filter project to a file.
 *
 *  \param filename     Filename of file (inclusive path).
 *  \param pProject     Pointer to project data.
 *
 *  \return             Zero on succes, else an error number.
 ******************************************************************************/
    int prjFileWrite (const char *filename, DFCPRJ_FILTER *pProject);


/* FUNCTION *******************************************************************/
/** Reads a filter project from a file. In case of an error it sets the pointer
 *  to the error structure, which itself can be used to display the original
 *  error message. Modification of the error structure pointer indicates an
 *  critical error. In that case it has to be free'ed with g_error_free().
 *
 *  \param filename     Filename of file (with path).
 *  \param pProject     Pointer to project buffer. This project will be
 *                      overwritten (only) on success.
 *  \param err          Pointer to an error (structure) pointer. The pointer
 *                      to the error structure must be set NULL before calling
 *                      function prjFileRead().
 *
 ******************************************************************************/
    void prjFileRead (const char *filename, DFCPRJ_FILTER *pProject, GError **err);



/* FUNCTION *******************************************************************/
/** Scans for project info (header) data in a DFCGen project file. In case
 *  of an error it sets the pointer to the error structure, which itself can be
 *  used to display the original error message. Modification of the error
 *  structure pointer indicates an critical error. In that case it has to be
 *  free'ed with g_error_free().
 *
 *  \param filename     Filename of file (with path).
 *  \param pInfo        Pointer to project info data buffer.
 *  \param err          Pointer to an error (structure) pointer. The pointer
 *                      to the error structure must be set NULL before calling
 *                      function prjFileScan().
 *
 ******************************************************************************/
    void prjFileScan (const char *filename, DFCPRJ_INFO *pInfo, GError **err);


/* FUNCTION *******************************************************************/
/** Frees malloc'ed memory space from project info (author, title, desc).
 *  This function is designed as a counterpart to function prjFileScan().
 *
 *  \param pInfo        Pointer to project info data.
 *
 ******************************************************************************/
    void prjFileFree (DFCPRJ_INFO *pInfo);


#ifdef  __cplusplus
}
#endif


#endif /* PRJFILE_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

