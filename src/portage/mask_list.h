// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_MASK_LIST_H_
#define SRC_PORTAGE_MASK_LIST_H_ 1

#include <fnmatch.h>

#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/ptr_list.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "portage/keywords.h"
#include "portage/mask.h"
#include "portage/package.h"

class Package;
class Version;

template<typename m_Type> class Masks : public std::list<m_Type> {
		std::string full;

	public:
		using std::list<m_Type>::push_back;
		using std::list<m_Type>::begin;
		using std::list<m_Type>::end;
		typedef typename std::list<m_Type>::iterator iterator;
		typedef typename std::list<m_Type>::const_iterator const_iterator;

		Masks(const std::string& f, const m_Type& m) : std::list<m_Type>(1, m), full(f) {
		}

		void add(const m_Type& m) {
			push_back(m);
		}

		bool match_full(const char *cat_name) const ATTRIBUTE_NONNULL_ {
			return !fnmatch(full.c_str(), cat_name, FNM_PATHNAME);
		}
};

template<typename m_Type> class MaskList : std::vector<Masks<m_Type> > {
	private:
		typedef typename std::vector<Masks<m_Type> > super;
		typedef typename super::size_type size_type;
		typedef typename super::iterator iterator;
		typedef typename super::const_iterator const_iterator;
		typedef typename Masks<m_Type>::const_iterator m_const_iterator;
		using super::begin;
		using super::end;
		using super::size;
		using super::operator[];
		typedef typename std::map<std::string, Masks<m_Type> > LookupType;
		typedef typename std::map<std::string, size_type> MapType;
		typedef typename MapType::const_iterator map_const_iterator;
		typedef typename LookupType::iterator lookup_iterator;
		typedef typename LookupType::const_iterator lookup_const_iterator;
		using super::push_back;

		// We collect only the patterns in the vector.
		// The exact ones are in a lookup-table.
		LookupType exact_name;
		MapType full_index;

	public:
		typedef typename eix::ptr_list<const m_Type> Get;

		bool empty() const {
			return (exact_name.empty() && super::empty());
		}

		void clear() {
			super::clear();
			exact_name.clear();
			full_index.clear();
		}

		bool match_full(const std::string& full) const {
			if(exact_name.find(full) != exact_name.end()) {
				return true;
			}
			for(const_iterator it(begin()); likely(it != end()); ++it) {
				if(unlikely(it->match_full(full.c_str()))) {
					return true;
				}
			}
			return false;
		}

		bool match_name(const Package *p) const ATTRIBUTE_NONNULL_ {
			return match_full(p->category + "/" + p->name);
		}

		inline static void push_result(Get **l, const Masks<m_Type>& r) ATTRIBUTE_NONNULL_ {
			if(*l == NULLPTR) {
				*l = new Get;
			}
			for(m_const_iterator m(r.begin()); likely(m != r.end()); ++m) {
				(*l)->push_back(&*m);
			}
		}

		Get *get_full(const std::string& full) const {
			Get *l(NULLPTR);
			for(const_iterator it(begin()); likely(it != end()); ++it) {
				if(unlikely(it->match_full(full.c_str()))) {
					push_result(&l, *it);
				}
			}
			lookup_const_iterator it(exact_name.find(full));
			if(it != exact_name.end()) {
				push_result(&l, it->second);
			}
			return l;
		}

		Get *get_setname(const std::string& setname) const {
			return get_full(std::string(SET_CATEGORY) + "/" + setname);
		}

		Get *get(const Package *p) const ATTRIBUTE_NONNULL_ {
			return get_full(p->category + "/" + p->name);
		}

		void add(const m_Type& m) {
			std::string full(m.getCategory());
			full.append(1, '/');
			full.append(m.getName());
			if(full.find_first_of("*?[") == std::string::npos) {
				lookup_iterator l(exact_name.find(full));
				if(l != exact_name.end()) {
					l->second.add(m);
					return;
				}
				exact_name.insert(std::pair<std::string, Masks<m_Type> >(full, Masks<m_Type>("", m)));
				return;
			}
			map_const_iterator f(full_index.find(full));
			if(f != full_index.end()) {
				(*this)[f->second].add(m);
				return;
			}
			full_index.insert(std::pair<std::string, size_type>(full, size()));
			push_back(Masks<m_Type>(full, m));
		}

		/* return true if something was added */
		bool add_file(const char *file, Mask::Type mask_type, bool recursive, bool keep_commentlines) ATTRIBUTE_NONNULL_;
		bool add_file(const char *file, Mask::Type mask_type, bool recursive) ATTRIBUTE_NONNULL_ {
			return add_file(file, mask_type, recursive, false);
		}

		/** This can be optionally called after the last add():
		 *  It will release memory. */
		void finalize() {
			full_index.clear();
		}

		void applyListItems(Package *p) const ATTRIBUTE_NONNULL_ {
			Get *masks(get(p));
			if(masks == NULLPTR) {
				return;
			}
			for(typename Get::const_iterator it(masks->begin());
				likely(it != masks->end()); ++it) {
				it->applyItem(p);
			}
			delete masks;
		}

		void applyListSetItems(Version *v, const std::string& set_name) const ATTRIBUTE_NONNULL_ {
			Get *masks(get_setname(set_name));
			if(masks == NULLPTR) {
				return;
			}
			for(typename Get::const_iterator it(masks->begin());
				likely(it != masks->end()); ++it) {
				it->applyItem(v);
			}
			delete masks;
		}

		// return true if some mask potentially applied
		bool applyMasks(Package *p, Keywords::Redundant check) const ATTRIBUTE_NONNULL_;
		bool applyMasks(Package *p) const ATTRIBUTE_NONNULL_ {
			return applyMasks(p, Keywords::RED_NOTHING);
		}

		// return true if some mask matches
		bool MaskMatches(Package *p) const ATTRIBUTE_NONNULL_;

		void applySetMasks(Version *v, const std::string& set_name) const ATTRIBUTE_NONNULL_;
};

// This is only needed for PreList
class PreListEntry {
	public:
		typedef WordVec::size_type FilenameIndex;
		typedef LineVec::size_type LineNumber;
		std::string name;
		WordVec args;
		FilenameIndex filename_index;
		LineNumber linenumber;
		bool locally_double;
};

// This is only needed for PreList
class PreListOrderEntry : public LineVec {
	public:
		typedef PreListEntry::FilenameIndex FilenameIndex;
		typedef PreListEntry::LineNumber LineNumber;
		typedef LineVec super;
		typedef super::const_iterator const_iterator;
		using super::begin;
		using super::end;
		using super::operator[];
		FilenameIndex filename_index;
		LineNumber linenumber;
		bool removed, locally_double;

		PreListOrderEntry(const super& line, FilenameIndex file, LineNumber number)
			: super(line), filename_index(file), linenumber(number), removed(false), locally_double(false) {
		}
};

class PreListFilename {
	private:
		std::string filename, m_repo;
		bool know_repo;

	public:
		PreListFilename(const std::string& n, const char *label);

		const std::string& name() const {
			return filename;
		}

		const char *repo() const ATTRIBUTE_PURE;
};

/* The PreList is needed to Prepare a MaskList:
 *
 * Until we call finalize() or initialize(), one can insert and delete lines.
 * (A line is a std::vector<std::string>). Duplicate lines are recognized, too.
 * However, the original order is preserved.
 * Moreover, after finalize() the entries are collected: For the lines
 *   foo/bar 1
 *   foo/bar 2
 *   =foo/bar-1 3
 *   =foo/bar-1 4
 *
 * the result looks like this
 *   foo/bar    -> 1 2
 *   =foo/bar-1 -> 3 4
 *
 * This corresponds to portage's sorting.
 */
class PreList : public std::vector<PreListEntry> {
	public:
		typedef PreListEntry::FilenameIndex FilenameIndex;
		typedef PreListEntry::LineNumber LineNumber;
		typedef std::vector<PreListEntry> super;
		typedef super::const_iterator const_iterator;
		using super::begin;
		using super::end;
		using super::size;
		using super::empty;
		using super::clear;

	private:
		using super::push_back;

		typedef std::vector<PreListOrderEntry> Order;
		Order order;
		typedef std::vector<PreListFilename> FileNames;
		FileNames filenames;
		typedef std::map<std::vector<std::string>, Order::size_type> Have;
		Have have;
		bool finalized;

	public:
		void clear() {
			finalize();
			filenames.clear();
			super::clear();
		}

		const std::string& file_name(FilenameIndex file) const {
			return filenames[file].name();
		}

		const char *repo(FilenameIndex file) const {
			return filenames[file].repo();
		}

		FilenameIndex push_name(const std::string& filename, const char *reponame) {
			FilenameIndex i(filenames.size());
			filenames.push_back(PreListFilename(filename, reponame));
			return i;
		}

		PreList() : finalized(false) {
		}

		PreList(const std::vector<std::string>& lines, const std::string& filename, const char *reponame, bool only_add) : finalized(false) {
			handle_file(lines, filename, reponame, only_add);
		}

		/// return true if something was changed
		bool handle_file(const std::vector<std::string>& lines, const std::string& filename, const char *reponame, bool only_add, bool keep_commentlines) {
			return handle_lines(lines, push_name(filename, reponame), only_add, NULLPTR, keep_commentlines);
		}

		/// return true if something was changed
		bool handle_file(const std::vector<std::string>& lines, const std::string& filename, const char *reponame, bool only_add) {
			return handle_lines(lines, push_name(filename, reponame), only_add, NULLPTR, false);
		}

		/// return true if something was changed
		bool handle_lines(const std::vector<std::string>& lines, FilenameIndex file, bool only_add, LineNumber *num, bool keep_commentlines);

		/// return true if something was changed
		bool handle_line(const std::string& line, FilenameIndex file, LineNumber number, bool only_add, bool keep_commentlines);

		/// return true if something was changed
		bool add_line(const std::string& line, FilenameIndex file, LineNumber number, bool keep_commentlines);

		/// return true if something was changed
		bool remove_line(const std::string& line);

		/// return true if something was changed
		bool add_splitted(const std::vector<std::string>& line, FilenameIndex file, LineNumber number);

		/// return true if something was changed
		bool remove_splitted(const std::vector<std::string>& line);

		void finalize();

		void initialize(MaskList<Mask> *l, Mask::Type t, bool keep_commentlines) ATTRIBUTE_NONNULL_;
		void initialize(MaskList<Mask> *l, Mask::Type t) ATTRIBUTE_NONNULL_ {
			initialize(l, t, false);
		}

		void initialize(MaskList<KeywordMask> *l, std::string raised_arch) ATTRIBUTE_NONNULL_;

		void initialize(MaskList<PKeywordMask> *l) ATTRIBUTE_NONNULL_;
};

#endif  // SRC_PORTAGE_MASK_LIST_H_
