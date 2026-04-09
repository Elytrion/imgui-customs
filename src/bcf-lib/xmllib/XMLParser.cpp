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

    class MemorySchemaResolver : public EntityResolver
    {
    public:
        MemorySchemaResolver(const std::string& systemId, const std::string& rawText)
            : m_SystemId(systemId), m_RawText(rawText)
        {}

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
                    false // prevent system from releasing the rawtext memory and creating a mem leak
                );
            }

            return nullptr;
        }

    private:
        std::string m_SystemId; // identifier
        std::string m_RawText;  // actual XSD text
    };
}

XMLDocumentHandle XMLParser::Parse(const std::string & xmlFilePath, const XMLSchema & schema, const XMLParseConfig & config)
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

        parser.setValidationConstraintFatal(config.validationConstraintFatal);
        parser.setValidationSchemaFullChecking(config.validationSchemaFullChecking);
        parser.setCreateEntityReferenceNodes(config.createEntityReferenceNodes);
        parser.setLoadExternalDTD(false);
        parser.setErrorHandler(&errorHandler);

        const bool useSchemaFile = !schema.schemaFilePath.empty();
        const bool useRawSchema = !useSchemaFile && !schema.rawText.empty();
        const bool useSchema = useSchemaFile || useRawSchema;
        parser.setDoNamespaces(useSchema ? true : config.doNamespaces);

        MemorySchemaResolver resolver(
            schema.systemId, schema.rawText
        );

        if (useSchemaFile || useRawSchema)
        {
            parser.setValidationScheme(XercesDOMParser::Val_Always);
            parser.setDoSchema(true);

            if (useSchemaFile)
            {
                // pull it from the file path
                parser.setExternalNoNamespaceSchemaLocation(schema.schemaFilePath.c_str());
            }
            else
            {
                const char* systemId = schema.systemId.c_str();
                parser.setExternalNoNamespaceSchemaLocation(systemId);
                parser.setEntityResolver(&resolver);
            }
        }
        else
        {
            // if no valid schema provided, do not validate
            parser.setValidationScheme(XercesDOMParser::Val_Never);
            parser.setDoSchema(false);
        }

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

        result.adoptDocument(adoptedDoc); // move document from parser to our holder

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