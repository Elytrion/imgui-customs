#pragma once
#pragma once
#include <memory>
#include <string>
#include <vector>

namespace XMLLib
{
struct InternalSchema
{
    // Used as the schema's virtual "filename" / system identifier.
    // Example: "markup.xsd", "visinfo.xsd", "note.xsd"
    std::string systemId;

    // Leave empty for no-namespace schema validation.
    // If set, validator will use namespace-aware schema location mode.
    std::string namespaceUri;

    // Raw XSD text
    std::string schemaText;
};

struct SchemaValidationResult
{
    bool success = false;
    std::string errorMessage;
    std::vector<std::string> warnings;
    std::string rootElementName;
};

class SchemaValidator
{
public:
    SchemaValidator();
    ~SchemaValidator();

    SchemaValidator(const SchemaValidator&) = delete;
    SchemaValidator& operator=(const SchemaValidator&) = delete;

    SchemaValidator(SchemaValidator&&) noexcept;
    SchemaValidator& operator=(SchemaValidator&&) noexcept;

    // Adds or replaces an embedded schema.
    void AddSchema(const InternalSchema& schema);

    // Clears all known schemas.
    void ClearSchemas();

    // Validate an XML file on disk against a named root schema.
    SchemaValidationResult ValidateFile(
        const std::string& xmlFilePath,
        const std::string& rootSchemaSystemId) const;

    // Validate XML text in memory against a named root schema.
    SchemaValidationResult ValidateXmlString(
        const std::string& xmlText,
        const std::string& virtualXmlSystemId,
        const std::string& rootSchemaSystemId) const;

private:
    class ImplSchemaValidator;
    std::unique_ptr<ImplSchemaValidator> m_implValidator;
};
}