#include "KWLog.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIGV, Log, All);

#define IGV_LOG_S(LogVerbosity, FormatString, ...) \
	KW_LOG_S(LogIGV, LogVerbosity, FormatString, ##__VA_ARGS__)

#define IGV_LOG(LogVerbosity, FormatString, ...) \
	KW_LOG(LogIGV, LogVerbosity, FormatString, ##__VA_ARGS__)
