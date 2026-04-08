#pragma once
#include <string>

namespace XMLLib
{
struct XMLSchema
{
	XMLSchema(const std::string& systemId = "")
		: systemId(systemId)
	{}

	std::string systemId;
	bool isValid() const { return !systemId.empty(); }

	// !!! mutually exclusive, if schemaFilePath is set, rawText is ignored !!!
	// will check schemaFilePath first, if it's not empty, will try to load schema from file, and ignore rawText
	// if the schema fails to load, this is considered an error, and rawText will not be used as a fallback, even if it's set
	std::string schemaFilePath = "";
	std::string rawText = "";
};
}