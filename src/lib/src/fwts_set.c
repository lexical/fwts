/*
 * Copyright (C) 2010-2014 Canonical
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "fwts.h"

/*
 *  fwts_set()
 *	write text to a given file, used to set
 *	values in /sys or /proc
 */
int fwts_set(const char *text, const char *file)
{
	FILE *fp;

	if ((fp = fopen(file, "w")) == NULL)
		return FWTS_ERROR;

	fprintf(fp, "%s\n", text);
	fclose(fp);

	return FWTS_OK;
}
