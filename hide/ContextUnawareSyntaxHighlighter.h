#ifndef HIDE_CONTEXTUNAWARESYNTAXHIGHLIGHTER_H
#define HIDE_CONTEXTUNAWARESYNTAXHIGHLIGHTER_H


#include <map>

#include <hide/Indexer.h>
#include <hide/utils/ListenersHolder.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct WordCategory
	{
		HIDE_ENUM_VALUES(NoneCategory, Unknown, Constant, NamedConstant, Variable, Function, Namespace, Type, Keyword, Macro);
		HIDE_ENUM_CLASS(WordCategory);

		HIDE_DECLARE_SWIG_TO_STRING_WRAPPER();
	};


	struct IContextUnawareSyntaxHighlighterListener
	{
		virtual ~IContextUnawareSyntaxHighlighterListener() { }

		virtual void OnWordCategoryChanged(const std::string& word, std::string category) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IContextUnawareSyntaxHighlighterListener);


	class ContextUnawareSyntaxHighlighter : private ListenersHolder<IContextUnawareSyntaxHighlighterListener>
	{
		HIDE_NONCOPYABLE(ContextUnawareSyntaxHighlighter);

		typedef ListenersHolder<IContextUnawareSyntaxHighlighterListener> ListenersHolderBase;
		typedef std::map<WordCategory, int>						CategoryToCountMap;
		typedef std::map<std::string, CategoryToCountMap>		WordsInfoMap;

		class IndexerListener;
		HIDE_DECLARE_PTR(IndexerListener);

	private:
		static NamedLogger				s_logger;
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

		void AddWord(const std::string& word, WordCategory category);
		void RemoveWord(const std::string& word, WordCategory category);
	};
	HIDE_DECLARE_PTR(ContextUnawareSyntaxHighlighter);

}

#endif
