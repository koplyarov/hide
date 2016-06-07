#ifndef HIDE_UTILS_OPERATIONPROFILER_H
#define HIDE_UTILS_OPERATIONPROFILER_H


#include <hide/utils/NamedLogger.h>
#include <hide/utils/Profiler.h>
#include <hide/utils/rethread.h>


namespace hide
{

	class OperationProfiler
    {
    private:
        NamedLogger						_logger;
    	Profiler<>			    		_p;
        std::string						_op;
        const cancellation_token*		_cancellationToken;

    public:
    	OperationProfiler(NamedLogger& logger, std::string op)
        	: _logger(logger), _op(std::move(op)), _cancellationToken(nullptr)
        { _p.Reset(); }

    	OperationProfiler(NamedLogger& logger, std::string op, const cancellation_token& cancellationToken)
        	: _logger(logger), _op(std::move(op)), _cancellationToken(&cancellationToken)
        { _p.Reset(); }

        ~OperationProfiler()
        { _logger.Info() << _op << ": " << _p.Reset() << (!_cancellationToken || *_cancellationToken ? "" : ", cancelled"); }
    };

}

#endif
