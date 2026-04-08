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
    static XMLDocumentHandle Parse(const std::string& xmlFilePath, const XMLSchema& schema, const XMLParseConfig& config = {});
    static XMLDocumentHandle Parse(const std::string& xmlFilePath, const XMLParseConfig& config = {})
    {
		return Parse(xmlFilePath, XMLSchema(), config);
    }
};

}
