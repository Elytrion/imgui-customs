#pragma once
#include "BCFDocumentStore.hpp"
#include <optional>
#include <vector>
#include <unordered_map>

struct ImageData
{
	int width = 0;
	int height = 0;
	int channels = 0;
	std::vector<std::uint8_t> data;
};

struct BCFViewpoint
{
	BCFDocumentRef doc;
	std::string guid;
	std::string fileName;
	// TODO: accessor functions
};

struct BCFMarkup
{
	BCFDocumentRef doc;
	// TODO: accessor functions
};

struct BCFSnapshot
{
	ImageData image;
	std::string fileName;
};

struct BCFDocument
{
	bool isValid() const { return valid; }
	bool hasDocumentsFolder() const { return hasDocFolder; }
	const std::string& getSourcePath() const { return srcPath; }

	BCFDocumentRef version;						// .version file, required
	std::optional<BCFDocumentRef> extensions;	// .xml file, optional, defines the extensions of a project
	std::optional<BCFDocumentRef> documents;	// .xml file, optional, defines any additional documents in a project
	std::optional<BCFDocumentRef> project;		// .bcfp file, optional, defines details of a project

	struct BCFTopic
	{
		bool isValid() const;					// if false, this topic's markup doc cannot be parsed
		std::string guid{};						// name of the folder is always the guid of the topic
		BCFMarkup markup;						// .bcf file, required
		std::vector<BCFViewpoint> viewpoints;	// .bcfv files, can be multiple or none
		std::vector<BCFSnapshot> snapshots;		// snapshots in the topic folder, can be multiple or none
	};

	std::unordered_map<std::string, BCFDocument::BCFTopic> topics;

	// TODO: convience accessor functions 

private:
	friend class BCFIO;
	bool valid = false;
	bool hasDocFolder = false;
	std::string srcPath = "";
};
