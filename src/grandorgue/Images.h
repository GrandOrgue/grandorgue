/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GO_IMAGES_H /* using IMAGES_H causes a preprocessor conflict */
#define GO_IMAGES_H

class wxImage;

#define DECLARE_IMAGE(A) wxImage GetImage_##A();

DECLARE_IMAGE(gauge);
DECLARE_IMAGE(help);
DECLARE_IMAGE(memory);
DECLARE_IMAGE(open);
DECLARE_IMAGE(panic);
DECLARE_IMAGE(polyphony);
DECLARE_IMAGE(properties);
DECLARE_IMAGE(record);
DECLARE_IMAGE(reload);
DECLARE_IMAGE(save);
DECLARE_IMAGE(set);
DECLARE_IMAGE(settings);
DECLARE_IMAGE(Splash);
DECLARE_IMAGE(transpose);
DECLARE_IMAGE(reverb);
DECLARE_IMAGE(volume);
DECLARE_IMAGE(ASIO);

DECLARE_IMAGE(GOIcon);

#undef DECLARE_IMAGE

#endif
