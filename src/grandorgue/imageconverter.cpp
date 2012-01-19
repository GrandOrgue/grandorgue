/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * MA 02111-1307, USA.
 */

#include <stdio.h>

int main(int ac, const char* ag[])
{
	FILE* in, *out;
	unsigned char c;
	unsigned pos = 0;

	if (ac != 4)
	{
		printf("Usage: %s graphic-file output-file structure-name\n", ag[0]);
		return 1;
	}
	in = fopen(ag[1], "rb");
	if (!in)
	{
		printf("Error opening %s\n", ag[1]);
		return 1;
	}
	out = fopen(ag[2], "w");
	if (!out)
	{
		printf("Error opening %s\n", ag[2]);
		return 1;
	}

	fprintf(out, "#include <wx/bitmap.h>\n");
	fprintf(out, "#include <wx/image.h>\n");
	fprintf(out, "#include <wx/mstream.h>\n");
	fprintf(out, "static const char image_%s[] = {\n", ag[3]);

	while(fread(&c, 1, 1, in) == 1)
	{
		pos++;
		fprintf(out, "  0x%02x,", c);
		if (!(pos%16))
			fprintf(out, "\n");
	}
	fprintf(out, "\n};\n\n");

	fprintf(out, "wxBitmap GetImage_%s()\n", ag[3]);
	fprintf(out, "{\n");
	fprintf(out, "\tstatic wxMemoryInputStream is(image_%s, sizeof(image_%s));\n", ag[3], ag[3]);
	fprintf(out, "\tstatic wxBitmap bitmap(wxImage(is, wxBITMAP_TYPE_ANY, -1), -1);\n");
	fprintf(out, "\treturn bitmap;\n");
	fprintf(out, "}\n");

	fprintf(out, "wxBitmap GetImage_%s_r()\n", ag[3]);
	fprintf(out, "{\n");
	fprintf(out, "\tstatic wxMemoryInputStream is(image_%s, sizeof(image_%s));\n", ag[3], ag[3]);
	fprintf(out, "\tstatic wxBitmap bitmap(wxImage(is, wxBITMAP_TYPE_ANY, -1).Rotate90(), -1);\n");
	fprintf(out, "\treturn bitmap;\n");
	fprintf(out, "}\n");

	if (!feof(in) || ferror(in) || ferror(out))
	{
		printf("Processing error\n");
		return 1;
	}

	fclose(in);
	fclose(out);

	return 0;
}
