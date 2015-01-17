#include <hide/ContextUnawareSyntaxHighlighter.h>

#include <iterator>

#include <hide/utils/Profiler.h>


namespace hide
{

	class ContextUnawareSyntaxHighlighter::IndexerListener : public IIndexerListener
	{
	private:
		ContextUnawareSyntaxHighlighter*	_inst;

	public:
		IndexerListener(ContextUnawareSyntaxHighlighter* inst)
			: _inst(inst)
		{ }

		virtual void OnEntryAdded(const IIndexEntryPtr& entry)
		{ _inst->AddWord(entry->GetName(), GetCategory(entry)); }

		virtual void OnEntryRemoved(const IIndexEntryPtr& entry)
		{ _inst->RemoveWord(entry->GetName(), GetCategory(entry)); }

	private:
		static WordCategory GetCategory(const IIndexEntryPtr& entry)
		{
			switch (entry->GetKind().GetRaw())
			{
			case IndexEntryKind::NamedConstant:		return WordCategory::NamedConstant;
			case IndexEntryKind::Variable:			return WordCategory::Variable;
			case IndexEntryKind::Function:			return WordCategory::Function;
			case IndexEntryKind::Type:				return WordCategory::Type;
			default:								return WordCategory::NoneCategory;
			}
		}
	};


	HIDE_NAMED_LOGGER(ContextUnawareSyntaxHighlighter)

	ContextUnawareSyntaxHighlighter::ContextUnawareSyntaxHighlighter(const IndexerPtr& indexer)
		: _indexer(indexer), _indexerListener(new IndexerListener(this))
	{
		_indexer->AddListener(_indexerListener);
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
