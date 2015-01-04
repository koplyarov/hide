#ifndef HIDE_INDEXER_H
#define HIDE_INDEXER_H


#include <hide/IIndexQuery.h>
#include <hide/utils/NamedLogger.h>


namespace hide
{

	class Indexer
	{
	private:
		static NamedLogger			s_logger;

	public:
		Indexer();
		~Indexer();

		IIndexQueryPtr QuerySymbolsBySubstring(const std::string& str);
	};
	HIDE_DECLARE_PTR(Indexer);

}

#endif
