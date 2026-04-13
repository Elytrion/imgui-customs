#include "XMLParser.h"
#include "XMLMain.hpp"

#include <cstring>
#include <string>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/EntityResolver.hpp>			// used to redirect to internal memory schema
#include <xercesc/sax/ErrorHandler.hpp>				// custom error handler to display errors during validation
#include <xercesc/sax/SAXParseException.hpp>		// for some reason, SAXParseExceptions are the ones thrown even during DOM parsing
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/dom/DOM.hpp>

#include "XMLUtils.hpp"

namespace XMLLib
{
using namespace xercesc;

namespace
{
    // custom error handler
    class SimpleErrorHandler : public ErrorHandler
    {
    public:
        void warning(const SAXParseException& e) override
        {
            if (m_Error.empty())
                m_Error = "Warning: " + XmlChToString(e.getMessage());
        }

        void error(const SAXParseException& e) override
        {
            if (m_Error.empty())
                m_Error = "Error: " + XmlChToString(e.getMessage());
        }

        void fatalError(const SAXParseException& e) override
        {
            if (m_Error.empty())
                m_Error = "Fatal: " + XmlChToString(e.getMessage());
        }

        void resetErrors() override
        {
            m_Error.clear();
        }

        const std::string& GetError() const { return m_Error; }

    private:
        std::string m_Error;
    };

    // custom resolver to catch header-only schemas and raw text xml
    class MemorySchemaResolver : public EntityResolver
    {
    public:
        MemorySchemaResolver(const std::string& systemId, const std::string& rawText)
            : m_SystemId(systemId), m_RawText(rawText)
        {
        }

        InputSource* resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId) override
        {
            (void)publicId;

            const std::string requestedId = XmlChToString(systemId);

            if (requestedId == m_SystemId)
            {
                return new MemBufInputSource(
                    reinterpret_cast<const XMLByte*>(m_RawText.data()),
                    static_cast<XMLSize_t>(m_RawText.size()),
                    m_SystemId.c_str(),
                    false
                );
            }

            return nullptr;
        }

    private:
        std::string m_SystemId;
        std::string m_RawText;
    };

    bool ConfigureParser(
        XercesDOMParser& parser,
        SimpleErrorHandler& errorHandler,
        MemorySchemaResolver& resolver,
        const XMLSchema& schema,
        const XMLParseConfig& config)
    {
        parser.setValidationConstraintFatal(config.validationConstraintFatal);
        parser.setValidationSchemaFullChecking(config.validationSchemaFullChecking);
        parser.setCreateEntityReferenceNodes(config.createEntityReferenceNodes);
        parser.setLoadExternalDTD(false);
        parser.setErrorHandler(&errorHandler);

        const bool useSchemaFile = !schema.schemaFilePath.empty();
        const bool useRawSchema = !useSchemaFile && !schema.rawText.empty();
        const bool useSchema = useSchemaFile || useRawSchema;

        parser.setDoNamespaces(useSchema ? true : config.doNamespaces);

        if (useSchema)
        {
            parser.setValidationScheme(XercesDOMParser::Val_Always);
            parser.setDoSchema(true);

            if (useSchemaFile)
            {
                parser.setExternalNoNamespaceSchemaLocation(schema.schemaFilePath.c_str());
            }
            else
            {
                parser.setExternalNoNamespaceSchemaLocation(schema.systemId.c_str());
                parser.setEntityResolver(&resolver);
            }
        }
        else
        {
            parser.setValidationScheme(XercesDOMParser::Val_Never);
            parser.setDoSchema(false);
        }

        return true;
    }
}

XMLDocumentHandle XMLParser::Parse(const std::string& xmlFilePath, const XMLSchema& schema, const XMLParseConfig& config)
{
    XMLDocumentHandle result;

    if (!IsXMLLibInit())
    {
        result.setError("XML Lib not initialised");
        return result;
    }

    try
    {
        XercesDOMParser parser;
        SimpleErrorHandler errorHandler;
        MemorySchemaResolver resolver(schema.systemId, schema.rawText);

        ConfigureParser(parser, errorHandler, resolver, schema, config);

        parser.parse(xmlFilePath.c_str());

        if (!errorHandler.GetError().empty())
        {
            result.setError(errorHandler.GetError());
            return result;
        }

        DOMDocument* adoptedDoc = parser.adoptDocument();
        if (!adoptedDoc)
        {
            result.setError("Failed to adopt parsed XML document");
            return result;
        }

        result.adoptDocument(adoptedDoc);
        return result;
    }
    catch (const XMLException& e)
    {
        result.setError("XMLException: " + XmlChToString(e.getMessage()));
        return result;
    }
    catch (const DOMException& e)
    {
        result.setError("DOMException: " + XmlChToString(e.msg));
        return result;
    }
    catch (const std::exception& e)
    {
        result.setError(std::string("std::exception: ") + e.what());
        return result;
    }
    catch (...)
    {
        result.setError("Unknown exception during XML parse");
        return result;
    }
}

XMLDocumentHandle XMLParser::ParseMemory(const std::string& xmlFileName, const std::string& xmlText, const XMLSchema& schema, const XMLParseConfig& config)
{
    XMLDocumentHandle result;

    if (!IsXMLLibInit())
    {
        result.setError("XML Lib not initialised");
        return result;
    }

    try
    {
        XercesDOMParser parser;
        SimpleErrorHandler errorHandler;
        MemorySchemaResolver resolver(schema.systemId, schema.rawText);

        ConfigureParser(parser, errorHandler, resolver, schema, config);

        MemBufInputSource xmlSource(
            reinterpret_cast<const XMLByte*>(xmlText.data()),
            static_cast<XMLSize_t>(xmlText.size()),
            xmlFileName.c_str(),
            false
        );

        parser.parse(xmlSource);

        if (!errorHandler.GetError().empty())
        {
            result.setError(errorHandler.GetError());
            return result;
        }

        DOMDocument* adoptedDoc = parser.adoptDocument();
        if (!adoptedDoc)
        {
            result.setError("Failed to adopt parsed XML document");
            return result;
        }

        result.adoptDocument(adoptedDoc);
        return result;
    }
    catch (const XMLException& e)
    {
        result.setError("XMLException: " + XmlChToString(e.getMessage()));
        return result;
    }
    catch (const DOMException& e)
    {
        result.setError("DOMException: " + XmlChToString(e.msg));
        return result;
    }
    catch (const std::exception& e)
    {
        result.setError(std::string("std::exception: ") + e.what());
        return result;
    }
    catch (...)
    {
        result.setError("Unknown exception during XML parse");
        return result;
    }
}

}
