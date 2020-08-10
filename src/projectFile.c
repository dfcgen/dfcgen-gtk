/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file     projectFile.c
 * \brief    Project file handling.
 *
 * \note     The DFCGen filter project file functions implemented here are
 *           very closely related to the \e GLib XML support functions.
 *           Adoption to other platforms or to libxml is possible, but need
 *           some rework.
 *
 * \note     All double values written to a DFCGen project file are formated
 *           in the \e C locale for LC_CTYPE.
 *
 * \author   Copyright (C) 2006, 2011-2012 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "dfcProject.h"
#include "filterSupport.h"
#include "projectFile.h"
#include "cfgSettings.h" /* cfgGetDesktopPrefs() */

#include <stdio.h>
#include <errno.h>
#include <string.h> /* memset() */
#include <locale.h> /* setlocale() */


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/

/** Bitset (flags) for a tag.
 */
typedef enum
{
    PRJF_TAGFLAG_MISC = 1 << FLTCLASS_MISC, /**< Tag is mandatory for misc filters */
    PRJF_TAGFLAG_LINFIR = 1 << FLTCLASS_LINFIR, /**< Tag is mandatory for linear FIR filters */
    PRJF_TAGFLAG_STDIIR = 1 << FLTCLASS_STDIIR, /**< Tag is mandatory for standard IIR filters */
    PRJF_TAGFLAG_FOUND = 1 << FLTCLASS_SIZE /**< Tag found (in DFCGen project file) */
} PRJF_TAGFLAG;


/** Forward declaration of a project file tag descriptor.
 */
typedef struct _PRJF_TAG_DESC PRJF_TAG_DESC;


/** Handler of tag contents.
 *
 *  \param ctx          Pointer to parser context.
 *  \param pTag         Pointer to tag descriptor.
 *  \param prj          Pointer to filter project.
 *  \param text         Pointer to content of tag (zero-terminated).
 */
typedef int (*PRJF_TAG_CONTENT_HANDLER) (GMarkupParseContext *ctx,
                                         PRJF_TAG_DESC *pTag,
                                         DFCPRJ_FILTER *prj,
                                         char *text);


/** Handler of \e Open tags.
 *
 *  \param ctx          Pointer to parser context.
 *  \param pTag         Pointer to tag descriptor.
 *  \param prj          Pointer to filter project.
 *  \param attrn        List of attribute names.
 *  \param attrv        List of associated attribute values.
 */
typedef int (*PRJF_TAG_OPEN_HANDLER) (GMarkupParseContext *ctx,
                                      PRJF_TAG_DESC *pTag,
                                      DFCPRJ_FILTER *prj,
                                      const gchar **attrn,
                                      const gchar **attrv);


/** Project file tag identifiers.
 */
typedef enum
{
    PRJF_IDTAG_AUTHOR = 0,
    PRJF_IDTAG_TITLE = 1,
    PRJF_IDTAG_DESCRIPTION = 2,
    PRJF_IDTAG_PROJECT = 3,
    PRJF_IDTAG_FILTER = 4,
    PRJF_IDTAG_SAMPLE = 5,
    PRJF_IDTAG_NUMERATOR = 6,
    PRJF_IDTAG_DENOMINATOR = 7,
    PRJF_IDTAG_DEGREE = 8,
    PRJF_IDTAG_COEFF = 9,
    PRJF_IDTAG_DESIGN = 10,
    PRJF_IDTAG_CLASS = 11,
    PRJF_IDTAG_TYPE = 12,
    PRJF_IDTAG_ORDER = 13,
    PRJF_IDTAG_CUTOFF = 14,
    PRJF_IDTAG_CENTER = 15,
    PRJF_IDTAG_BANDWIDTH = 16,
    PRJF_IDTAG_ALGOZ = 17,
    PRJF_IDTAG_PASSBAND = 18,
    PRJF_IDTAG_STOPBAND = 19,
    PRJF_IDTAG_MODULE = 20,                /**< Elliptic filters module angle */
    PRJF_IDTAG_FTR = 21,                   /**< Frequency transformation type */
    PRJF_IDTAG_DSPWIN = 22,                              /**< Window function */

    PRJF_IDTAG_SIZE

} PRJF_IDTAG;


/** Project file XML tag descriptor.
 */
struct _PRJF_TAG_DESC
{
    PRJF_IDTAG id;                             /**< Associated tag identifier */
    int depth;                                 /**< Depth where it is allowed */
    char *name;                                          /**< Name of XML tag */

    /** Meaning of \a constraint depends on type of data/tag/attribute:
     *  - Integer: Maximum allowed value.
     *  - Double: TRUE if value is allowed to be negative, else FALSE.
     */
    int constraint;
    unsigned flags;                   /**< Bitset from values in PRJF_TAGFLAG */
    void *data;             /**< Pointer to (\e Open) tag associated variable */
    PRJF_TAG_OPEN_HANDLER openHandler;               /**< \e Open tag handler */
    PRJF_TAG_CONTENT_HANDLER contentsHandler;       /**< Tag contents handler */
};


/** Keyword identifier in an export template file.
 */
typedef enum
{
    PRJF_IDKEY_INVALID = -1,                            /**< invalid keyword */
    PRJF_IDKEY_NUM_DEGREE = 0,                         /**< numerator degree */
    PRJF_IDKEY_NUM_EXPONENT = 1,                     /**< numerator exponent */
    PRJF_IDKEY_NUM_COEFF = 2,                     /**< numerator coefficient */
    PRJF_IDKEY_DEN_DEGREE = 3,                       /**< denominator degree */
    PRJF_IDKEY_DEN_EXPONENT = 4,                   /**< denominator exponent */
    PRJF_IDKEY_DEN_COEFF = 5,                   /**< denominator coefficient */

    PRJF_IDKEY_SIZE
} PRJF_IDKEY;



/* LOCAL CONSTANT DEFINITIONS *************************************************/

#define PRJF_TEMPLATES_BASENAME "export"       /**< Basename of export files */

#define PRJF_IDTAG_INFO_END     PRJF_IDTAG_DESCRIPTION    /**< End of header */
#define PRJF_TAGFLAG_ALL        (PRJF_TAGFLAG_MISC | PRJF_TAGFLAG_STDIIR | PRJF_TAGFLAG_LINFIR)

#define PRJF_TAG_AUTHOR         "author"
#define PRJF_TAG_TITLE          "title"
#define PRJF_TAG_DESCRIPTION    "description"
#define PRJF_TAG_PROJECT        "project"
#define PRJF_TAG_FILTER         "filter"
#define PRJF_TAG_SAMPLE         "sample"
#define PRJF_TAG_NUMERATOR      "numerator"
#define PRJF_TAG_DENOMINATOR    "denominator"
#define PRJF_TAG_DEGREE         "degree"
#define PRJF_TAG_COEFF          "coefficient"
#define PRJF_TAG_DESIGN         "design"
#define PRJF_TAG_CLASS          "class"
#define PRJF_TAG_TYPE           "type"
#define PRJF_TAG_ORDER          "order"
#define PRJF_TAG_ALGOZ          "algorithm"
#define PRJF_TAG_CUTOFF         "cutoff"
#define PRJF_TAG_CENTER         "center"
#define PRJF_TAG_BANDWIDTH      "bandwidth"
#define PRJF_TAG_PASSBAND       "passband"
#define PRJF_TAG_STOPBAND       "stopband"
#define PRJF_TAG_MODULE         "module"
#define PRJF_TAG_FTR            "transform"      /**< Frequency transform tag */
#define PRJF_TAG_DSPWIN         "window"                 /**< Window function */

#define PRJF_ATTRN_LANG         "lang"   /**< Language attribute (of strings) */
#define PRJF_ATTRN_SUPERSEDED   "superseded"       /**< Design attribute name */
#define PRJF_ATTRN_GEOMETRIC    "geometric" /**< Frequency transform attribute */
#define PRJF_ATTRN_KAISER       "alpha" /**< Parameter \f$\alpha\f$ of \e Kaiser window */
#define PRJF_ATTRV_TRUE         "true"      /**< Boolean attribute value TRUE */
#define PRJF_ATTRV_FALSE        "false"    /**< Boolean attribute value FALSE */


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static PRJF_IDKEY exportLookupTemplateKeyword (gchar* keyword);
static gint exportConvertCoeff (char* buffer, gint bufsize, gdouble val);
static void writeMarkupText (FILE *f, char *lang, char *tag, char *text);
static void writeFrequTransf (FILE *f, double cutoff, FTRDESIGN *ftr);
static void writePolyCoeffs (FILE *f, MATHPOLY *poly);
static void writeMiscFltDesign (FILE *f, MISCFLT_DESIGN *pDesign);
static void writeStdIirDesign (FILE *f, STDIIR_DESIGN *pDesign);
static void writeLinFirDesign (FILE *f, LINFIR_DESIGN *pDesign);
static void readProject (const char *filename, unsigned flags, GError **err);
static int writeProject (FILE *f, DFCPRJ_FILTER *prj);
static int tagStringHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                             DFCPRJ_FILTER *prj, char *text);
static int tagIntHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                          DFCPRJ_FILTER *prj, char *text);
static int tagDoubleHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                             DFCPRJ_FILTER *prj, char *text);
static int tagDegreeHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                             DFCPRJ_FILTER *prj, char *text);
static int tagCoeffHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                            DFCPRJ_FILTER *prj, char *text);
static int tagPolyHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                           DFCPRJ_FILTER *prj, const gchar **attrn, const gchar **attrv);
static int attrStringHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                              DFCPRJ_FILTER *prj, const gchar **attrn, const gchar **attrv);
static int attrSupersedHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                                DFCPRJ_FILTER *prj, const gchar **attrn, const gchar **attrv);
static int attrGeometricHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                                 DFCPRJ_FILTER *prj, const gchar **attrn, const gchar **attrv);
static int attrKaiserHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                              DFCPRJ_FILTER *prj, const gchar **attrn, const gchar **attrv);



/* LOCAL VARIABLE DEFINITIONS *************************************************/

static GData *prjTagsList;                       /**< Hash table for all tags */
static int prjTagsDepth;                       /**< Current depth in XML file */
static PRJF_TAG_DESC *prjTagPtr;      /**< Current tag descriptor (pointer) */
static DFCPRJ_FILTER tmpPrj;                /**< The project currently parsed */


/** All XML tag descriptors (optionally) used in a project file. Most of them
 *  are constant, but for differentiating between numerator and denominator
 *  polynomials some elemenents are changed while reading the project file:
 *  - prjTagsDesc[PRJF_IDTAG_FILTER].data points to the polynomial currently
 *    processed (after detection of numerator or denominator \e Open tag).
 *  - prjTagsDesc[PRJF_IDTAG_DEGREE].data points to the element \a degree of
 *    the currently processed polynomial.
 *  - prjTagsDesc[PRJF_IDTAG_COEFF].data points to the next coefficient (same
 *    circumstances as above).
 */
static PRJF_TAG_DESC prjTagsDesc[PRJF_IDTAG_SIZE] =
{ /* id,                       depth,             name,                   constraint,
     mandatory, data,                     openHandler,       contentsHandler
  */
    {PRJF_IDTAG_AUTHOR,      1,                 PRJF_TAG_AUTHOR,      INT_MAX,
     0, &tmpPrj.info.author, attrStringHandler, tagStringHandler},
    {PRJF_IDTAG_TITLE,       1,                 PRJF_TAG_TITLE,       INT_MAX,
     0, &tmpPrj.info.title,  attrStringHandler, tagStringHandler},
    {PRJF_IDTAG_DESCRIPTION, 1,                 PRJF_TAG_DESCRIPTION, INT_MAX,
     0, &tmpPrj.info.desc,   attrStringHandler, tagStringHandler},
    {PRJF_IDTAG_PROJECT,     0,                 PRJF_TAG_PROJECT,     0,
     PRJF_TAGFLAG_ALL, NULL,                     NULL,              NULL},
    {PRJF_IDTAG_FILTER,      1,                 PRJF_TAG_FILTER,      0,
     PRJF_TAGFLAG_ALL, NULL,                     NULL,              NULL},
    {PRJF_IDTAG_SAMPLE,      2,                 PRJF_TAG_SAMPLE,      FALSE,
     PRJF_TAGFLAG_ALL, &tmpPrj.filter.f0,        NULL,              tagDoubleHandler},
    {PRJF_IDTAG_NUMERATOR,   2,                 PRJF_TAG_NUMERATOR,   0,
     PRJF_TAGFLAG_ALL, &tmpPrj.filter.num,       tagPolyHandler,    NULL},
    {PRJF_IDTAG_DENOMINATOR, 2,                 PRJF_TAG_DENOMINATOR, 0,
     PRJF_TAGFLAG_ALL, &tmpPrj.filter.den,       tagPolyHandler,    NULL},
    {PRJF_IDTAG_DEGREE,      3,                 PRJF_TAG_DEGREE,      FLT_DEGREE_MAX,
     PRJF_TAGFLAG_ALL,       NULL,              NULL,              tagDegreeHandler},
    {PRJF_IDTAG_COEFF,       3,                 PRJF_TAG_COEFF,       TRUE,
     PRJF_TAGFLAG_ALL, NULL,                     NULL,              tagCoeffHandler},
    {PRJF_IDTAG_DESIGN,      1,                 PRJF_TAG_DESIGN,      0,
     PRJF_TAGFLAG_STDIIR | PRJF_TAGFLAG_LINFIR, NULL,                     attrSupersedHandler, NULL},
    {PRJF_IDTAG_CLASS,       2,                 PRJF_TAG_CLASS,       FLTCLASS_SIZE - 1,
     PRJF_TAGFLAG_STDIIR | PRJF_TAGFLAG_LINFIR, &tmpPrj.fltcls,           NULL,              tagIntHandler},
    {PRJF_IDTAG_TYPE,        2,                 PRJF_TAG_TYPE,        INT_MAX,
     PRJF_TAGFLAG_STDIIR | PRJF_TAGFLAG_LINFIR, &tmpPrj.design.all.type,  NULL,              tagIntHandler},
    {PRJF_IDTAG_ORDER,       2,                 PRJF_TAG_ORDER,       FLT_DEGREE_MAX,
     PRJF_TAGFLAG_STDIIR | PRJF_TAGFLAG_LINFIR, &tmpPrj.design.all.order, NULL,              tagIntHandler},
    {PRJF_IDTAG_CUTOFF,      2,                 PRJF_TAG_CUTOFF,      FALSE,
     0, &tmpPrj.design.all.cutoff, NULL,              tagDoubleHandler},
    {PRJF_IDTAG_CENTER,      2,                 PRJF_TAG_CENTER,      FALSE,
     0, &tmpPrj.design.all.ftr.fc, attrGeometricHandler, tagDoubleHandler},
    {PRJF_IDTAG_BANDWIDTH,   2,                 PRJF_TAG_BANDWIDTH,   FALSE,
     0, &tmpPrj.design.all.ftr.bw, NULL, tagDoubleHandler},
    {PRJF_IDTAG_ALGOZ,       2,                 PRJF_TAG_ALGOZ,       ZTR_SIZE - 1,
     PRJF_TAGFLAG_STDIIR, &tmpPrj.design.stdIir.zAlgo, NULL,           tagIntHandler},
    {PRJF_IDTAG_PASSBAND,    2,                 PRJF_TAG_PASSBAND,    FALSE,
     0, &tmpPrj.design.stdIir.ripple, NULL,     tagDoubleHandler},
    {PRJF_IDTAG_STOPBAND,    2,                 PRJF_TAG_STOPBAND,    FALSE,
     0, &tmpPrj.design.stdIir.minatt, NULL,     tagDoubleHandler},
    {PRJF_IDTAG_MODULE,      2,                 PRJF_TAG_MODULE,      FALSE,
     0, &tmpPrj.design.stdIir.angle, NULL,      tagDoubleHandler},
    {PRJF_IDTAG_FTR,        2,                  PRJF_TAG_FTR,         FTR_SIZE,
     PRJF_TAGFLAG_STDIIR | PRJF_TAGFLAG_LINFIR, &tmpPrj.design.all.ftr.type,  NULL, tagIntHandler},
    {PRJF_IDTAG_DSPWIN,     2,                  PRJF_TAG_DSPWIN,      LINFIR_DSPWIN_SIZE,
     PRJF_TAGFLAG_LINFIR,  &tmpPrj.design.linFir.dspwin, attrKaiserHandler, tagIntHandler}
};



/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Handler for tag contents which is stored as a string.
 *
 *  \param ctx          Pointer to parser context.
 *  \param pTag         Pointer to tag descriptor.
 *  \param prj          Pointer to DFCGen project.
 *  \param text         Pointer to content of tag (zero-terminated).
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
static int tagStringHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                             DFCPRJ_FILTER *prj, char *text)
{
    char **str;                                  /* pointer to string pointer */

    ASSERT (pTag != NULL);

    str = (char **)(pTag->data);

    if (*str == NULL)                            /* take the string only once */
    {
        *str = g_strdup (text);
    } /* if */

    return 0;
} /* tagStringHandler() */


/* FUNCTION *******************************************************************/
/** Handler for tag contents which is an integer number.
 *
 *  \param ctx          Pointer to parser context.
 *  \param p            Pointer to tag descriptor.
 *  \param prj          Pointer to DFCGen project.
 *  \param text         Pointer to content of tag (zero-terminated).
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
static int tagIntHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *p,
                          DFCPRJ_FILTER *prj, char *text)
{
    long int result;

    errno = 0;
    result = strtol (text, NULL, 10);

    if ((result <= p->constraint) && (errno == 0))
    {
        *(int *)(p->data) = result;
        return 0;
    } /* if */

    return G_MARKUP_ERROR_INVALID_CONTENT;
} /* tagIntHandler() */



/* FUNCTION *******************************************************************/
/** Handler for tag contents which is an double number.
 *
 *  \param ctx          Pointer to parser context.
 *  \param p            Pointer to tag descriptor.
 *  \param prj          Pointer to DFCGen project.
 *  \param text         Pointer to content of tag (zero-terminated).
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
static int tagDoubleHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *p,
                             DFCPRJ_FILTER *prj, char *text)
{
    double result;

    ASSERT (p != NULL);
    errno = 0;
    result = g_ascii_strtod (text, NULL);

    if ((errno == 0) && (p->constraint || (result > 0)))
    {
        *(double *)(p->data) = result;

        return 0;
    } /* if */

    return G_MARKUP_ERROR_INVALID_CONTENT;
} /* tagDoubleHandler() */


/* FUNCTION *******************************************************************/
/** Handler for tag contents which is a polynomial coefficient.
 *
 *  \note A precondition for success of this function is a preceding call of
 *        function tagDegreeHandler().
 *
 *  \param ctx          Pointer to parser context.
 *  \param pTag         Pointer to tag descriptor.
 *  \param prj          Pointer to DFCGen project.
 *  \param text         Pointer to content of tag (zero-terminated).
 *
 *  \return             Zero on success, else an error number in domain G_MARKUP_ERROR.
 ******************************************************************************/
static int tagCoeffHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                            DFCPRJ_FILTER *prj, char *text)
{
    int ret;
    double *pCoeff;

    ASSERT (pTag != NULL);
    pCoeff = pTag->data;

    if (pCoeff == NULL)
    {
        return G_MARKUP_ERROR_INVALID_CONTENT;
    } /* if */

    ret = tagDoubleHandler (ctx, pTag, prj, text);
    pTag->data = ++pCoeff;                      /* switch to next coefficient */

    return ret;
} /* tagCoeffHandler() */

    


/* FUNCTION *******************************************************************/
/** Handler for tag contents which is an polynomial degree number.
 *
 *  \note A precondition for success of this function is a preceding call of
 *        function tagPolyHandler().
 *
 *  \param ctx          Pointer to parser context.
 *  \param pTag         Pointer to tag descriptor.
 *  \param prj          Pointer to DFCGen project.
 *  \param text         Pointer to content of tag (zero-terminated).
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
static int tagDegreeHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                             DFCPRJ_FILTER *prj, char *text)
{
    int ret;

    MATHPOLY *poly = prjTagsDesc[PRJF_IDTAG_FILTER].data;

    ASSERT (pTag != NULL);

    if (poly != NULL)                                 /* polynomial selected? */
    {
        pTag->data = &poly->degree; /* set pointer to degree (for conversion) */
        ret = tagIntHandler (ctx, pTag, prj, text);     /* *p->data := degree */

        if (ret == 0)
        {
            poly->coeff = g_malloc (((1 + poly->degree) * sizeof(*poly->coeff)));
            prjTagsDesc[PRJF_IDTAG_COEFF].data = poly->coeff; /* first coefficient */

            if (poly->coeff != NULL)
            {
                return 0;
            } /* if */
        } /* if */
    } /* if */

    return G_MARKUP_ERROR_INVALID_CONTENT;
} /* tagDegreeHandler() */



/* FUNCTION *******************************************************************/
/** Handler for \e Open tag of numerator or denominator polynomial. Stores a
 *  pointer to the current polynomial in prjTagsDesc[PRJF_IDTAG_FILTER].data.
 *
 *  \param ctx          Pointer to parser context.
 *  \param p            Pointer to tag descriptor.
 *  \param prj          Pointer to DFCGen project.
 *  \param attrn        List of attribute names.
 *  \param attrv        List of associated attribute values.
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
static int tagPolyHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *p,
                           DFCPRJ_FILTER *prj, const gchar **attrn, const gchar **attrv)
{
    ASSERT (p != NULL);
    prjTagsDesc[PRJF_IDTAG_FILTER].data = p->data;   /* pointer to polynomial */

    return 0;
} /* tagPolyHandler() */


/* FUNCTION *******************************************************************/
/** Handler for \e Open tag of design (PRJF_IDTAG_DESIGN).
 *
 *  \param ctx          Pointer to parser context.
 *  \param pTag         Pointer to tag descriptor.
 *  \param prj          Pointer to DFCGen project.
 *  \param attrn        List of attribute names.
 *  \param attrv        List of associated attribute values.
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
static int attrSupersedHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                                DFCPRJ_FILTER *prj, const gchar *attrn[],
                                const gchar *attrv[])
{
    ASSERT (pTag != NULL);

    while (*attrn != NULL)           /* go through the array of char pointers */
    {
        if (!strncmp (*attrn, PRJF_ATTRN_SUPERSEDED,              /* found? */
                      sizeof (PRJF_ATTRN_SUPERSEDED)))
        {
            if (!strncmp (*attrv, PRJF_ATTRV_TRUE, sizeof (PRJF_ATTRV_TRUE)))
            {
                prj->flags |= DFCPRJ_FLAG_SUPERSEDED;
            } /* if */
        } /* if */

        ++attrn;                                        /* try next attribute */
        ++attrv;
    } /* while */

    return 0;
} /* attrSupersedHandler() */



/* FUNCTION *******************************************************************/
/** Handler for \e Open tag of strings (author, title, description).
 *
 *  \param ctx          Pointer to parser context.
 *  \param pTag         Pointer to tag descriptor.
 *  \param prj          Pointer to DFCGen project.
 *  \param attrn        List of attribute names.
 *  \param attrv        List of associated attribute values.
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
static int attrStringHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                              DFCPRJ_FILTER *prj, const gchar **attrn, const gchar **attrv)
{
    char **str;                                /* Pointer to a string pointer */

    /* Split structure of locale into pieces, regarding the scheme:
     * language[_territory[.codeset]][@modifier]
     */
    char **locvec = g_strsplit_set (setlocale (LC_CTYPE, NULL), "_.@", -1);

    ASSERT (pTag != NULL);

    while (*attrn != NULL)           /* go through the array of char pointers */
    {
        if (!strncmp (*attrn, PRJF_ATTRN_LANG,                    /* found? */
                      sizeof (PRJF_ATTRN_LANG)))
        {
            if ((locvec[0] != NULL) &&                     /* language match? */
                (!g_strcmp0 (locvec[0], *attrv)))
            {
                str = (char **)(pTag->data);

                if (*str != NULL)                /* (string) tag found again? */
                {
                    g_free (*str);
                    *str = NULL;              /* allow strdup on tag contents */
                } /* if */
            } /* if */
        } /* if */

        ++attrn;                                        /* try next attribute */
        ++attrv;
    } /* while */

    g_strfreev(locvec);

    return 0;
} /* attrStringHandler() */


/* FUNCTION *******************************************************************/
/** Handler for \e Open tag of center frequency (PRJF_IDTAG_CENTER).
 *
 *  \note At the moment we use only \e two attributes: \e superseded and
 *        \e geometric. But if the number of attributes increases then we
 *        should think about a redesign of the PRJF_TAG_DESC structure.
 *
 *  \param ctx          Pointer to parser context.
 *  \param pTag         Pointer to tag descriptor.
 *  \param prj          Pointer to DFCGen project.
 *  \param attrn        List of attribute names.
 *  \param attrv        List of associated attribute values.
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
static int attrGeometricHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                                 DFCPRJ_FILTER *prj, const gchar **attrn, const gchar **attrv)
{
    ASSERT (pTag != NULL);

    while (*attrn != NULL)           /* go through the array of char pointers */
    {
        if (!strncmp (*attrn, PRJF_ATTRN_GEOMETRIC,               /* found? */
                      sizeof (PRJF_ATTRN_GEOMETRIC)))
        {
            if (!strncmp (*attrv, PRJF_ATTRV_TRUE, sizeof (PRJF_ATTRV_TRUE)))
            {
                prj->design.all.ftr.flags |= FTRDESIGN_FLAG_CENTER_GEOMETRIC;
            } /* if */
        } /* if */

        ++attrn;                                    /* try next attribute */
        ++attrv;
    } /* while */

    return 0;
} /* attrGeometricHandler() */



/* FUNCTION *******************************************************************/
/** Handler for \e Open tag of window function (PRJF_IDTAG_DSPWIN).
 *
 *  \param ctx          Pointer to parser context.
 *  \param pTag         Pointer to tag descriptor.
 *  \param prj          Pointer to DFCGen project.
 *  \param attrn        List of attribute names.
 *  \param attrv        List of associated attribute values.
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
static int attrKaiserHandler (GMarkupParseContext *ctx, PRJF_TAG_DESC *pTag,
                              DFCPRJ_FILTER *prj, const gchar **attrn, const gchar **attrv)
{
    ASSERT (pTag != NULL);

    while (*attrn != NULL)           /* go through the array of char pointers */
    {
        if (!strncmp (*attrn, PRJF_ATTRN_KAISER,                    /* found? */
                      sizeof (PRJF_ATTRN_KAISER)))
        {
            prj->design.linFir.winparm = g_ascii_strtod (*attrv, NULL);
        } /* if */

        ++attrn;                                        /* try next attribute */
        ++attrv;
    } /* while */

    return 0;
} /* attrKaiserHandler() */


/* FUNCTION *******************************************************************/
/** Handler which is called for every \e Open tag found in the XML file.
 *
 *  \param ctx          Pointer to parser context.
 *  \param element      Pointer to name of element.
 *  \param attrn        List of attribute names.
 *  \param attrv        List of associated attribute values.
 *  \param data         User data as passed to g_markup_parse_context_new().
 *                      Here it is a pointer to the project (DFCPRJ_FILTER).
 *  \param err          Pointer to error (structure).
 *
 ******************************************************************************/
static void prjFileOpenTagHandler (GMarkupParseContext *ctx,
                                   const gchar *element,
                                   const gchar **attrn,
                                   const gchar **attrv,
                                   gpointer data,
                                   GError **err)
{
    prjTagPtr = g_datalist_get_data (&prjTagsList, element);

    if (prjTagPtr == NULL)                                    /* unknown tag? */
    {
        return;                   /* skip unknow tags (forward compatibility) */
    } /* if */


    if (prjTagPtr->depth == prjTagsDepth)
    {
        ++prjTagsDepth;
        prjTagPtr->flags |= PRJF_TAGFLAG_FOUND;            /* mark as 'found' */

        if ((prjTagPtr->openHandler == NULL) ||                /* no handler? */
            ((((DFCPRJ_FILTER*)data)->flags & DFCPRJ_FLAG_INTERNAL) &&
             (prjTagPtr->id > PRJF_IDTAG_INFO_END)))            /* only scan? */
        {
            return;                                              /* that's it */
        } /* if */

        if ((prjTagPtr->openHandler)(ctx, prjTagPtr, data, attrn, attrv) == 0)
        {
            return;                                           /* okay handled */
        } /* if */
    } /* if */


    g_set_error (err, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                 _("Unexpected tag '%s'"), element);

} /* prjFileOpenTagHandler() */



/* FUNCTION *******************************************************************/
/** Handler which is called for an \e Close tag found in the XML file.
 *
 *  \param ctx          Pointer to parser context.
 *  \param element      Pointer to name of element.
 *  \param data         User data as passed to g_markup_parse_context_new().
 *                      Here it is a pointer to the project (DFCPRJ_FILTER).
 *  \param err          Pointer to error (structure).
 *
 ******************************************************************************/
static void prjFileCloseTagHandler (GMarkupParseContext *ctx, const gchar *element,
                                    gpointer data, GError **err)
{
    prjTagPtr = NULL;
    --prjTagsDepth;
} /* prjFileCloseTagHandler() */



/* FUNCTION *******************************************************************/
/** Handler which is called for the text embedded between an \e Open and a
 *  \e Close tag (in XML project file).
 *
 *  \param ctx          Pointer to parser context.
 *  \param text         Pointer to the text between \e Open and \e Close tag.
 *                      Notice that the text is not zero-terminated.
 *  \param len          Length of \p text.
 *  \param data         User data as passed to g_markup_parse_context_new().
 *                      Here it is a pointer to the project (DFCPRJ_FILTER).
 *  \param err          Pointer to error (structure).
 *
 ******************************************************************************/
static void prjFileTextHandler (GMarkupParseContext *ctx, const gchar *text,
                                gsize len, gpointer data, GError **err)
{
    int lineno, ret;
    char *str;

    if (prjTagPtr == NULL)            /* ignore tag contents if not on a leaf */
    {
        return;
    } /* if */

    if ((prjTagPtr->contentsHandler == NULL) || /* no handler (unimportant tag)? */
        ((((DFCPRJ_FILTER*)data)->flags & DFCPRJ_FLAG_INTERNAL) &&
         (prjTagPtr->id > PRJF_IDTAG_INFO_END)))                /* only scan? */
    {
        return;
    } /* if */

    str = g_strndup (text, len);         /* convert to zero-terminated string */
    ret = prjTagPtr->contentsHandler (ctx, prjTagPtr, data, str);
    g_free (str);

    if (ret == 0)
    {
        return;
    } /* if */

    g_markup_parse_context_get_position (ctx, &lineno, &ret);
    g_set_error (err, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                 _("Syntax error at line %d"), lineno);

} /* prjFileTextHandler() */



/* FUNCTION *******************************************************************/
/** Writes coefficients of a polynomial to the (open) project file.
 *
 *  \param f            File.
 *  \param poly         Pointer to polynomial.
 *
 ******************************************************************************/
static void writePolyCoeffs(FILE *f, MATHPOLY *poly)
{
    int i;
    char buf[G_ASCII_DTOSTR_BUF_SIZE];        /* buffer for double conversion */

    fprintf (f, "\t\t\t<" PRJF_TAG_DEGREE ">%d</" PRJF_TAG_DEGREE ">\n",
             poly->degree);

    for (i = 0; i <= poly->degree; i++)
    {
        fprintf (f, "\t\t\t<" PRJF_TAG_COEFF " tap=\"%d\">%s</" PRJF_TAG_COEFF ">\n",
                 i, g_ascii_dtostr (buf, sizeof(buf), poly->coeff[i]));
    } /* for */
} /* writePolyCoeffs() */



/* FUNCTION *******************************************************************/
/** Writes frequency transformation design data to a project file.
 *
 *  \param f            File.
 *  \param cutoff       Lowpass cutoff frequency to be written if FTR_NON is
 *                      indicated.
 *  \param ftr          Pointer to frequency transformation data.
 *
 ******************************************************************************/
static void writeFrequTransf(FILE *f, double cutoff, FTRDESIGN *ftr)
{
    char buf[G_ASCII_DTOSTR_BUF_SIZE];        /* buffer for double conversion */

    fprintf (f, "\t\t<" PRJF_TAG_FTR ">%d</" PRJF_TAG_FTR ">\n", ftr->type);

    switch (ftr->type)
    {
        case FTR_NON:
            fprintf (f, "\t\t<" PRJF_TAG_CUTOFF ">%s</" PRJF_TAG_CUTOFF ">\n",
                     g_ascii_dtostr (buf, sizeof(buf), cutoff));
            break;


        case FTR_HIGHPASS:
            fprintf (f, "\t\t<" PRJF_TAG_CUTOFF ">%s</" PRJF_TAG_CUTOFF ">\n",
                     g_ascii_dtostr (buf, sizeof(buf), ftr->fc));
            break;


        case FTR_BANDPASS:
        case FTR_BANDSTOP:
            fprintf (f, "\t\t<" PRJF_TAG_CENTER " " PRJF_ATTRN_GEOMETRIC
                     "=\"%s\">%s</" PRJF_TAG_CENTER ">\n",
                     (ftr->flags & FTRDESIGN_FLAG_CENTER_GEOMETRIC) ?
                     PRJF_ATTRV_TRUE : PRJF_ATTRV_FALSE,
                     g_ascii_dtostr (buf, sizeof(buf), ftr->fc));

            fprintf (f, "\t\t<" PRJF_TAG_BANDWIDTH ">%s</" PRJF_TAG_BANDWIDTH ">\n",
                     g_ascii_dtostr (buf, sizeof(buf), ftr->bw));
            break;


        default:
            ASSERT (0);
    } /* switch */
} /* writeFrequTransf() */



/* FUNCTION *******************************************************************/
/** Writes design data of a Misc filter (FLTCLASS_MISC) to project file.
 *
 *  \param f            File.
 *  \param pDesign      Pointer to Misc filter design data.
 *
 ******************************************************************************/
static void writeMiscFltDesign (FILE *f, MISCFLT_DESIGN *pDesign)
{
    fprintf (f, "\t\t<" PRJF_TAG_TYPE ">%d</" PRJF_TAG_TYPE ">\n",
             pDesign->type);

    if (pDesign->type != MISCFLT_UNKNOWN)                /* not a raw filter? */
    {
        fprintf (f, "\t\t<" PRJF_TAG_ORDER ">%d</" PRJF_TAG_ORDER ">\n",
                 pDesign->order);
    } /* if */
} /* writeMiscFltDesign() */



/* FUNCTION *******************************************************************/
/** Writes design data of standard IIR filter (FLTCLASS_STDIIR) to project file.
 *
 *  \param f            File.
 *  \param pDesign      Pointer to standard IIR filter design data.
 *
 ******************************************************************************/
static void writeStdIirDesign(FILE *f, STDIIR_DESIGN *pDesign)
{
    char buf[G_ASCII_DTOSTR_BUF_SIZE];        /* buffer for double conversion */

    fprintf (f, "\t\t<" PRJF_TAG_TYPE ">%d</" PRJF_TAG_TYPE ">\n"
                "\t\t<" PRJF_TAG_ORDER ">%d</" PRJF_TAG_ORDER ">\n"
                "\t\t<" PRJF_TAG_ALGOZ ">%d</" PRJF_TAG_ALGOZ ">\n",
                pDesign->type, pDesign->order, pDesign->zAlgo);

    switch (pDesign->type)
    {
        case STDIIR_TYPE_CHEBY:
            fprintf (f, "\t\t<" PRJF_TAG_PASSBAND ">%s</" PRJF_TAG_PASSBAND ">\n",
                     g_ascii_dtostr (buf, sizeof(buf), pDesign->ripple));
            break;


        case STDIIR_TYPE_CHEBYINV:
            fprintf (f, "\t\t<" PRJF_TAG_STOPBAND ">%s</" PRJF_TAG_STOPBAND ">\n",
                     g_ascii_dtostr (buf, sizeof(buf), pDesign->minatt));
            break;


        case STDIIR_TYPE_CAUER1:            /* design by max. passband ripple */
            fprintf (f, "\t\t<" PRJF_TAG_PASSBAND ">%s</" PRJF_TAG_PASSBAND ">\n",
                     g_ascii_dtostr (buf, sizeof(buf), pDesign->ripple));
            fprintf (f, "\t\t<" PRJF_TAG_MODULE ">%s</" PRJF_TAG_MODULE ">\n",
                     g_ascii_dtostr (buf, sizeof(buf), pDesign->angle));
            break;


        case STDIIR_TYPE_CAUER2:            /* design by min. stopband atten. */
            fprintf (f, "\t\t<" PRJF_TAG_STOPBAND ">%s</" PRJF_TAG_STOPBAND ">\n",
                     g_ascii_dtostr (buf, sizeof(buf), pDesign->minatt));
            fprintf (f, "\t\t<" PRJF_TAG_MODULE ">%s</" PRJF_TAG_MODULE ">\n",
                     g_ascii_dtostr (buf, sizeof(buf), pDesign->angle));
            break;


        case STDIIR_TYPE_BUTTERWORTH:
        case STDIIR_TYPE_BESSEL:
            break;

        default:
            ASSERT (0);
    } /* switch */
        

    writeFrequTransf (f, pDesign->cutoff, &pDesign->ftr);
} /* writeStdIirDesign() */



/* FUNCTION *******************************************************************/
/** Writes design data of linear FIR filter (FLTCLASS_LINFIR) to project file.
 *
 *  \param f            File.
 *  \param pDesign      Pointer to linear FIR filter design data.
 *
 ******************************************************************************/
static void writeLinFirDesign (FILE *f, LINFIR_DESIGN *pDesign)
{
    char buf[G_ASCII_DTOSTR_BUF_SIZE];        /* buffer for double conversion */

    fprintf (f, "\t\t<" PRJF_TAG_TYPE ">%d</" PRJF_TAG_TYPE ">\n"
                "\t\t<" PRJF_TAG_ORDER ">%d</" PRJF_TAG_ORDER ">\n",
                pDesign->type, pDesign->order);

    if (pDesign->dspwin == LINFIR_DSPWIN_KAISER)
    {
        fprintf (f, "\t\t<" PRJF_TAG_DSPWIN " " PRJF_ATTRN_KAISER "=\"%s\">%d</" PRJF_TAG_DSPWIN ">\n",
                 g_ascii_dtostr (buf, sizeof(buf), pDesign->winparm), LINFIR_DSPWIN_KAISER);
    } /* if */
    else
    {
        fprintf (f, "\t\t<" PRJF_TAG_DSPWIN ">%d</" PRJF_TAG_DSPWIN ">\n",
                 pDesign->dspwin);
    } /* else */

    writeFrequTransf (f, pDesign->cutoff, &pDesign->ftr);

} /* writeLinFirDesign() */



/* FUNCTION *******************************************************************/
/** Writes a filter project to a project file.
 *
 *  \param f            File.
 *  \param lang         Language from locale LC_CTYPE (may be NULL).
 *  \param tag          Tag.
 *  \param text         Markup text to write (may be NULL, then nothing is written).
 *
 ******************************************************************************/
static void writeMarkupText (FILE *f, char *lang, char *tag, char *text)
{
    char *xmltext;

    if (text != NULL)
    {
        xmltext = g_markup_escape_text (text, -1);

        if (lang == NULL)
        {
            fprintf (f, "\t<%s lang=\"%s\">%s</%s>\n", tag, lang, xmltext, tag);
        } /* if */
        else
        {
            fprintf (f, "\t<%s>%s</%s>\n", tag, xmltext, tag);
        } /* else */

        g_free (xmltext);                               /* free escaped strings */
    } /* if */
} /* writeMarkupText() */


/* FUNCTION *******************************************************************/
/** Writes a filter project to a project file.
 *
 *  \param f            File.
 *  \param prj          Pointer to project.
 *
 *  \return             Zero on succes, else an error number.
 ******************************************************************************/
static int writeProject (FILE *f, DFCPRJ_FILTER *prj)
{
    char buf[G_ASCII_DTOSTR_BUF_SIZE] = {'\0'}; /* buffer for double conversion */

    /* Split structure of locale into pieces, regarding the scheme:
     * language[_territory[.codeset]][@modifier]
     */
    char **locvec = g_strsplit_set (setlocale (LC_CTYPE, NULL), "_.@", -1);

    fprintf (f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<" PRJF_TAG_PROJECT " generator=\"%s\" version=\"%s\">\n",
                PACKAGE, VERSION);

    writeMarkupText (f, locvec[0], PRJF_TAG_AUTHOR, prj->info.author);
    writeMarkupText (f, locvec[0], PRJF_TAG_TITLE, prj->info.title);
    writeMarkupText (f, locvec[0], PRJF_TAG_DESCRIPTION, prj->info.desc);

    g_strfreev(locvec);

    fprintf (f, "\t<" PRJF_TAG_FILTER ">\n"
                "\t\t<" PRJF_TAG_SAMPLE ">%s</" PRJF_TAG_SAMPLE ">\n"
                "\t\t<" PRJF_TAG_NUMERATOR ">\n",
                g_ascii_dtostr (buf, sizeof(buf), prj->filter.f0));

    writePolyCoeffs (f, &prj->filter.num);
    fprintf (f, "\t\t</" PRJF_TAG_NUMERATOR ">\n"
                "\t\t<" PRJF_TAG_DENOMINATOR ">\n");
    writePolyCoeffs (f, &prj->filter.den);
    fprintf (f, "\t\t</" PRJF_TAG_DENOMINATOR ">\n"
                "\t</" PRJF_TAG_FILTER ">\n"
                "\t<" PRJF_TAG_DESIGN " " PRJF_ATTRN_SUPERSEDED "=\"%s\">\n"
                "\t\t<" PRJF_TAG_CLASS ">%d</" PRJF_TAG_CLASS ">\n",
                (prj->flags & DFCPRJ_FLAG_SUPERSEDED) ?
                PRJF_ATTRV_TRUE : PRJF_ATTRV_FALSE, prj->fltcls);

    switch (prj->fltcls)
    {
        case FLTCLASS_STDIIR:
            writeStdIirDesign (f, &prj->design.stdIir);
            break;

        case FLTCLASS_MISC:
            writeMiscFltDesign (f, &prj->design.miscFlt);
            break;

        case FLTCLASS_LINFIR:
            writeLinFirDesign (f, &prj->design.linFir);
            break;

        default:
            ASSERT (0);
    } /* switch */

    fprintf (f, "\t</" PRJF_TAG_DESIGN ">\n"
                "</" PRJF_TAG_PROJECT ">\n");

    return 0;
} /* writeProject() */



/* FUNCTION *******************************************************************/
/**
 *  \brief              Reads a filter project from a file into variable tmpPrj.
 *
 *                      In case of an error it sets the pointer to the error
 *                      structure, which itself can be used to display the
 *                      original error message. Modification of the error
 *                      structure pointer indicates an critical error. In that
 *                      case it has to be free'ed with g_error_free().
 *
 *  \param filename     Filename of file (with path).
 *  \param flags        Project flags to be preset. Set this to
 *                      DFCPRJ_FLAG_INTERNAL if you want to scan only for
 *                      project header information.
 *  \param err          Pointer to an error (structure) pointer. The pointer
 *                      to the error structure must be set NULL before calling
 *                      function readProject().
 *
 ******************************************************************************/
static void readProject (const char *filename, unsigned flags, GError **err)
{
    static GMarkupParser parser =                   /* XML parser (functions) */
    {
        prjFileOpenTagHandler,                            /* open tag handler */
        prjFileCloseTagHandler,                          /* close tag handler */
        prjFileTextHandler,                               /* contents handler */
        NULL,                                        /* verbatim text handler */
        NULL                                                 /* error handler */
    };


    int i;
    gsize len;
    gchar *content;
    GMarkupParseContext *ctx;

    if (prjTagsList == NULL)             /* on first call build the hash list */
    {
        g_datalist_init (&prjTagsList);

        for (len = 0; len < N_ELEMENTS (prjTagsDesc); len++)
        {
            g_datalist_set_data (&prjTagsList, prjTagsDesc[len].name, &prjTagsDesc[len]);
        } /* for */
    } /* if */


    ctx = g_markup_parse_context_new (&parser, 0, &tmpPrj, NULL);
    ASSERT (ctx != NULL);

    if (g_file_get_contents (filename, &content, &len, err))     /* read file */
    {
        prjTagsDepth = 0;                  /* reset current depth in XML file */
        prjTagPtr = NULL;             /* reset current tag descriptor pointer */

        prjTagsDesc[PRJF_IDTAG_DEGREE].data =       /* clear dynamic tag data */
            prjTagsDesc[PRJF_IDTAG_COEFF].data =
            prjTagsDesc[PRJF_IDTAG_FILTER].data = NULL;

        for (i = 0; i < N_ELEMENTS (prjTagsDesc); i++)
        {                                      /* clear all 'tag found' flags */
            prjTagsDesc[i].flags &= ~PRJF_TAGFLAG_FOUND;
        } /* for */

        memset (&tmpPrj, 0, sizeof (tmpPrj));           /* reset temp. filter */
        tmpPrj.fltcls = FLTCLASS_DEFAULT;     /* all design data are optional */
        tmpPrj.design.all.type = MISCFLT_UNKNOWN;    /* default -> raw filter */
        tmpPrj.flags = flags;

        if (g_markup_parse_context_parse (ctx, content, len, err))   /* parse */
        {
            ASSERT (*err == NULL);

            if (g_markup_parse_context_end_parse (ctx, err))
            {                                     /* check for mandatory tags */
                unsigned mask = 1 << tmpPrj.fltcls;
                ASSERT (*err == NULL);

                for (i = 0; i < N_ELEMENTS (prjTagsDesc); i++)
                {
                    if (prjTagsDesc[i].flags & mask)        /* mandatory tag? */
                    {
                        if (!(prjTagsDesc[i].flags & PRJF_TAGFLAG_FOUND))
                        {
                            g_set_error (err, G_MARKUP_ERROR,
                                         G_MARKUP_ERROR_INVALID_CONTENT,
                                         _("Mandatory tag '%s' missing"),
                                         prjTagsDesc[i].name);
                            i = INT_MAX;                    /* break for loop */
                        } /* if */
                    } /* if */
                } /* for */

                /*
                 * The next step is a counterpart to writeFrequTransf().  It
                 * restores the cutoff/center frequency the right way, when
                 * frequency transformations are active.
                 */
                switch (tmpPrj.design.all.ftr.type)
                {
                    case FTR_NON:                  /* ftr.fc has no meaning */
                    case FTR_BANDPASS: /* wrote ftr.fc with PRJF_TAG_CENTER */
                    case FTR_BANDSTOP:
                        break;

                    case FTR_HIGHPASS: /* wrote ftr.fc with PRJF_TAG_CUTOFF */
                        tmpPrj.design.all.ftr.fc = tmpPrj.design.all.cutoff;
                        break;

                    default:
                        g_set_error (err, G_MARKUP_ERROR,
                                     G_MARKUP_ERROR_INVALID_CONTENT,
                                     _("Unknown frequency transform type %d"),
                                     tmpPrj.design.all.ftr.type);
                        break;
                } /* switch */
            } /* if */
        } /* if */


        if (*err != NULL)                                     /* parse error? */
        {
            dfcPrjFree (&tmpPrj);
        } /* if */

        g_free (content);
    } /* if */

    g_markup_parse_context_free (ctx);

} /* readProject() */



/* FUNCTION *******************************************************************/
/** \brief  Performs a template file keyword lookup and returns the associated
 *          identifier.
 *
 *  \param[in] keyword  Template file keyword to search for.
 *
 *  \return    Keyword identifier from ::PRJF_IDKEY (if unkown keyword the
 *             value PRJF_IDKEY::PRJF_IDKEY_INVALID.
 *
 ******************************************************************************/
static PRJF_IDKEY exportLookupTemplateKeyword (gchar* keyword)
{
    static gchar* s_templKeywordList[] =
    {
        "PRJ:FILTER:NUM:DEGREE", /* PRJF_IDKEY_NUM_DEGREE: numerator degree */
        "PRJ:FILTER:NUM:EXPONENT", /* PRJF_IDKEY_NUM_EXPONENT: numerator exponent */
        "PRJ:FILTER:NUM:COEFF", /* PRJF_IDKEY_NUM_COEFF: numerator coefficient */
        "PRJ:FILTER:DEN:DEGREE", /* PRJF_IDKEY_DEN_DEGREE: denominator degree */
        "PRJ:FILTER:DEN:EXPONENT", /* PRJF_IDKEY_DEN_EXPONENT: denominator exponent */
        "PRJ:FILTER:DEN:COEFF" /* PRJF_IDKEY_DEN_COEFF: denominator coefficient */
    };

    PRJF_IDKEY i = 0;

    while (i < N_ELEMENTS (s_templKeywordList))
    {
        if (g_strcmp0 (keyword, s_templKeywordList[i]) == 0)
        {
            return i;
        } /* if */

        ++i;
    } /* while */

    return PRJF_IDKEY_INVALID;
} /* exportLookupTemplateKeyword() */



/* FUNCTION *******************************************************************/
/** \brief  Writes a \c double value as string to a buffer, always using '.' as
 *          decimal point (does not regard the current locale).
 *
 *  \param[out] buffer  Pointer to buffer to use (should be at least of size \c G_ASCII_DTOSTR_BUF_SIZE).
 *  \param[in]  bufsize Size of this buffer.
 *  \param[in]  val     \c double value to be converted.
 *
 *  \return    Number of (printable) characters written. This value should be
 *             less then \p bufsize at success (see return value of C99 function
 *             snprintf() for details).
 *
 ******************************************************************************/
static gint exportConvertCoeff (char* buffer, gint bufsize, gdouble val)
{
    if (bufsize < G_ASCII_DTOSTR_BUF_SIZE)
    {
        return G_ASCII_DTOSTR_BUF_SIZE; /* indicate error (as snprintf() does) */
    } /* if */

    g_ascii_dtostr (buffer, bufsize, val);

    return (strlen (buffer));
} /* exportConvertCoeff() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/



/* FUNCTION *******************************************************************/
/** Exports a filter project to a file.
 *
 *  \param type         Export type.
 *  \param filename     Filename of file (inclusive path).
 *  \param pProject     Pointer to project data.
 *
 *  \return             Zero on succes, else an error number from errno.h.
 *  \todo               Create and use an export precision (for coefficients)
 *                      instead of using the output precision.
 *
 ******************************************************************************/
int prjFileExport (PRJFILE_EXPORT_TYPE type, const char *filename, DFCPRJ_FILTER *pProject)
{
    gchar inbuf[128];
    gchar outbuf[128];
    gchar* templname;                                   /* template filename */
    FILE *templfile;                                        /* template file */
    FILE *exportfile;

    MATHPOLY* poly = NULL;            /* numerator or denominator polynomial */
    int repcnt = 0;      /* repeat counter for a line (index of coefficient) */
    int err = 0;
    BOOL doReadTemplateLines = TRUE;              /* no END OF FILE so far ? */
    gchar* path = getPackageDirectory (DIR_ID_TEMPLATES);

    switch (type)
    {
        case PRJFILE_EXPORT_CLANG:
            templname = g_build_filename (path, PRJF_TEMPLATES_BASENAME ".c", NULL);
            break;

        case PRJFILE_EXPORT_MATLAB:
            templname = g_build_filename (path, PRJF_TEMPLATES_BASENAME ".m", NULL);
            break;

        case PRJFILE_EXPORT_PLAIN:
        default:
            templname = g_build_filename (path, PRJF_TEMPLATES_BASENAME ".txt", NULL);
            break;
    } /* switch */


    templfile = fopen (templname, "r");                /* open template file */

    if (templfile == NULL)
    {
        DEBUG_LOG ("Cannot read template file '%s'", templname);
        g_free (templname);
        g_free (path);

        return errno;
    } /* if */


    exportfile = fopen (filename, "w");

    if (exportfile == NULL)
    {
        (void) fclose (templfile);
        g_free (templname);
        g_free (path);

        DEBUG_LOG ("Cannot create export file '%s'", filename);
        return errno;
    } /* if */


    /* read line-by-line from template file
     */
    while ((err == 0) && doReadTemplateLines)
    {
        char* inptr = inbuf;
        char* outptr = outbuf;

        if ((poly == NULL) ||                         /* non-repeating line? */
            ((poly != NULL) && (repcnt > poly->degree)))   /* enough repeat? */
        {
            repcnt = 0;                        /* clear repeat count of line */
            poly = NULL;                         /* reset polynomial pointer */

            if (fgets (inbuf, sizeof (inbuf), templfile) == NULL)
            {
                doReadTemplateLines = FALSE;                  /* end of file */
                inbuf[0] = '\0';                             /* nothing read */
            } /* if */
        } /* if */


        while ((err == 0) && (*inptr != '\0'))       /* scan the read buffer */
        {
            if (*inptr == '$')              /* replacement token delimiter ? */
            {
                char* eptr = ++inptr;           /* pointer to closing dollar */

                while ((*eptr != '$') && (*eptr != '\0'))
                {
                    ++eptr;
                } /* while */

                if (*eptr == '\0')
                {
                    DEBUG_LOG ("No matching dollar found in template file");
                    err = EILSEQ; /* no matching dollar found (or buffer truncated) */
                } /* if */
                else                                       /* so far so good */
                {
                    gint written = 0;             /* number of bytes written */
                    gint bufsize = outbuf + sizeof (outbuf) - outptr;

                    *eptr = '\0';

                    switch (exportLookupTemplateKeyword (inptr))
                    {
                        case PRJF_IDKEY_NUM_DEGREE:
                            written = g_snprintf (outptr, bufsize, "%d",
                                                  pProject->filter.num.degree);
                            break; /* PRJF_IDKEY_NUM_DEGREE */

                        case PRJF_IDKEY_NUM_EXPONENT:
                            poly = &pProject->filter.num;
                            written = g_snprintf (outptr, bufsize, "%d", -repcnt);
                            break; /* PRJF_IDKEY_NUM_EXPONENT */

                        case PRJF_IDKEY_NUM_COEFF:
                            poly = &pProject->filter.num;
                            written = exportConvertCoeff (outptr, bufsize,
                                                        poly->coeff[repcnt]);
                            break; /* PRJF_IDKEY_NUM_COEFF */

                        case PRJF_IDKEY_DEN_DEGREE:
                            written = g_snprintf (outptr, bufsize, "%d",
                                                  pProject->filter.den.degree);
                            break; /* PRJF_IDKEY_DEN_DEGREE */

                        case PRJF_IDKEY_DEN_EXPONENT:
                            poly = &pProject->filter.den;
                            written = g_snprintf (outptr, bufsize, "%d", -repcnt);
                            break; /* PRJF_IDKEY_DEN_EXPONENT */

                        case PRJF_IDKEY_DEN_COEFF:
                            poly = &pProject->filter.den;
                            written = exportConvertCoeff (outptr, bufsize,
                                                        poly->coeff[repcnt]);
                            break; /* PRJF_IDKEY_DEN_COEFF */


                        case PRJF_IDKEY_INVALID:                    /* error */
                        default:                               /* unknown ID */
                            DEBUG_LOG ("Unknown keyword in template file");
                            break;
                    } /* switch */

                    if ((written >= 0) && (written < bufsize))   /* success? */
                    {
                        DEBUG_LOG ("Replacing keyword '%s' by '%s'", inptr, outptr);
                        outptr += written;
                    } /* if */
                    else
                    {
                        if (err == 0)
                        {
                            DEBUG_LOG ("Conversion error during keyword replacement");
                            err = EFBIG;
                        } /* if */
                    } /* else */

                    *eptr = '$';   /* restore old character at eptr position */
                    inptr = eptr;       /* skip the keyword in template file */
                } /* else */
            } /* if */
            else
            {
                *outptr = *inptr;
                ++outptr;
            } /* else */

            ++inptr;
        } /* while */


        if (err == 0)
        {
            *outptr = '\0';

            if (fputs (outbuf, exportfile) < 0)
            {
                err = errno;
                DEBUG_LOG ("File write error at export");
            } /* if */
            else
            {
                if (poly != NULL)
                {
                    ++repcnt;
                } /* if */
            } /* else */
        } /* if */

    } /* while */


    (void) fclose (templfile);

    if (fclose (exportfile) != 0)
    {
        err = errno;
    } /* if */

    g_free (templname);
    g_free (path);

    return err;
} /* prjFileExport() */



/* FUNCTION *******************************************************************/
/** Writes a filter project to a file.
 *
 *  \param filename     Filename of file (inclusive path).
 *  \param pProject     Pointer to project data.
 *
 *  \return             Zero on succes, else an error number from errno.h.
 ******************************************************************************/
int prjFileWrite (const char *filename, DFCPRJ_FILTER *pProject)
{
    int err;

    FILE *f = fopen (filename, "w");

    if (f == NULL)
    {
        DEBUG_LOG ("Cannot create project file '%s'", filename);
        return errno;
    } /* if */

    err = writeProject (f, pProject);

    if (fclose (f) != 0)
    {
        return errno;
    } /* if */

    return err;
} /* prjFileWrite() */



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
void prjFileRead (const char *filename, DFCPRJ_FILTER *pProject, GError **err)
{
    readProject (filename, DFCPRJ_FLAG_SAVED, err);

    if (*err == NULL)
    {
        DEBUG_LOG ("Project file '%s' successfully read", filename);

        if ((mathPolyMallocRoots (&tmpPrj.filter.num) == 0) &&
            (mathPolyMallocRoots (&tmpPrj.filter.den) == 0) &&
            FLTERR_SUCCESS (filterCheck(&tmpPrj.filter)))
        {
            *pProject = tmpPrj;
        } /* if */
        else
        {
            g_set_error (err, G_MARKUP_ERROR,
                         G_MARKUP_ERROR_INVALID_CONTENT,
                         _("Something seems wrong with the project in '%s'"), filename);
        } /* else */
    } /* if */

} /* prjFileRead() */



/* FUNCTION *******************************************************************/
/** Scans for project info (header) in a DFCGen project file. In case
 *  of an error it sets the pointer to the error structure, which itself can be
 *  used to display the original error message. Modification of the error
 *  structure pointer indicates an critical error. In that case it has to be
 *  free'ed with g_error_free().
 *
 *  \param filename     Filename of file (with path).
 *  \param pInfo        Pointer to info data buffer.
 *  \param err          Pointer to an error (structure) pointer. The pointer
 *                      to the error structure must be set NULL before calling
 *                      function prjFileScan().
 *
 ******************************************************************************/
void prjFileScan (const char *filename, DFCPRJ_INFO *pInfo, GError **err)
{
    readProject (filename, DFCPRJ_FLAG_INTERNAL, err);

    if (*err == NULL)
    {
        DEBUG_LOG ("Project file '%s' successfully scaned", filename);
        *pInfo = tmpPrj.info;
    } /* if */
} /* prjFileScan() */



/* FUNCTION *******************************************************************/
/** Frees malloc'ed memory space from a project info (author, title, desc).
 *  This function is designed as a counterpart to function prjFileScan().
 *
 *  \param pInfo        Pointer to project info data.
 *
 ******************************************************************************/
void prjFileFree (DFCPRJ_INFO *pInfo)
{
    if (pInfo->author != NULL)
    {
        g_free (pInfo->author);
        pInfo->author = NULL;
    } /* if */

    if (pInfo->title != NULL)
    {
        g_free (pInfo->title);
        pInfo->title = NULL;
    } /* if */

    if (pInfo->desc != NULL)
    {
        g_free (pInfo->desc);
        pInfo->desc = NULL;
    } /* if */
} /* prjFileFree() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
