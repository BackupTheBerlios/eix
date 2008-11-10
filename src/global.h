// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#if !defined(EIX__EIXRC_GLOBALS_H__)
#define EIX__EIXRC_GLOBALS_H__

#include <eixrc/eixrc.h>

/** Return a static eixrc. */
EixRc &get_eixrc(const char *varprefix);

void fill_defaults_part_1(EixRc &eixrc);
void fill_defaults_part_2(EixRc &eixrc);
void fill_defaults_part_3(EixRc &eixrc);
void fill_defaults_part_4(EixRc &eixrc);

#endif /* EIX__EIXRC_GLOBALS_H__ */
