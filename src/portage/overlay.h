// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_OVERLAY_H_
#define SRC_PORTAGE_OVERLAY_H_ 1

#include <string>
#include <vector>

#include "eixTk/inttypes.h"
#include "eixTk/null.h"

class OverlayIdent {
	private:
		void readLabel_internal(const char *Path);

	public:
		bool know_path, know_label;
		std::string path, label;
		typedef int64_t Priority;
		Priority priority;
		bool is_main;

		void init(const char *Path, const char *Label) ATTRIBUTE_NONNULL((2));

		OverlayIdent(const char *Path, const char *Label, Priority prio, bool ismain) ATTRIBUTE_NONNULL((2)) : priority(prio), is_main(ismain) {
			init(Path, Label);
		}

		OverlayIdent(const char *Path, const char *Label) ATTRIBUTE_NONNULL((2)) : priority(0), is_main(false) {
			init(Path, Label);
		}

		explicit OverlayIdent(const char *Path) ATTRIBUTE_NONNULL_ : priority(0), is_main(false) {
			init(Path, NULLPTR);
		}

		void readLabel(const char *Path) {
			if(!know_label) {
				readLabel_internal(Path);
			}
		}
		void readLabel() {
			readLabel(NULLPTR);
		}

		void setLabel(const std::string& Label) {
			label = Label;
			know_label = true;
		}

		std::string human_readable() const;

		static void init_static();
};

typedef std::vector<OverlayIdent> OverlayVec;

class RepoList : public OverlayVec {
	private:
		bool trust_cache;
		WordMap cache;

	public:
		typedef OverlayVec super;

		RepoList() : trust_cache(true) {
		}

		const char *get_path(const std::string& label);

		RepoList::iterator find_filename(const char *search, bool parent_ok, bool resolve_mask) ATTRIBUTE_NONNULL_;
		RepoList::iterator find_filename(const char *search, bool parent_ok) ATTRIBUTE_NONNULL_ {
			return find_filename(search, parent_ok, true);
		}
		RepoList::iterator find_filename(const char *search) ATTRIBUTE_NONNULL_ {
			return find_filename(search, false);
		}

		void set_priority(OverlayIdent *overlay);

		RepoList::const_iterator second() const {
			RepoList::const_iterator i(begin());
			if(i != end()) {
				++i;
			}
			return i;
		}

		void push_back(const OverlayIdent& s, bool no_path_dupes);
		void push_back(const OverlayIdent& s) {
			push_back(s, false);
		}

		void clear();
};

#endif  // SRC_PORTAGE_OVERLAY_H_
