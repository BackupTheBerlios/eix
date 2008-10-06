// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __METADATA_H__
#define __METADATA_H__

#include <cache/base.h>

class MetadataCache : public BasicCache {
	protected:
		bool metadata, flat, have_override_path;
		std::string override_path;
		std::string m_type;

		void setType(bool set_flat, bool set_metadata);
	public:
		bool initialize(const std::string &name);

		bool readCategory(Category &vec) throw(ExBasic);

		bool use_prefixport() const
		{ return metadata; }

		const char *getType() const
		{ return m_type.c_str(); }
};

#endif /* __METADATA_H__ */
