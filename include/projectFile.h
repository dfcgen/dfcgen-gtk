/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Project file handling.
 *
 * \note     All double values written to a \e dfcgen project file are formated
 *           in the "C" locale for LC_NUMERIC.
 *
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/projectFile.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
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


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/

#define PRJFILE_NAME_SUFFIX     ".dfc"   /**< Default project filename suffix */


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


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
/** Scans for (administrative) header data in a \e dfcgen project file. In case
 *  of an error it sets the pointer to the error structure, which itself can be
 *  used to display the original error message. Modification of the error
 *  structure pointer indicates an critical error. In that case it has to be
 *  free'ed with g_error_free().
 *
 *  \param filename     Filename of file (with path).
 *  \param pHeader      Pointer to header data buffer.
 *  \param err          Pointer to an error (structure) pointer. The pointer
 *                      to the error structure must be set NULL before calling
 *                      function prjFileScan().
 *
 ******************************************************************************/
    void prjFileScan (const char *filename, DFCPRJ_HEADER *pHeader, GError **err);


/* FUNCTION *******************************************************************/
/** Frees malloc'ed memory space from a project header (author, title, desc).
 *  This function is designed as a counterpart to function prjFileScan().
 *
 *  \param pHeader      Pointer to header data.
 *
 ******************************************************************************/
    void prjFileFree (DFCPRJ_HEADER *pHeader);


#ifdef  __cplusplus
}
#endif


#endif /* PRJFILE_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

