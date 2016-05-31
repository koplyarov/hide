#ifndef HIDE_CONTEXTUNAWARESYNTAXHIGHLIGHTER_H
#define HIDE_CONTEXTUNAWARESYNTAXHIGHLIGHTER_H


#include <map>

#include <boost/regex.hpp>

#include <hide/Indexer.h>
#include <hide/ProjectFiles.h>
#include <hide/utils/ListenersHolder.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct SyntaxWordCategory
	{
		HIDE_ENUM_VALUES(
				Unknown,
				Constant,
				Boolean,
				NamedConstant,
				Variable,
				Function,
				Namespace,
				BuiltinType,
				Type,
				Statement,
				Conditional,
				Repeat,
				Label,
				Operator,
				Exception,
				Keyword,
				Import,
				Macro
			);
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

		virtual void OnWordsChanged(const std::string& filename, const Diff<SyntaxWordInfo>& diff) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IContextUnawareSyntaxHighlighterListener);


	struct IContextUnawareSyntaxHighlighterFileListener
	{
		virtual ~IContextUnawareSyntaxHighlighterFileListener() { }

		virtual void OnVisibleFilesChanged(const Diff<std::string>& diff) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IContextUnawareSyntaxHighlighterFileListener);


	class ContextUnawareSyntaxHighlighterFile : private ListenersHolder<IContextUnawareSyntaxHighlighterFileListener>
	{
		typedef ListenersHolder<IContextUnawareSyntaxHighlighterFileListener> ListenersHolderBase;

		class ProjectFilesListener : public IProjectFilesListener
		{
		private:
			ContextUnawareSyntaxHighlighterFile*	_inst;

		public:
			ProjectFilesListener(ContextUnawareSyntaxHighlighterFile* inst);

			virtual void OnFileAdded(const IFilePtr& file);
			virtual void OnFileRemoved(const IFilePtr& file);
			virtual void OnFileModified(const IFilePtr& file);
		};

	private:
		ProjectFilesPtr				_files;
		std::set<std::string>		_filenames;
		IProjectFilesListenerPtr	_listener;

	public:
		ContextUnawareSyntaxHighlighterFile(const ProjectFilesPtr& files);
		~ContextUnawareSyntaxHighlighterFile();

		virtual void AddListener(const IContextUnawareSyntaxHighlighterFileListenerPtr& listener)		{ ListenersHolderBase::AddListener(listener); }
		virtual void RemoveListener(const IContextUnawareSyntaxHighlighterFileListenerPtr& listener)	{ ListenersHolderBase::RemoveListener(listener); }

	protected:
		virtual void PopulateState(const IContextUnawareSyntaxHighlighterFileListenerPtr& listener) const;
	};
	HIDE_DECLARE_PTR(ContextUnawareSyntaxHighlighterFile);


	class ContextUnawareSyntaxHighlighter : private ListenersHolder<IContextUnawareSyntaxHighlighterListener>
	{
		HIDE_NONCOPYABLE(ContextUnawareSyntaxHighlighter);

		typedef ListenersHolder<IContextUnawareSyntaxHighlighterListener> ListenersHolderBase;
		typedef std::map<SyntaxWordCategory, int>						CategoryToCountMap;
		typedef std::map<std::string, CategoryToCountMap>		WordsInfoMap;

		class IndexerListener;
		HIDE_DECLARE_PTR(IndexerListener);

		class FileData
		{
		private:
			WordsInfoMap					_wordsInfo;

		public:
			const WordsInfoMap& GetWordsInfo() const { return _wordsInfo; }

			void AddWord(Diff<SyntaxWordInfo>& diff, const std::string& word, SyntaxWordCategory category);
			void RemoveWord(Diff<SyntaxWordInfo>& diff, const std::string& word, SyntaxWordCategory category);
		};
		HIDE_DECLARE_PTR(FileData);

		typedef std::map<std::string, FileDataPtr>	FileDataMap;

	private:
		static NamedLogger				s_logger;
		boost::regex					_whitespaceRegex;
		FileDataMap						_fileData;
		WordsInfoMap					_wordsInfo;
		ProjectFilesPtr					_files;
		IndexerPtr						_indexer;
		IIndexerListenerPtr				_indexerListener;

	public:
		ContextUnawareSyntaxHighlighter(const ProjectFilesPtr& files, const IndexerPtr& indexer);
		~ContextUnawareSyntaxHighlighter();

		ContextUnawareSyntaxHighlighterFilePtr GetFileContext(const std::string& filename);

		virtual void AddListener(const IContextUnawareSyntaxHighlighterListenerPtr& listener)		{ ListenersHolderBase::AddListener(listener); }
		virtual void RemoveListener(const IContextUnawareSyntaxHighlighterListenerPtr& listener)	{ ListenersHolderBase::RemoveListener(listener); }

	protected:
		virtual void PopulateState(const IContextUnawareSyntaxHighlighterListenerPtr& listener) const;

		void OnIndexChanged(const Diff<IIndexEntryPtr>& diff);

		bool EntryIsAWord(const IIndexEntryPtr& entry);
		static SyntaxWordCategory GetCategory(const IIndexEntryPtr& entry);

	private:
		FileDataPtr GetFileData(const std::string& filename);
	};
	HIDE_DECLARE_PTR(ContextUnawareSyntaxHighlighter);

}

#endif
