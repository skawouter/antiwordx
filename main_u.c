/*
 * main_u.c
 *
 * Released under GPL
 *
 * Copyright (C) 1998-2004 A.J. van Os
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Description:
 * The main program of 'antiword' (Unix version)
 */

#include <stdio.h>
#include <stdlib.h>
#include "version.h"
#include "antiword.h"

/* The name of this program */
static const char	*szTask = NULL;


static void
vUsage(void)
{
	fprintf(stderr, "\tName: %s\n", szTask);
	fprintf(stderr, "\tPurpose: "PURPOSESTRING"\n");
	fprintf(stderr, "\tAuthor: "AUTHORSTRING"\n");
	fprintf(stderr, "\tVersion: "VERSIONSTRING);
	fprintf(stderr, "\n");
	fprintf(stderr, "\tStatus: "STATUSSTRING"\n");
	fprintf(stderr,
		"\tUsage: %s [switches] wordfile1 [wordfile2 ...]\n", szTask);
	fprintf(stderr,
		"\tSwitches: [-f|-t]"
		"[-m mapping][-w #][-s]\n");
	fprintf(stderr, "\t\t-f formatted text output\n");
	fprintf(stderr, "\t\t-t text output (default)\n");
	fprintf(stderr, "\t\t-m <mapping> character mapping file\n");
	fprintf(stderr, "\t\t-w <width> in characters of text output\n");
	fprintf(stderr, "\t\t-r Show removed text\n");
	fprintf(stderr, "\t\t-s Show hidden (by Word) text\n");
} /* end of vUsage */

/*
 * pStdin2TmpFile - save stdin in a temporary file
 *
 * returns: the pointer to the temporary file or NULL
 */
static FILE *
pStdin2TmpFile(long *lFilesize)
{
	FILE	*pTmpFile;
	size_t	tSize;
	BOOL	bFailure;
	UCHAR	aucBytes[BUFSIZ];

	DBG_MSG("pStdin2TmpFile");

	fail(lFilesize == NULL);

	/* Open the temporary file */
	pTmpFile = tmpfile();
	if (pTmpFile == NULL) {
		return NULL;
	}

	/* Copy stdin to the temporary file */
	*lFilesize = 0;
	bFailure = TRUE;
	for (;;) {
		tSize = fread(aucBytes, 1, sizeof(aucBytes), stdin);
		if (tSize == 0) {
			bFailure = feof(stdin) == 0;
			break;
		}
		if (fwrite(aucBytes, 1, tSize, pTmpFile) != tSize) {
			bFailure = TRUE;
			break;
		}
		*lFilesize += (long)tSize;
	}

	/* Deal with the result of the copy action */
	if (bFailure) {
		*lFilesize = 0;
		(void)fclose(pTmpFile);
		return NULL;
	}
	rewind(pTmpFile);
	return pTmpFile;
} /* end of pStdin2TmpFile */

/*
 * bProcessFile - process a single file
 *
 * returns: TRUE when the given file is a supported Word file, otherwise FALSE
 */
static BOOL
bProcessFile(const char *szFilename)
{
	FILE		*pFile;
	diagram_type	*pDiag;
	long		lFilesize;
	int		iWordVersion;
	BOOL		bResult;

	fail(szFilename == NULL || szFilename[0] == '\0');

	DBG_MSG(szFilename);

	if (szFilename[0] == '-' && szFilename[1] == '\0') {
		pFile = pStdin2TmpFile(&lFilesize);
		if (pFile == NULL) {
			werr(0, "I can't save the standard input to a file");
			return FALSE;
		}
	} else {
		pFile = fopen(szFilename, "rb");
		if (pFile == NULL) {
			werr(0, "I can't open '%s' for reading", szFilename);
			return FALSE;
		}

		lFilesize = lGetFilesize(szFilename);
		if (lFilesize < 0) {
			(void)fclose(pFile);
			werr(0, "I can't get the size of '%s'", szFilename);
			return FALSE;
		}
	}

	iWordVersion = iGuessVersionNumber(pFile, lFilesize);
	if (iWordVersion < 0 || iWordVersion == 3) {
		if (bIsRtfFile(pFile)) {
			werr(0, "%s is not a Word Document."
				" It is probably a Rich Text Format file",
				szFilename);
		} if (bIsWordPerfectFile(pFile)) {
			werr(0, "%s is not a Word Document."
				" It is probably a Word Perfect file",
				szFilename);
		} else {
			werr(0, "%s is not a Word Document.", szFilename);
		}
		(void)fclose(pFile);
		return FALSE;
	}
	/* Reset any reading done during file testing */
	rewind(pFile);

	pDiag = pCreateDiagram(szTask, szFilename);
	if (pDiag == NULL) {
		(void)fclose(pFile);
		return FALSE;
	}

	bResult = bWordDecryptor(pFile, lFilesize, pDiag);
	vDestroyDiagram(pDiag);

	(void)fclose(pFile);
	return bResult;
} /* end of bProcessFile */

int
main(int argc, char **argv)
{
	options_type	tOptions;
	const char	*szWordfile;
	int	iFirst, iIndex, iGoodCount;
	BOOL	bUsage, bMultiple, bUseTXT;

	if (argc <= 0) {
		return EXIT_FAILURE;
	}

	szTask = szBasename(argv[0]);

	if (argc <= 1) {
		iFirst = 1;
		bUsage = TRUE;
	} else {
		iFirst = iReadOptions(argc, argv);
		bUsage = iFirst <= 0;
	}
	if (bUsage) {
		vUsage();
		return iFirst < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	vGetOptions(&tOptions);

	bMultiple = argc - iFirst > 1;
	bUseTXT = tOptions.eConversionType == conversion_text ||
		tOptions.eConversionType == conversion_fmt_text;
	iGoodCount = 0;



	for (iIndex = iFirst; iIndex < argc; iIndex++) {
		if (bMultiple && bUseTXT) {
			szWordfile = szBasename(argv[iIndex]);
			fprintf(stdout, "::::::::::::::\n");
			fprintf(stdout, "%s\n", szWordfile);
			fprintf(stdout, "::::::::::::::\n");
		}
		if (bProcessFile(argv[iIndex])) {
			iGoodCount++;
		}
	}

	DBG_DEC(iGoodCount);
	return iGoodCount <= 0 ? EXIT_FAILURE : EXIT_SUCCESS;
} /* end of main */
