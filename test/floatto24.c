/*
	Audio File Library

	Copyright (C) 2001, Silicon Graphics, Inc.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be
	useful, but WITHOUT ANY WARRANTY; without even the implied
	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the Free
	Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
	MA 02111-1307, USA.
*/

/*
	floatto24.c

	This program creates a BICSF floating-point sound file and then
	reads the sample data back as 24-bit data (i.e. 32-bit integers
	with the high 8 bits equal to the sign extension of the lower
	24 bits).
*/

#include <audiofile.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_FILE "/tmp/test.sf"
#define FRAME_COUNT 10

/*
	When converted to samples with width 24 bits, the samples
	should have the following values:
*/

const float samples[] =
{
	0,
	0.5,
	-0.5,
	0,
	1,
	-1,
	-0.25,
	0.25,
	0.75,
	-0.75
};

const int referenceConvertedSamples[] =
{
	0,
	4194303,	/* = (2^23 - 1) / 2 */
	-4194303,
	0,
	8388607,	/* = 2^23 - 1 */
	-8388607,
	-2097151,
	2097151,	/* = (2^23 - 1) / 4 */
	6291455,	/* = (2^23 - 1) * 3 / 4 */
	-6291455
};

int main (int argc, char **argv)
{
	AFfilehandle	file;
	AFfilesetup	setup;
	AFframecount	framesWritten, framesRead;

	int	readsamples[FRAME_COUNT] = {-1000, -1001, -1002, -1003,
			-1004, -1005, -1006, -1007};
	int	i;

	if ((setup = afNewFileSetup()) == AF_NULL_FILESETUP)
	{
		fprintf(stderr, "Could not allocate file setup.\n");
		exit(EXIT_FAILURE);
	}

	afInitFileFormat(setup, AF_FILE_IRCAM);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_FLOAT, 32);

	file = afOpenFile(TEST_FILE, "w", setup);
	if (file == AF_NULL_FILEHANDLE)
	{
		printf("could not open file for writing\n");
		exit(-1);
	}

	framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK, (void *) samples,
		FRAME_COUNT);

	if (framesWritten != FRAME_COUNT)
	{
		fprintf(stderr, "Wrong number of frames read.\n");
		exit(EXIT_FAILURE);
	}

	if (afCloseFile(file) != 0)
	{
		fprintf(stderr, "Closing file returned non-zero status.\n");
		exit(EXIT_FAILURE);
	}

	file = afOpenFile(TEST_FILE, "r", AF_NULL_FILESETUP);
	if (file == AF_NULL_FILEHANDLE)
	{
		fprintf(stderr, "Could not open file for writing.\n");
		exit(EXIT_FAILURE);
	}

	if (afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK,
		AF_SAMPFMT_TWOSCOMP, 24) != 0)
	{
		fprintf(stderr, "afSetVirtualSampleFormat returned non-zero status.\n");
		exit(EXIT_FAILURE);
	}

	framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readsamples,
		FRAME_COUNT);

	if (framesRead != FRAME_COUNT)
	{
		fprintf(stderr, "Wrong number of frames read.\n");
		exit(EXIT_FAILURE);
	}

	for (i=0; i<framesRead; i++)
	{
#ifdef DEBUG
		printf("[%d] = %d\n", i, readsamples[i]);
#endif

		if (readsamples[i] == -1000 - i)
		{
			fprintf(stderr, "Data in destination array untouched.\n");
			exit(EXIT_FAILURE);
		}

		/*
			Ensure that the high-order 8 bits represent
			sign extension: only 0x00 (+) or 0xff (-) is
			valid.
		*/
		if ((readsamples[i] & 0xff000000) != 0x000000 &&
			(readsamples[i] & 0xff000000) != 0xff000000)
		{
			fprintf(stderr, "Data is not within range of "
				"{-2^23, ..., 2^23-1}.\n");
			exit(EXIT_FAILURE);
		}

		if (readsamples[i] != referenceConvertedSamples[i])
		{
			fprintf(stderr, "Data doesn't match reference data.\n");
			exit(EXIT_FAILURE);
		}
	}

	if (afCloseFile(file) != 0)
	{
		fprintf(stderr, "Closing file returned non-zero status.\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}