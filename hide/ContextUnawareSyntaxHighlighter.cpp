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

	public:
		IndexerListener(ContextUnawareSyntaxHighlighter* inst) : _inst(inst) { }
		virtual void OnIndexChanged(const Diff<IIndexEntryPtr>& diff) { _inst->OnIndexChanged(diff); }
	};


	HIDE_NAMED_LOGGER(ContextUnawareSyntaxHighlighter)

	ContextUnawareSyntaxHighlighter::ContextUnawareSyntaxHighlighter(const IndexerPtr& indexer)
		: _whitespaceRegex("\\s"), _indexer(indexer), _indexerListener(new IndexerListener(this))
	{
		_indexer->AddListener(_indexerListener);

		Diff<SyntaxWordInfo> diff;

		StringArray builtin_consts = { "nullptr", "NULL" };
		for (const auto& w : builtin_consts)
			AddWord(diff, w, SyntaxWordCategory::Constant);

		StringArray bool_consts = { "true", "false" };
		for (const auto& w : bool_consts)
			AddWord(diff, w, SyntaxWordCategory::Boolean);

		StringArray builtin_types = { "auto", "bool", "char", "char16_t", "char32_t", "double", "float", "int", "long", "short", "signed", "unsigned", "wchar_t" };
		for (const auto& w : builtin_types)
			AddWord(diff, w, SyntaxWordCategory::BuiltinType);

		StringArray std_types = { "any", "auto_ptr", "bad_alloc", "bad_cast", "bad_exception", "bad_typeid", "basic_string", "basic_stringstream", "binary_negate", "binder1st", "binder2nd", "bitset", "const_iterator", "ctime", "deque", "difference_type", "domain_error", "divides", "exception", "greater", "greater_equal", "hash_map", "hash_multimap", "hash_multiset", "hash_set", "ifstream", "invalid_argument", "ios", "ios_base", "istream", "istream_iterator", "istringstream", "iterator", "iterator_category", "less", "less_equal", "list", "logic_error", "logical_and", "logical_not", "logical_or", "map", "multimap", "multiplies", "multiset", "negate", "numeric_limits", "ofstream", "ostream", "ostream_iterator", "ostringstream", "out_of_range", "overflow_error", "pair", "placeholders", "plus", "pointer", "pointer_to_binary_function", "pointer_to_unary_function", "range_error", "reference", "reverse_iterator", "runtime_error", "set", "size_t", "size_type", "std", "wstring", "string", "temporary_buffer", "thread", "time_t", "unary_compose", "unary_negate", "underflow_error", "value_type", "vector" };
		for (const auto& w : std_types)
			AddWord(diff, w, SyntaxWordCategory::Type);

		StringArray std_functions = { "_1", "_2", "_3", "_4", "_5", "abort", "abs", "accumulate", "acos", "adjacent_difference", "adjacent_find", "adjacent_find_if", "append", "asctime", "asin", "assert", "assign", "at", "atan", "atan2", "atexit", "atof", "atoi", "atol", "back", "back_inserter", "bad", "badbit", "beg", "begin", "binary_compose", "binary_search", "bind", "bind2nd", "bsearch", "c_str", "calloc", "capacity", "ceil", "cerr", "cin", "clear", "clearerr", "clock", "clog", "close", "compare", "compose1", "compose2", "construct", "copy", "copy_backward", "copy_n", "cos", "cosh", "count", "count_if", "cout", "data", "destroy", "difftime", "div", "empty", "end", "endl", "eof", "eofbit", "equal", "equal_range", "erase", "exit", "exp", "fabs", "fail", "failbit", "failure", "fclose", "feof", "ferror", "fflush", "fgetc", "fgetpos", "fgets", "fill", "fill_n", "find", "find_end", "find_first_not_of", "find_first_of", "find_if", "find_last_not_of", "find_last_of", "first", "flags", "flip", "floor", "flush", "fmod", "fopen", "for_each", "fprintf", "fputc", "fputs", "fread", "free", "freopen", "frexp", "front", "fscanf", "fseek", "fsetpos", "fstream", "ftell", "fwrite", "gcount", "generate", "generate_n", "get", "get_temporary_buffer", "getc", "getchar", "getenv", "getline", "gets", "gmtime", "good", "goodbit", "ignore", "in", "includes", "inner_product", "inplace_merge", "insert", "inserter", "iostate", "iota", "is_heap", "is_open", "is_sorted", "isalnum", "isalpha", "iscntrl", "isdigit", "isgraph", "islower", "isprint", "ispunct", "isspace", "isupper", "isxdigit", "iter_swap", "key_comp", "ldiv", "length", "length_error", "lexicographical_compare", "lexicographical_compare_3way", "localtime", "log", "log10", "longjmp", "lower_bound", "make_heap", "make_pair", "make_shared", "malloc", "max", "max_element", "max_size", "mem_fun", "mem_fun1", "mem_fun1_ref", "mem_fun_ref", "memchr", "memcpy", "memmove", "memset", "merge", "min", "min_element", "minus", "mismatch", "mktime", "modf", "modulus", "next_permutation", "npos", "nth_element", "open", "partial_sort", "partial_sort_copy", "partial_sum", "partition", "peek", "perror", "pop", "pop_back", "pop_front", "pop_heap", "pow", "power", "precision", "prev_permutation", "printf", "ptr_fun", "push", "push_back", "push_front", "push_heap", "put", "putback", "putc", "putchar", "puts", "qsort", "raise", "rand", "random_sample", "random_sample_n", "random_shuffle", "rbegin", "rdbuf", "rdstate", "read", "realloc", "remove", "remove_copy", "remove_copy_if", "remove_if", "rename", "rend", "replace", "replace_copy", "replace_copy_if", "replace_if", "reserve", "reset", "resize", "return_temporary_buffer", "reverse", "reverse_copy", "rewind", "rfind", "rotate", "rotate_copy", "scanf", "search", "search_n", "second", "seekg", "seekp", "set_difference", "set_intersection", "set_symmetric_difference", "set_union", "setbuf", "setf", "setjmp", "setlocale", "setvbuf", "signal", "sin", "sinh", "size", "sort", "sort_heap", "splice", "sprintf", "sqrt", "srand", "sscanf", "stable_partition", "stable_sort", "str", "strcat", "strchr", "strcmp", "strcoll", "strcpy", "strcspn", "strerror", "strftime", "strlen", "strncat", "strncmp", "strncpy", "strpbrk", "strrchr", "strspn", "strstr", "strtod", "strtok", "strtol", "strtoul", "strxfrm", "substr", "swap", "swap_ranges", "sync_with_stdio", "system", "tan", "tanh", "tellg", "tellp", "test", "time", "tmpfile", "tmpnam", "to_string", "to_ulong", "tolower", "top", "toupper", "transform", "unget", "ungetc", "uninitialized_copy", "uninitialized_copy_n", "uninitialized_fill", "uninitialized_fill_n", "unique", "unique_copy", "unsetf", "upper_bound", "va_arg", "value_comp", "vfprintf", "vprintf", "vsprintf", "width", "write" };
		for (const auto& w : std_functions)
			AddWord(diff, w, SyntaxWordCategory::Function);

		StringArray statement = { "break", "continue", "goto", "return" };
		for (const auto& w : statement)
			AddWord(diff, w, SyntaxWordCategory::Statement);

		StringArray conditional = { "else", "if", "switch" };
		for (const auto& w : conditional)
			AddWord(diff, w, SyntaxWordCategory::Conditional);

		StringArray repeat = { "do", "for", "while" };
		for (const auto& w : repeat)
			AddWord(diff, w, SyntaxWordCategory::Repeat);

		StringArray label = { "case", "default" };
		for (const auto& w : label)
			AddWord(diff, w, SyntaxWordCategory::Label);

		StringArray operator_ = { "alignas", "alignof", "and", "and_eq", "bitand", "bitor", "compl", "const_cast", "decltype", "delete", "dynamic_cast", "new", "not", "not_eq", "or", "or_eq", "reinterpret_cast", "sizeof", "static_assert", "static_cast", "typeid", "xor", "xor_eq" };
		for (const auto& w : operator_)
			AddWord(diff, w, SyntaxWordCategory::Operator);

		StringArray exception = { "catch", "throw", "try" };
		for (const auto& w : exception)
			AddWord(diff, w, SyntaxWordCategory::Exception);

		StringArray keywords = { "asm", "class", "const", "constexpr", "enum", "explicit", "export", "extern", "friend", "inline", "mutable", "namespace", "noexcept", "operator", "private", "protected", "public", "register", "static", "struct", "template", "this", "thread_local", "typedef", "typename", "union", "using", "virtual", "void", "volatile" };
		for (const auto& w : keywords)
			AddWord(diff, w, SyntaxWordCategory::Keyword);

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
		Diff<SyntaxWordInfo> diff;
		for (const auto& wi : _wordsInfo)
		{
			if (wi.second.empty() || wi.second.rbegin()->second <= 0)
				continue;

			diff.GetAdded().push_back(SyntaxWordInfo(wi.first, wi.second.rbegin()->first.ToString()));
		}
		listener->OnWordsChanged(diff);
		s_logger.Info() << "PopulateState: " << profiler.Reset();
	}


	void ContextUnawareSyntaxHighlighter::AddWord(Diff<SyntaxWordInfo>& diff, const std::string& word, SyntaxWordCategory category)
	{
		auto wi_it = _wordsInfo.find(word);
		if (wi_it == _wordsInfo.end())
			wi_it = _wordsInfo.insert(std::make_pair(word, CategoryToCountMap())).first;

		auto cat_it = wi_it->second.find(category);
		if (cat_it == wi_it->second.end())
		{
			cat_it = wi_it->second.insert(std::make_pair(category, 1)).first;
			if (std::next(cat_it) == wi_it->second.end()) // This was the highest priority
				diff.GetAdded().push_back(SyntaxWordInfo(word, cat_it->first.ToString()));
		}
		else
			++cat_it->second;
	}


	void ContextUnawareSyntaxHighlighter::RemoveWord(Diff<SyntaxWordInfo>& diff, const std::string& word, SyntaxWordCategory category)
	{
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
				diff.GetRemoved().push_back(SyntaxWordInfo(word, cat_it->first.ToString()));
				if (wi_it->second.size() > 1)
					diff.GetAdded().push_back(SyntaxWordInfo(word, std::prev(cat_it)->first.ToString()));
			}
			wi_it->second.erase(cat_it);
		}

		if (wi_it->second.empty())
			_wordsInfo.erase(wi_it);
	}


	void ContextUnawareSyntaxHighlighter::OnIndexChanged(const Diff<IIndexEntryPtr>& diff)
	{
		HIDE_LOCK(GetMutex());

		Diff<SyntaxWordInfo> words_diff;

		for (auto&& entry : diff.GetRemoved())
			if (EntryIsAWord(entry))
				RemoveWord(words_diff, entry->GetName(), GetCategory(entry));

		for (auto&& entry : diff.GetAdded())
			if (EntryIsAWord(entry))
				AddWord(words_diff, entry->GetName(), GetCategory(entry));

		InvokeListeners(std::bind(&IContextUnawareSyntaxHighlighterListener::OnWordsChanged, std::placeholders::_1, words_diff));
	}


	bool ContextUnawareSyntaxHighlighter::EntryIsAWord(const IIndexEntryPtr& entry)
	{
		boost::smatch m;
		return !boost::regex_search(entry->GetName(), m, _whitespaceRegex, boost::match_partial);
	}


	SyntaxWordCategory ContextUnawareSyntaxHighlighter::GetCategory(const IIndexEntryPtr& entry)
	{
		switch (entry->GetKind().GetRaw())
		{
		case IndexEntryKind::Unknown:			return SyntaxWordCategory::Unknown;
		case IndexEntryKind::NamedConstant:		return SyntaxWordCategory::NamedConstant;
		case IndexEntryKind::Variable:			return SyntaxWordCategory::Variable;
		case IndexEntryKind::Function:			return SyntaxWordCategory::Function;
		case IndexEntryKind::Type:				return SyntaxWordCategory::Type;
		case IndexEntryKind::Namespace:			return SyntaxWordCategory::Namespace;
		case IndexEntryKind::Macro:				return SyntaxWordCategory::Macro;
		default:								return SyntaxWordCategory::Unknown;
		}
	}

}
