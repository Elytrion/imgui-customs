#pragma once
#include <string>
#include "XMLDocumentHandle.h"
#include "XMLSchema.hpp"

namespace XMLLib
{

struct XMLParseConfig
{
    bool doNamespaces = false;
    bool validationConstraintFatal = true;
    bool validationSchemaFullChecking = false;
    bool createEntityReferenceNodes = false;
};

struct XMLParser
{
    // Parses a given xml file, and validates it against a provided schema using the config settings
    // If an empty or invalid schema is provided, no schema validation will be done
    // Well-formedness checks are done no matter if a schema is provided or not
    static XMLDocumentHandle Parse(const std::string& xmlFilePath, const XMLSchema& schema, const XMLParseConfig& config = {});
    static XMLDocumentHandle Parse(const std::string& xmlFilePath, const XMLParseConfig& config = {})
    {
		return Parse(xmlFilePath, XMLSchema(), config);
    }
};

}
