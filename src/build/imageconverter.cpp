/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include <stdio.h>

int main(int ac, const char *ag[]) {
  FILE *in, *out;
  unsigned char c;
  unsigned pos = 0;

  if (ac != 4) {
    printf("Usage: %s graphic-file output-file structure-name\n", ag[0]);
    return 1;
  }
  in = fopen(ag[1], "rb");
  if (!in) {
    printf("Error opening %s\n", ag[1]);
    return 1;
  }
  out = fopen(ag[2], "w");
  if (!out) {
    printf("Error opening %s\n", ag[2]);
    return 1;
  }

  fprintf(out, "#include <wx/image.h>\n");
  fprintf(out, "#include <wx/mstream.h>\n");
  fprintf(out, "static const unsigned char image_%s[] = {\n", ag[3]);

  while (fread(&c, 1, 1, in) == 1) {
    pos++;
    fprintf(out, "  0x%02x,", c);
    if (!(pos % 16))
      fprintf(out, "\n");
  }
  fprintf(out, "\n};\n\n");

  fprintf(out, "wxImage GetImage_%s()\n", ag[3]);
  fprintf(out, "{\n");
  fprintf(
    out,
    "\tstatic wxMemoryInputStream is(image_%s, sizeof(image_%s));\n",
    ag[3],
    ag[3]);
  fprintf(out, "\tstatic wxImage image(is, wxBITMAP_TYPE_ANY, -1);\n");
  fprintf(out, "\treturn image;\n");
  fprintf(out, "}\n");

  if (!feof(in) || ferror(in) || ferror(out)) {
    printf("Processing error\n");
    return 1;
  }

  fclose(in);
  fclose(out);

  return 0;
}
