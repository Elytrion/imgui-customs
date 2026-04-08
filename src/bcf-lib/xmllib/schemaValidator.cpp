#include "schemaValidator.h"

#include <cstring>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLException.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include <xercesc/framework/MemBufInputSource.hpp>

#include <xercesc/dom/DOM.hpp>

using namespace XMLLib;
using namespace xercesc;

namespace
{
    std::string XmlChToString(const XMLCh* text)
    {
        if (!text)
            return {};

        char* temp = XMLString::transcode(text);
        if (!temp)
            return {};

        std::string out(temp);
        XMLString::release(&temp);
        return out;
    }

    struct SchemaEntry
    {
        std::string systemId;
        std::string namespaceUri;
        std::string schemaText;
    };

    class InMemorySchemaResolver : public EntityResolver
    {
    public:
        explicit InMemorySchemaResolver(const std::unordered_map<std::string, SchemaEntry>& schemas)
            : m_Schemas(schemas)
        {
        }

        InputSource* resolveEntity(const XMLCh* const publicId,
            const XMLCh* const systemId) override
        {
            (void)publicId;

            const std::string requestedSystemId = XmlChToString(systemId);

            auto it = m_Schemas.find(requestedSystemId);
            if (it == m_Schemas.end())
            {
                // Fallback: sometimes parsers pass a full/relative path-like system id.
                // Match by suffix against known system ids.
                for (const auto& pair : m_Schemas)
                {
                    const std::string& knownId = pair.first;
                    if (requestedSystemId.size() >= knownId.size() &&
                        requestedSystemId.compare(
                            requestedSystemId.size() - knownId.size(),
                            knownId.size(),
                            knownId) == 0)
                    {
                        it = m_Schemas.find(knownId);
                        break;
                    }
                }
            }

            if (it == m_Schemas.end())
                return nullptr;

            const SchemaEntry& schema = it->second;

            return new MemBufInputSource(
                reinterpret_cast<const XMLByte*>(schema.schemaText.data()),
                static_cast<XMLSize_t>(schema.schemaText.size()),
                schema.systemId.c_str(),
                false // don't let Xerces free std::string memory
            );
        }

    private:
        const std::unordered_map<std::string, SchemaEntry>& m_Schemas;
    };

    class CollectingErrorHandler : public ErrorHandler
    {
    public:
        void warning(const SAXParseException& e) override
        {
            m_Warnings.push_back(FormatException("Warning", e));
        }

        void error(const SAXParseException& e) override
        {
           
            m_HadError = true;
            if (m_ErrorMessage.empty())
                m_ErrorMessage = FormatException("Error", e);
        }

        void fatalError(const SAXParseException& e) override
        {
            m_HadError = true;
            if (m_ErrorMessage.empty())
                m_ErrorMessage = FormatException("Fatal", e);
        }

        void resetErrors() override
        {
            m_HadError = false;
            m_ErrorMessage.clear();
            m_Warnings.clear();
        }

        bool HadError() const { return m_HadError; }
        const std::string& GetErrorMessage() const { return m_ErrorMessage; }
        const std::vector<std::string>& GetWarnings() const { return m_Warnings; }

    private:
        static std::string FormatException(const char* severity, const SAXParseException& e)
        {
            return std::string(severity) +
                " at line " + std::to_string(e.getLineNumber()) +
                ", column " + std::to_string(e.getColumnNumber()) +
                ": " + XmlChToString(e.getMessage());
        }

        bool m_HadError = false;
        std::string m_ErrorMessage;
        std::vector<std::string> m_Warnings;
    };
}

class SchemaValidator::ImplSchemaValidator
{
public:
    void AddSchema(const InternalSchema& schema)
    {
        SchemaEntry entry;
        entry.systemId = schema.systemId;
        entry.namespaceUri = schema.namespaceUri;
        entry.schemaText = schema.schemaText;

        m_Schemas[entry.systemId] = std::move(entry);
    }

    void ClearSchemas()
    {
        m_Schemas.clear();
    }

    SchemaValidationResult ValidateFile(
        const std::string& xmlFilePath,
        const std::string& rootSchemaSystemId) const
    {
        XercesDOMParser parser;
        CollectingErrorHandler errorHandler;
        InMemorySchemaResolver resolver(m_Schemas);

        ConfigureParser(parser, errorHandler, resolver, rootSchemaSystemId);

        SchemaValidationResult result;

        try
        {
            parser.parse(xmlFilePath.c_str());
            PopulateResultFromParser(parser, errorHandler, result);
        }
        catch (const XMLException& e)
        {
            result.success = false;
            result.errorMessage = "XMLException: " + XmlChToString(e.getMessage());
        }
        catch (const DOMException& e)
        {
            result.success = false;
            result.errorMessage = "DOMException: " + XmlChToString(e.msg);
        }
        catch (const std::exception& e)
        {
            result.success = false;
            result.errorMessage = std::string("std::exception: ") + e.what();
        }
        catch (...)
        {
            result.success = false;
            result.errorMessage = "Unknown exception";
        }

        return result;
    }

    SchemaValidationResult ValidateXmlString(
        const std::string& xmlText,
        const std::string& virtualXmlSystemId,
        const std::string& rootSchemaSystemId) const
    {
        XercesDOMParser parser;
        CollectingErrorHandler errorHandler;
        InMemorySchemaResolver resolver(m_Schemas);

        ConfigureParser(parser, errorHandler, resolver, rootSchemaSystemId);

        SchemaValidationResult result;

        try
        {
            MemBufInputSource xmlSource(
                reinterpret_cast<const XMLByte*>(xmlText.data()),
                static_cast<XMLSize_t>(xmlText.size()),
                virtualXmlSystemId.c_str(),
                false);

            parser.parse(xmlSource);
            PopulateResultFromParser(parser, errorHandler, result);
        }
        catch (const XMLException& e)
        {
            result.success = false;
            result.errorMessage = "XMLException: " + XmlChToString(e.getMessage());
        }
        catch (const DOMException& e)
        {
            result.success = false;
            result.errorMessage = "DOMException: " + XmlChToString(e.msg);
        }
        catch (const std::exception& e)
        {
            result.success = false;
            result.errorMessage = std::string("std::exception: ") + e.what();
        }
        catch (...)
        {
            result.success = false;
            result.errorMessage = "Unknown exception";
        }

        return result;
    }

private:
    void ConfigureParser(
        XercesDOMParser& parser,
        CollectingErrorHandler& errorHandler,
        InMemorySchemaResolver& resolver,
        const std::string& rootSchemaSystemId) const
    {
        parser.setValidationScheme(XercesDOMParser::Val_Always);
        parser.setDoSchema(true);
        parser.setDoNamespaces(true);
        parser.setLoadExternalDTD(false);
        parser.setValidationConstraintFatal(true);
        parser.setEntityResolver(&resolver);
        parser.setErrorHandler(&errorHandler);

        const SchemaEntry* rootSchema = FindSchema(rootSchemaSystemId);
        if (!rootSchema)
            throw std::runtime_error("Root schema not found: " + rootSchemaSystemId);

        if (rootSchema->namespaceUri.empty())
        {
            parser.setExternalNoNamespaceSchemaLocation(rootSchema->systemId.c_str());
        }
        else
        {
            const std::string schemaLocation = rootSchema->namespaceUri + " " + rootSchema->systemId;
            parser.setExternalSchemaLocation(schemaLocation.c_str());
        }
    }

    void PopulateResultFromParser(
        XercesDOMParser& parser,
        const CollectingErrorHandler& errorHandler,
        SchemaValidationResult& outResult) const
    {
        outResult.warnings = errorHandler.GetWarnings();

        if (errorHandler.HadError())
        {
            outResult.success = false;
            outResult.errorMessage = errorHandler.GetErrorMessage();
            return;
        }

        DOMDocument* doc = parser.getDocument();
        if (!doc)
        {
            outResult.success = false;
            outResult.errorMessage = "Parser returned null DOMDocument";
            return;
        }

        DOMElement* root = doc->getDocumentElement();
        if (!root)
        {
            outResult.success = false;
            outResult.errorMessage = "XML document has no root element";
            return;
        }

        outResult.success = true;
        outResult.rootElementName = XmlChToString(root->getTagName());
    }

    const SchemaEntry* FindSchema(const std::string& systemId) const
    {
        auto it = m_Schemas.find(systemId);
        if (it == m_Schemas.end())
            return nullptr;
        return &it->second;
    }

private:
    std::unordered_map<std::string, SchemaEntry> m_Schemas;
};

SchemaValidator::SchemaValidator()
    : m_implValidator(std::make_unique<ImplSchemaValidator>())
{
}

SchemaValidator::~SchemaValidator() = default;

SchemaValidator::SchemaValidator(SchemaValidator&&) noexcept = default;
SchemaValidator& SchemaValidator::operator=(SchemaValidator&&) noexcept = default;

void SchemaValidator::AddSchema(const InternalSchema& schema)
{
    m_implValidator->AddSchema(schema);
}

void SchemaValidator::ClearSchemas()
{
    m_implValidator->ClearSchemas();
}

SchemaValidationResult SchemaValidator::ValidateFile(
    const std::string& xmlFilePath,
    const std::string& rootSchemaSystemId) const
{
    return m_implValidator->ValidateFile(xmlFilePath, rootSchemaSystemId);
}

SchemaValidationResult SchemaValidator::ValidateXmlString(
    const std::string& xmlText,
    const std::string& virtualXmlSystemId,
    const std::string& rootSchemaSystemId) const
{
    return m_implValidator->ValidateXmlString(xmlText, virtualXmlSystemId, rootSchemaSystemId);
}