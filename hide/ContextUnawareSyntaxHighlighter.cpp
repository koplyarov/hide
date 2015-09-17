#include <hide/ContextUnawareSyntaxHighlighter.h>

#include <iterator>

#include <boost/regex.hpp>

#include <hide/utils/Profiler.h>


namespace hide
{

	class ContextUnawareSyntaxHighlighter::IndexerListener : public IIndexerListener
	{
	private:
		ContextUnawareSyntaxHighlighter*	_inst;
		boost::regex						_whitespaceRegex;

	public:
		IndexerListener(ContextUnawareSyntaxHighlighter* inst)
			: _inst(inst), _whitespaceRegex("\\s")
		{ }

		virtual void OnIndexChanged(const Diff<IIndexEntryPtr>& diff)
		{
			for (auto&& entry : diff.GetRemoved())
				if (EntryIsAWord(entry))
					_inst->RemoveWord(entry->GetName(), GetCategory(entry));
			for (auto&& entry : diff.GetAdded())
				if (EntryIsAWord(entry))
					_inst->AddWord(entry->GetName(), GetCategory(entry));
		}

	private:
		bool EntryIsAWord(const IIndexEntryPtr& entry)
		{
			boost::smatch m;
			return !boost::regex_search(entry->GetName(), m, _whitespaceRegex, boost::match_partial);
		}

		static WordCategory GetCategory(const IIndexEntryPtr& entry)
		{
			switch (entry->GetKind().GetRaw())
			{
			case IndexEntryKind::Unknown:			return WordCategory::Unknown;
			case IndexEntryKind::NamedConstant:		return WordCategory::NamedConstant;
			case IndexEntryKind::Variable:			return WordCategory::Variable;
			case IndexEntryKind::Function:			return WordCategory::Function;
			case IndexEntryKind::Type:				return WordCategory::Type;
			case IndexEntryKind::Namespace:			return WordCategory::Namespace;
			case IndexEntryKind::Macro:				return WordCategory::Macro;
			default:								return WordCategory::NoneCategory;
			}
		}
	};


	HIDE_NAMED_LOGGER(ContextUnawareSyntaxHighlighter)

	ContextUnawareSyntaxHighlighter::ContextUnawareSyntaxHighlighter(const IndexerPtr& indexer)
		: _indexer(indexer), _indexerListener(new IndexerListener(this))
	{
		_indexer->AddListener(_indexerListener);

		StringArray builtin_consts = { "true", "false", "nullptr", "NULL" };
		for (const auto& w : builtin_consts)
			AddWord(w, WordCategory::Constant);

		StringArray builtin_types = { "auto", "bool", "char", "char16_t", "char32_t", "double", "float", "int", "long", "short", "signed", "unsigned", "wchar_t" };
		builtin_types.insert(builtin_types.end(), { "any", "auto_ptr", "bad_alloc", "bad_cast", "bad_exception", "bad_typeid", "basic_string", "basic_stringstream", "binary_negate", "binder1st", "binder2nd", "bitset", "const_iterator", "ctime", "deque", "difference_type", "domain_error", "divides", "exception", "greater", "greater_equal", "hash_map", "hash_multimap", "hash_multiset", "hash_set", "ifstream", "invalid_argument", "ios", "ios_base", "istream", "istream_iterator", "istringstream", "iterator", "iterator_category", "less", "less_equal", "list", "logic_error", "logical_and", "logical_not", "logical_or", "map", "multimap", "multiplies", "multiset", "negate", "numeric_limits", "ofstream", "ostream", "ostream_iterator", "ostringstream", "out_of_range", "overflow_error", "pair", "placeholders", "plus", "pointer", "pointer_to_binary_function", "pointer_to_unary_function", "range_error", "reference", "reverse_iterator", "runtime_error", "set", "size_t", "size_type", "std", "wstring", "string", "temporary_buffer", "thread", "time_t", "unary_compose", "unary_negate", "underflow_error", "value_type", "vector" });
		for (const auto& w : builtin_types)
			AddWord(w, WordCategory::Type);

		StringArray builtin_functions = { "_1", "_2", "_3", "_4", "_5", "abort", "abs", "accumulate", "acos", "adjacent_difference", "adjacent_find", "adjacent_find_if", "append", "asctime", "asin", "assert", "assign", "at", "atan", "atan2", "atexit", "atof", "atoi", "atol", "back", "back_inserter", "bad", "badbit", "beg", "begin", "binary_compose", "binary_search", "bind", "bind2nd", "bsearch", "c_str", "calloc", "capacity", "ceil", "cerr", "cin", "clear", "clearerr", "clock", "clog", "close", "compare", "compose1", "compose2", "construct", "copy", "copy_backward", "copy_n", "cos", "cosh", "count", "count_if", "cout", "data", "destroy", "difftime", "div", "empty", "end", "endl", "eof", "eofbit", "equal", "equal_range", "erase", "exit", "exp", "fabs", "fail", "failbit", "failure", "fclose", "feof", "ferror", "fflush", "fgetc", "fgetpos", "fgets", "fill", "fill_n", "find", "find_end", "find_first_not_of", "find_first_of", "find_if", "find_last_not_of", "find_last_of", "first", "flags", "flip", "floor", "flush", "fmod", "fopen", "for_each", "fprintf", "fputc", "fputs", "fread", "free", "freopen", "frexp", "front", "fscanf", "fseek", "fsetpos", "fstream", "ftell", "fwrite", "gcount", "generate", "generate_n", "get", "get_temporary_buffer", "getc", "getchar", "getenv", "getline", "gets", "gmtime", "good", "goodbit", "ignore", "in", "includes", "inner_product", "inplace_merge", "insert", "inserter", "iostate", "iota", "is_heap", "is_open", "is_sorted", "isalnum", "isalpha", "iscntrl", "isdigit", "isgraph", "islower", "isprint", "ispunct", "isspace", "isupper", "isxdigit", "iter_swap", "key_comp", "ldiv", "length", "length_error", "lexicographical_compare", "lexicographical_compare_3way", "localtime", "log", "log10", "longjmp", "lower_bound", "make_heap", "make_pair", "make_shared", "malloc", "max", "max_element", "max_size", "mem_fun", "mem_fun1", "mem_fun1_ref", "mem_fun_ref", "memchr", "memcpy", "memmove", "memset", "merge", "min", "min_element", "minus", "mismatch", "mktime", "modf", "modulus", "next_permutation", "npos", "nth_element", "open", "partial_sort", "partial_sort_copy", "partial_sum", "partition", "peek", "perror", "pop", "pop_back", "pop_front", "pop_heap", "pow", "power", "precision", "prev_permutation", "printf", "ptr_fun", "push", "push_back", "push_front", "push_heap", "put", "putback", "putc", "putchar", "puts", "qsort", "raise", "rand", "random_sample", "random_sample_n", "random_shuffle", "rbegin", "rdbuf", "rdstate", "read", "realloc", "remove", "remove_copy", "remove_copy_if", "remove_if", "rename", "rend", "replace", "replace_copy", "replace_copy_if", "replace_if", "reserve", "reset", "resize", "return_temporary_buffer", "reverse", "reverse_copy", "rewind", "rfind", "rotate", "rotate_copy", "scanf", "search", "search_n", "second", "seekg", "seekp", "set_difference", "set_intersection", "set_symmetric_difference", "set_union", "setbuf", "setf", "setjmp", "setlocale", "setvbuf", "signal", "sin", "sinh", "size", "sort", "sort_heap", "splice", "sprintf", "sqrt", "srand", "sscanf", "stable_partition", "stable_sort", "str", "strcat", "strchr", "strcmp", "strcoll", "strcpy", "strcspn", "strerror", "strftime", "strlen", "strncat", "strncmp", "strncpy", "strpbrk", "strrchr", "strspn", "strstr", "strtod", "strtok", "strtol", "strtoul", "strxfrm", "substr", "swap", "swap_ranges", "sync_with_stdio", "system", "tan", "tanh", "tellg", "tellp", "test", "time", "tmpfile", "tmpnam", "to_string", "to_ulong", "tolower", "top", "toupper", "transform", "unget", "ungetc", "uninitialized_copy", "uninitialized_copy_n", "uninitialized_fill", "uninitialized_fill_n", "unique", "unique_copy", "unsetf", "upper_bound", "va_arg", "value_comp", "vfprintf", "vprintf", "vsprintf", "width", "write" };
		for (const auto& w : builtin_functions)
			AddWord(w, WordCategory::Function);

		StringArray keywords = { "alignas", "alignof", "and", "and_eq", "asm", "bitand", "bitor", "break", "case", "catch", "class", "compl", "const", "constexpr", "const_cast", "continue", "decltype", "default", "delete", "do", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "for", "friend", "goto", "if", "inline", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "operator", "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast", "return", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "template", "this", "thread_local", "throw", "try", "typedef", "typeid", "typename", "union", "using", "virtual", "void", "volatile", "while", "xor", "xor_eq" };
		for (const auto& w : keywords)
			AddWord(w, WordCategory::Keyword);

		s_logger.Info() << "Created";
	}


	ContextUnawareSyntaxHighlighter::~ContextUnawareSyntaxHighlighter()
	{
		s_logger.Info() << "Destroying";
		_indexer->RemoveListener(_indexerListener);
	}


	void ContextUnawareSyntaxHighlighter::PopulateState(const IContextUnawareSyntaxHighlighterListenerPtr& listener) const
	{
		Profiler<> profiler;
		for (const auto& wi : _wordsInfo)
		{
			if (wi.second.empty() || wi.second.rbegin()->second <= 0)
				continue;

			listener->OnWordCategoryChanged(wi.first, wi.second.rbegin()->first.ToString());
		}
		s_logger.Info() << "PopulateState: " << profiler.Reset();
	}


	void ContextUnawareSyntaxHighlighter::AddWord(const std::string& word, WordCategory category)
	{
		HIDE_LOCK(GetMutex());

		auto wi_it = _wordsInfo.find(word);
		if (wi_it == _wordsInfo.end())
			wi_it = _wordsInfo.insert(std::make_pair(word, CategoryToCountMap())).first;

		auto cat_it = wi_it->second.find(category);
		if (cat_it == wi_it->second.end())
		{
			cat_it = wi_it->second.insert(std::make_pair(category, 1)).first;
			if (std::next(cat_it) == wi_it->second.end()) // This was the highest priority
				InvokeListeners(std::bind(&IContextUnawareSyntaxHighlighterListener::OnWordCategoryChanged, std::placeholders::_1, word, category.ToString()));
		}
		else
			++cat_it->second;
	}


	void ContextUnawareSyntaxHighlighter::RemoveWord(const std::string& word, WordCategory category)
	{
		HIDE_LOCK(GetMutex());

		auto wi_it = _wordsInfo.find(word);
		if (wi_it == _wordsInfo.end())
		{
			s_logger.Error() << "Cannot find an entry for '" << word << "' word!";
			return;
		}

		auto cat_it = wi_it->second.find(category);
		if (cat_it == wi_it->second.end())
		{
			s_logger.Error() << "Cannot find an entry for '" << word << "':" << category << " category!";
			return;
		}

		if (--cat_it->second <= 0)
		{
			if (std::next(cat_it) == wi_it->second.end()) // This was the highest priority
			{
				WordCategory category = (wi_it->second.size() == 1) ? WordCategory::NoneCategory : std::prev(cat_it)->first.GetRaw();
				InvokeListeners(std::bind(&IContextUnawareSyntaxHighlighterListener::OnWordCategoryChanged, std::placeholders::_1, word, category.ToString()));
			}
			wi_it->second.erase(cat_it);
		}

		if (wi_it->second.empty())
			_wordsInfo.erase(wi_it);
	}

}
