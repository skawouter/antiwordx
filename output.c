/*
 * output.c
 * Copyright (C) 2002-2004 A.J. van Os; Released under GNU GPL
 *
 * Description:
 * Generic output generating functions
 */

#include "antiword.h"

static conversion_type	eConversionType = conversion_unknown;
static encoding_type	eEncoding = encoding_neutral;


/*
 * vPrologue1 - get options and call a specific initialization
 */
static void
vPrologue1(diagram_type *pDiag, const char *szTask, const char *szFilename)
{
	options_type	tOptions;

	fail(pDiag == NULL);
	fail(szTask == NULL || szTask[0] == '\0');

	vGetOptions(&tOptions);
	eConversionType = tOptions.eConversionType;
	eEncoding = tOptions.eEncoding;

	switch (eConversionType) {
	case conversion_text:
		vPrologueTXT(pDiag, &tOptions);
		break;
	case conversion_fmt_text:
		vPrologueFMT(pDiag, &tOptions);
		break;
	default:
		DBG_DEC(eConversionType);
		break;
	}
} /* end of vPrologue1 */

/*
 * vEpilogue - clean up after everything is done
 */
static void
vEpilogue(diagram_type *pDiag)
{
	switch (eConversionType) {
	case conversion_text:
	case conversion_fmt_text:
		vEpilogueTXT(pDiag->pOutFile);
		break;
	default:
		DBG_DEC(eConversionType);
		break;
	}
} /* end of vEpilogue */

/*
 * vImagePrologue - perform image initialization
 */
void
vImagePrologue(diagram_type *pDiag, const imagedata_type *pImg)
{
    return;
} /* end of vImagePrologue */

/*
 * vImageEpilogue - clean up an image
 */
void
vImageEpilogue(diagram_type *pDiag)
{
    return;
} /* end of vImageEpilogue */

/*
 * bAddDummyImage - add a dummy image
 *
 * return TRUE when successful, otherwise FALSE
 */
BOOL
bAddDummyImage(diagram_type *pDiag, const imagedata_type *pImg)
{
    return FALSE;
} /* end of bAddDummyImage */

/*
 * pCreateDiagram - create and initialize a diagram
 *
 * remark: does not return if the diagram can't be created
 */
diagram_type *
pCreateDiagram(const char *szTask, const char *szFilename)
{
	diagram_type	*pDiag;

	fail(szTask == NULL || szTask[0] == '\0');
	DBG_MSG("pCreateDiagram");

	/* Get the necessary memory */
	pDiag = xmalloc(sizeof(diagram_type));
	/* Initialization */
	pDiag->pOutFile = stdout;
	vPrologue1(pDiag, szTask, szFilename);
	/* Return success */
	return pDiag;
} /* end of pCreateDiagram */

/*
 * vDestroyDiagram - remove a diagram by freeing the memory it uses
 */
void
vDestroyDiagram(diagram_type *pDiag)
{
	DBG_MSG("vDestroyDiagram");

	fail(pDiag == NULL);

	if (pDiag == NULL) {
		return;
	}
	vEpilogue(pDiag);
	pDiag = xfree(pDiag);
} /* end of vDestroyDiagram */

/*
 * vPrologue2 - call a specific initialization
 */
void
vPrologue2(diagram_type *pDiag, int iWordVersion)
{
    return;
} /* end of vPrologue2 */

/*
 * vSubstring2Diagram - put a sub string into a diagram
 */
void
vSubstring2Diagram(diagram_type *pDiag,
	char *szString, size_t tStringLength, long lStringWidth,
	UCHAR ucFontColor, USHORT usFontstyle, drawfile_fontref tFontRef,
	USHORT usFontSize, USHORT usMaxFontSize)
{
	switch (eConversionType) {
	case conversion_text:
		vSubstringTXT(pDiag, szString, tStringLength, lStringWidth);
		break;
	case conversion_fmt_text:
		vSubstringFMT(pDiag, szString, tStringLength, lStringWidth,
				usFontstyle);
		break;
	default:
		DBG_DEC(eConversionType);
		break;
	}
	pDiag->lXleft += lStringWidth;
} /* end of vSubstring2Diagram */

/*
 * Create an end of paragraph
 */
void
vEndOfParagraph(diagram_type *pDiag,
	drawfile_fontref tFontRef, USHORT usFontSize, long lAfterIndentation)
{
	fail(pDiag == NULL);
	fail(pDiag->pOutFile == NULL);
	fail(usFontSize < MIN_FONT_SIZE || usFontSize > MAX_FONT_SIZE);
	fail(lAfterIndentation < 0);

	switch (eConversionType) {
	case conversion_text:
	case conversion_fmt_text:
		vEndOfParagraphTXT(pDiag, lAfterIndentation);
		break;
	default:
		DBG_DEC(eConversionType);
		break;
	}
} /* end of vEndOfParagraph */

/*
 * Create an end of page
 */
void
vEndOfPage(diagram_type *pDiag, long lAfterIndentation, BOOL bNewSection)
{
	switch (eConversionType) {
	case conversion_text:
	case conversion_fmt_text:
		vEndOfPageTXT(pDiag, lAfterIndentation);
		break;
	default:
		DBG_DEC(eConversionType);
		break;
	}
} /* end of vEndOfPage */
