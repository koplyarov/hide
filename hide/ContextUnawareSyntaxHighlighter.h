#ifndef HIDE_CONTEXTUNAWARESYNTAXHIGHLIGHTER_H
#define HIDE_CONTEXTUNAWARESYNTAXHIGHLIGHTER_H


#include <map>

#include <boost/regex.hpp>

#include <hide/Indexer.h>
#include <hide/utils/ListenersHolder.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct SyntaxWordCategory
	{
		HIDE_ENUM_VALUES(NoneCategory, Unknown, Constant, NamedConstant, Variable, Function, Namespace, Type, Keyword, Macro);
		HIDE_ENUM_CLASS(SyntaxWordCategory);

		HIDE_DECLARE_SWIG_TO_STRING_WRAPPER();
	};


	class SyntaxWordInfo
	{
	private:
		std::string	_word;
		std::string	_category;

	public:
		SyntaxWordInfo()
		{ }

		SyntaxWordInfo(const std::string& word, const std::string& category)
			: _word(word), _category(category)
		{ }

		std::string GetWord() const			{ return _word; }
		std::string GetCategory() const	{ return _category; }
	};


	struct IContextUnawareSyntaxHighlighterListener
	{
		virtual ~IContextUnawareSyntaxHighlighterListener() { }

		virtual void OnWordsChanged(const Diff<SyntaxWordInfo>& diff) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IContextUnawareSyntaxHighlighterListener);


	class ContextUnawareSyntaxHighlighter : private ListenersHolder<IContextUnawareSyntaxHighlighterListener>
	{
		HIDE_NONCOPYABLE(ContextUnawareSyntaxHighlighter);

		typedef ListenersHolder<IContextUnawareSyntaxHighlighterListener> ListenersHolderBase;
		typedef std::map<SyntaxWordCategory, int>						CategoryToCountMap;
		typedef std::map<std::string, CategoryToCountMap>		WordsInfoMap;

		class IndexerListener;
		HIDE_DECLARE_PTR(IndexerListener);

	private:
		static NamedLogger				s_logger;
		boost::regex					_whitespaceRegex;
		WordsInfoMap					_wordsInfo;
		IndexerPtr						_indexer;
		IIndexerListenerPtr				_indexerListener;

	public:
		ContextUnawareSyntaxHighlighter(const IndexerPtr& indexer);
		~ContextUnawareSyntaxHighlighter();

		virtual void AddListener(const IContextUnawareSyntaxHighlighterListenerPtr& listener)		{ ListenersHolderBase::AddListener(listener); }
		virtual void RemoveListener(const IContextUnawareSyntaxHighlighterListenerPtr& listener)	{ ListenersHolderBase::RemoveListener(listener); }

	protected:
		virtual void PopulateState(const IContextUnawareSyntaxHighlighterListenerPtr& listener) const;

		void AddWord(Diff<SyntaxWordInfo>& diff, const std::string& word, SyntaxWordCategory category);
		void RemoveWord(Diff<SyntaxWordInfo>& diff, const std::string& word, SyntaxWordCategory category);

		void OnIndexChanged(const Diff<IIndexEntryPtr>& diff);

		bool EntryIsAWord(const IIndexEntryPtr& entry);
		static SyntaxWordCategory GetCategory(const IIndexEntryPtr& entry);
	};
	HIDE_DECLARE_PTR(ContextUnawareSyntaxHighlighter);

}

#endif
