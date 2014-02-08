/*
 * options.c
 * Copyright (C) 1998-2004 A.J. van Os; Released under GNU GPL
 *
 * Description:
 * Read and write the options
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "antiword.h"

#define LEAFNAME_SIZE		(32+1)

/* Current values for options */
static options_type	tOptionsCurr;
typedef struct papersize_tag {
	char	szName[16];	/* Papersize name */
	USHORT	usWidth;	/* In points */
	USHORT	usHeight;	/* In points */
} papersize_type;

static const papersize_type atPaperSizes[] = {
	{	"10x14",	 720,	1008	},
	{	"a3",		 842,	1191	},
	{	"a4",		 595,	 842	},
	{	"a5",		 420,	 595	},
	{	"b4",		 729,	1032	},
	{	"b5",		 516,	 729	},
	{	"executive",	 540,	 720	},
	{	"folio",	 612,	 936	},
	{	"legal",	 612,	1008	},
	{	"letter",	 612,	 792	},
	{	"note",		 540,	 720	},
	{	"quarto",	 610,	 780	},
	{	"statement",	 396,	 612	},
	{	"tabloid",	 792,	1224	},
	{	"",		   0,	   0	},
};
/* Default values for options */
static const options_type	tOptionsDefault = {
	DEFAULT_SCREEN_WIDTH,
	conversion_text,
	TRUE,
	TRUE,
	FALSE,
	encoding_latin_1,
	INT_MAX,
	INT_MAX,
};


/*
 * bCorrectPapersize - see if the papersize is correct
 *
 * TRUE if the papersize is correct, otherwise FALSE
 */
static BOOL
bCorrectPapersize(const char *szName, conversion_type eConversionType)
{
	const papersize_type	*pPaperSize;

	for (pPaperSize = atPaperSizes;
	     pPaperSize->szName[0] != '\0';
	     pPaperSize++) {
		if (!STRCEQ(pPaperSize->szName,  szName)) {
			continue;
		}
		DBG_DEC(pPaperSize->usWidth);
		DBG_DEC(pPaperSize->usHeight);
		tOptionsCurr.eConversionType = eConversionType;
		tOptionsCurr.iPageHeight = (int)pPaperSize->usHeight;
		tOptionsCurr.iPageWidth = (int)pPaperSize->usWidth;
		return TRUE;
	}
	return FALSE;
} /* end of bCorrectPapersize */

/*
 * szCreateSuffix - create a suffix for the file
 *
 * Returns the suffix
 */
static const char *
szCreateSuffix(const char *szLeafname)
{
	const char	*pcDot;

	pcDot = strrchr(szLeafname, '.');
	if (pcDot != NULL && STRCEQ(pcDot, ".txt")) {
		/* There is already a .txt suffix, no need for another one */
		return "";
	}
	return ".txt";
} /* end of szCreateSuffix */

/*
 * eMappingFile2Encoding - convert the mapping file to an encoding
 */
static encoding_type
eMappingFile2Encoding(const char *szLeafname)
{
	char	szMappingFile[LEAFNAME_SIZE+4];

	fail(szLeafname == NULL);

	if (strlen(szLeafname) + 4 >= sizeof(szMappingFile)) {
		DBG_MSG(szLeafname);
		return encoding_latin_1;
	}

	sprintf(szMappingFile, "%s%s", szLeafname, szCreateSuffix(szLeafname));

	DBG_MSG(szMappingFile);

	if (STRCEQ(szMappingFile, MAPPING_FILE_UTF_8)) {
		return encoding_utf_8;
	}
	if (STRCEQ(szMappingFile, MAPPING_FILE_CP852) ||
	    STRCEQ(szMappingFile, MAPPING_FILE_CP1250) ||
	    STRCEQ(szMappingFile, MAPPING_FILE_8859_2)) {
		return encoding_latin_2;
	}
	if (STRCEQ(szMappingFile, MAPPING_FILE_KOI8_R) ||
	    STRCEQ(szMappingFile, MAPPING_FILE_KOI8_U) ||
	    STRCEQ(szMappingFile, MAPPING_FILE_CP866) ||
	    STRCEQ(szMappingFile, MAPPING_FILE_CP1251) ||
	    STRCEQ(szMappingFile, MAPPING_FILE_8859_5)) {
		return encoding_cyrillic;
	}
	return encoding_latin_1;
} /* end of eMappingFile2Encoding */

/*
 * pOpenCharacterMappingFile - open the mapping file
 *
 * Returns the file pointer or NULL
 */
static FILE *
pOpenCharacterMappingFile(const char *szLeafname)
{
	FILE	*pFile;
	const char	*szHome, *szAntiword, *szSuffix;
	size_t	tFilenameLen;
	char	szMappingFile[PATH_MAX+1];

	if (szLeafname == NULL || szLeafname[0] == '\0') {
		return NULL;
	}

	DBG_MSG(szLeafname);

	/* Set the suffix */
	szSuffix = szCreateSuffix(szLeafname);

	/* Set length */
	tFilenameLen = strlen(szLeafname) + strlen(szSuffix);

	/* Try the environment version of the mapping file */
	szAntiword = szGetAntiwordDirectory();
	if (szAntiword != NULL && szAntiword[0] != '\0') {
	    if (strlen(szAntiword) + tFilenameLen <
		sizeof(szMappingFile) -
		sizeof(FILE_SEPARATOR)) {
			sprintf(szMappingFile,
				"%s" FILE_SEPARATOR "%s%s",
				szAntiword, szLeafname, szSuffix);
			DBG_MSG(szMappingFile);
			pFile = fopen(szMappingFile, "r");
			if (pFile != NULL) {
				return pFile;
			}
		} else {
			werr(0, "Environment mappingfilename ignored");
		}
	}

	/* Try the local version of the mapping file */
	szHome = szGetHomeDirectory();
	if (strlen(szHome) + tFilenameLen <
	    sizeof(szMappingFile) -
	    sizeof(ANTIWORD_DIR) -
	    2 * sizeof(FILE_SEPARATOR)) {
		sprintf(szMappingFile,
			"%s" FILE_SEPARATOR ANTIWORD_DIR FILE_SEPARATOR "%s%s",
			szHome, szLeafname, szSuffix);
		DBG_MSG(szMappingFile);
		pFile = fopen(szMappingFile, "r");
		if (pFile != NULL) {
			return pFile;
		}
	} else {
		werr(0, "Local mappingfilename too long, ignored");
	}

	/* Try the global version of the mapping file */
	if (tFilenameLen <
	    sizeof(szMappingFile) -
	    sizeof(GLOBAL_ANTIWORD_DIR) -
	    sizeof(FILE_SEPARATOR)) {
		sprintf(szMappingFile,
			GLOBAL_ANTIWORD_DIR FILE_SEPARATOR "%s%s",
			szLeafname, szSuffix);
		DBG_MSG(szMappingFile);
		pFile = fopen(szMappingFile, "r");
		if (pFile != NULL) {
			return pFile;
		}
	} else {
		werr(0, "Global mappingfilename too long, ignored");
	}
	werr(0, "I can't open your mapping file (%s%s)\n"
		"It is not in '%s" FILE_SEPARATOR ANTIWORD_DIR "' nor in '"
		GLOBAL_ANTIWORD_DIR "'.", szLeafname, szSuffix, szHome);
	return NULL;
} /* end of pOpenCharacterMappingFile */

/*
 * vCloseCharacterMappingFile - close the mapping file
 */
static void
vCloseCharacterMappingFile(FILE *pFile)
{
	(void)fclose(pFile);
} /* end of pCloseCharacterMappingFile */


/*
 * iReadOptions - read options
 *
 * returns:	-1: error
 *		 0: help
 *		>0: index first file argument
 */
int
iReadOptions(int argc, char **argv)
{
	extern	char	*optarg;
	extern int	optind;
	char	*pcChar, *szTmp;
	int	iChar;
	char	szLeafname[LEAFNAME_SIZE];
	FILE	*pCharacterMappingFile;
	int	iTmp;
	BOOL	bSuccess;

	DBG_MSG("iReadOptions");

/* Defaults */
	tOptionsCurr = tOptionsDefault;

/* Environment */
	szTmp = getenv("COLUMNS");
	if (szTmp != NULL) {
		DBG_MSG(szTmp);
		iTmp = (int)strtol(szTmp, &pcChar, 10);
		if (*pcChar == '\0') {
			iTmp -= 4;	/* This is for the edge */
			if (iTmp < MIN_SCREEN_WIDTH) {
				iTmp = MIN_SCREEN_WIDTH;
			} else if (iTmp > MAX_SCREEN_WIDTH) {
				iTmp = MAX_SCREEN_WIDTH;
			}
			tOptionsCurr.iParagraphBreak = iTmp;
			DBG_DEC(tOptionsCurr.iParagraphBreak);
		}
	}
	strncpy(szLeafname, szGetDefaultMappingFile(), sizeof(szLeafname) - 1);
	szLeafname[sizeof(szLeafname) - 1] = '\0';
/* Command line */
	while ((iChar = getopt(argc, argv, "La:fhi:m:p:rstw:x:")) != -1) {
		switch (iChar) {
		case 'f':
			tOptionsCurr.eConversionType = conversion_fmt_text;
			break;
		case 'h':
			return 0;
		case 'm':
			if (tOptionsCurr.eConversionType == conversion_xml) {
				werr(0, "XML doesn't need a mapping file");
				break;
			}
			strncpy(szLeafname, optarg, sizeof(szLeafname) - 1);
			szLeafname[sizeof(szLeafname) - 1] = '\0';
			DBG_MSG(szLeafname);
			break;
		case 'p':
			if (!bCorrectPapersize(optarg, conversion_ps)) {
				werr(0, "-p without a valid papersize");
				return -1;
			}
			break;
		case 'r':
			tOptionsCurr.bRemoveRemovedText = FALSE;
			break;
		case 's':
			tOptionsCurr.bHideHiddenText = FALSE;
			break;
		case 't':
			tOptionsCurr.eConversionType = conversion_text;
			break;
		case 'w':
			iTmp = (int)strtol(optarg, &pcChar, 10);
			if (*pcChar == '\0') {
				if (iTmp != 0 && iTmp < MIN_SCREEN_WIDTH) {
					iTmp = MIN_SCREEN_WIDTH;
				} else if (iTmp > MAX_SCREEN_WIDTH) {
					iTmp = MAX_SCREEN_WIDTH;
				}
				tOptionsCurr.iParagraphBreak = iTmp;
				DBG_DEC(tOptionsCurr.iParagraphBreak);
			}
			break;
		case 'x':
			if (STREQ(optarg, "db")) {
				tOptionsCurr.iParagraphBreak = 0;
				tOptionsCurr.eConversionType = conversion_xml;
				strcpy(szLeafname, MAPPING_FILE_UTF_8);
			} else {
				werr(0, "-x %s is not supported", optarg);
				return -1;
			}
			break;
		default:
			return -1;
		}
	}

	tOptionsCurr.eEncoding = eMappingFile2Encoding(szLeafname);
	DBG_DEC(tOptionsCurr.eEncoding);

	if (tOptionsCurr.eConversionType == conversion_ps &&
	    tOptionsCurr.eEncoding == encoding_utf_8) {
		werr(0,
		"The combination PostScript and UTF-8 is not supported");
		return -1;
	}

	if (tOptionsCurr.eConversionType == conversion_pdf &&
	    tOptionsCurr.eEncoding == encoding_utf_8) {
		werr(0,
		"The combination PDF and UTF-8 is not supported");
		return -1;
	}

	if (tOptionsCurr.eConversionType == conversion_pdf &&
	    tOptionsCurr.eEncoding == encoding_cyrillic) {
		werr(0,
		"The combination PDF and Cyrillic is not supported");
		return -1;
	}

	if (tOptionsCurr.eConversionType == conversion_ps ||
	    tOptionsCurr.eConversionType == conversion_pdf) {
		/* PostScript or PDF mode */
		if (tOptionsCurr.bUseLandscape) {
			/* Swap the page height and width */
			iTmp = tOptionsCurr.iPageHeight;
			tOptionsCurr.iPageHeight = tOptionsCurr.iPageWidth;
			tOptionsCurr.iPageWidth = iTmp;
		}
		/* The paragraph break depends on the width of the paper */
		tOptionsCurr.iParagraphBreak = iMilliPoints2Char(
			(long)tOptionsCurr.iPageWidth * 1000 -
			lDrawUnits2MilliPoints(
				PS_LEFT_MARGIN + PS_RIGHT_MARGIN));
		DBG_DEC(tOptionsCurr.iParagraphBreak);
	}

	pCharacterMappingFile = pOpenCharacterMappingFile(szLeafname);
	if (pCharacterMappingFile != NULL) {
		bSuccess = bReadCharacterMappingTable(pCharacterMappingFile);
		vCloseCharacterMappingFile(pCharacterMappingFile);
	} else {
		bSuccess = FALSE;
	}
	return bSuccess ? optind : -1;
} /* end of iReadOptions */

/*
 * vGetOptions - get a copy of the current option values
 */
void
vGetOptions(options_type *pOptions)
{
	fail(pOptions == NULL);

	*pOptions = tOptionsCurr;
} /* end of vGetOptions */
