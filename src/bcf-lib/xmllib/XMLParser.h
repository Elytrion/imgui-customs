#pragma once
#include <string>
#include "XMLDocumentHandle.h"
#include "XMLSchema.hpp"

namespace XMLLib
{

struct XMLParseConfig
{
	bool doNamespaces = false;                  // whether to perform namespace processing during parsing
	bool validationConstraintFatal = true;      // whether to treat validation constraint violations as fatal errors that cause parsing to fail, or to allow parsing to succeed but report the violations in the XMLDocumentHandle's error state
    bool validationSchemaFullChecking = false;  // whether to perform full schema validation checks, if a schema is provided
    bool createEntityReferenceNodes = false;    // whether to create entity reference nodes during parsing
};

struct XMLParser
{
    // Parses a given xml file from filepath, and validates it against a provided schema using the config settings
    // If an empty or invalid schema is provided, no schema validation will be done
    // Well-formedness checks are done no matter if a schema is provided or not
    static XMLDocumentHandle Parse(const std::string& xmlFilePath, const XMLSchema& schema, const XMLParseConfig& config = {});
    static XMLDocumentHandle Parse(const std::string& xmlFilePath, const XMLParseConfig& config = {})
    {
		return Parse(xmlFilePath, XMLSchema(), config);
    }

    // Parses raw xml text and validates it against a provided schema using the config settings
    // If an empty or invalid schema is provided, no schema validation will be done
    // Well-formedness checks are done no matter if a schema is provided or not
    static XMLDocumentHandle ParseMemory(const std::string& xmlFileName, const std::string& xmlRawText, const XMLSchema& schema, const XMLParseConfig& config = {});
    static XMLDocumentHandle ParseMemory(const std::string& xmlFileName, const std::string& xmlRawText, const XMLParseConfig& config = {})
    {
        return ParseMemory(xmlFileName, xmlRawText, XMLSchema(), config);
    }
};

}
