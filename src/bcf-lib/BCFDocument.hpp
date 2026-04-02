#pragma once
#include <vector>
#include <string>
#include <optional>
#include <cstdint>
#include <cstdio>

// we store internall as BCF-XML v3.0
// https://github.com/buildingSMART/BCF-XML/tree/release_3_0/Documentation

using optStr = std::optional<std::string>;
using optInt = std::optional<int>;
using optBool = std::optional<bool>;

struct BCFDVec3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};
struct BCFColor
{
    std::uint8_t a = 255;
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;

    std::string toHexARGB() const
    {
        char buffer[9];
        std::snprintf(buffer, sizeof(buffer), "%02X%02X%02X%02X", a, r, g, b);
		return std::string(buffer);
    }
    void setFromHexARGB(std::string hex)
    {
        if (!hex.empty() && hex[0] == '#')
            hex.erase(hex.begin());

        if (hex.size() == 8)
        {
            a = std::stoul(hex.substr(0, 2), nullptr, 16);
            r = std::stoul(hex.substr(2, 2), nullptr, 16);
            g = std::stoul(hex.substr(4, 2), nullptr, 16);
            b = std::stoul(hex.substr(6, 2), nullptr, 16);
        }
        else if (hex.size() == 6) // assume full opacity if alpha is not provided
        {
            a = 255;
            r = std::stoul(hex.substr(0, 2), nullptr, 16);
            g = std::stoul(hex.substr(2, 2), nullptr, 16);
            b = std::stoul(hex.substr(4, 2), nullptr, 16);
		}
	}
};

struct BCFProject // .bcfp file, optional
{
    std::string projectId;
    std::string name;
    std::string extensionSchema;
};

struct BCFTopic
{
	// attributes
    std::string guid;
    optStr serverAssignedId;
    std::string topicType;
    std::string topicStatus;

    // elements
    std::vector<std::string> referenceLinks;
    std::string title;
    optStr priority;
    optInt index;
    std::vector<std::string> labels;
    std::string creationDate;
	optStr creationAuthor;
	optStr modifiedDate;
	optStr modifiedAuthor;
    optStr dueDate;
    optStr assignedTo;
    optStr description;
    optStr stage;
};

struct BCFComment
{
   // attributes
    std::string guid;

    // elements
    std::string date;
    std::string author;
    optStr text;
	optStr viewpointGuid;
    optStr modifiedDate;
	optStr modifiedAuthor;
};

struct BCFViewpointRef
{
    // attributes
    std::string guid;

    // elements
    optStr viewpointFileName;
	optStr snapshotFileName;
    optInt index;
};

struct BCFMarkup
{
	BCFTopic topic;
	std::vector<BCFComment> comments;
	std::vector<BCFViewpointRef> viewpointRefs;
};

struct BCFComponentRef
{
    // attributes
    optStr ifcGuid;

    // elements
    optStr originatingSystem;
    optStr authoringToolId;
};

struct BCFViewSetupHints
{
    bool spacesVisible = false;
    bool spaceBoundariesVisible = false;
    bool openingsVisible = false;
};

struct BCFComponentSelection
{
    // any component in here is marked as selected
    std::vector<BCFComponentRef> components;
};

struct BCFComponentVisibility
{
    bool defaultVisibility = true;

    std::optional<BCFViewSetupHints> viewSetupHints;

    std::vector<BCFComponentRef> exceptions;

    // visibility of components is applied as:
    // 1. apply default vis
	// 2. apply view setup hints (if any)
	// 3. apply exceptions (if any) [inverts the visibility of the component]
};

struct BCFColorGroup
{
    BCFColor color; // ARGB
    std::vector<BCFComponentRef> components;
};

struct BCFComponentColoring
{
    std::vector<BCFColorGroup> groups;
};

struct BCFComponents
{
    std::optional<BCFComponentSelection> selection;
    std::optional<BCFComponentVisibility> visibility;
    std::optional<BCFComponentColoring> coloring;
};

struct BCFCamera
{
	bool perspective = true;

    BCFDVec3 cameraViewPoint;
    BCFDVec3 cameraDirection;
    BCFDVec3 cameraUpVector;

	double viewToWorldScale = 1.0; // only for orthogonal cameras, the size of the view in world units (e.g. meters)
	float fieldOfView = 0.0; // only for perspective cameras, in degrees

    float aspectRatio = 1.0f; // Assume 1.0 when reading previous BCF versions
};
struct BCFLine
{
    BCFDVec3 startPoint;
	BCFDVec3 endPoint;
};
struct BCFClippingPlane
{
    BCFDVec3 location;
    BCFDVec3 direction;
};
struct BCFBitmap
{
    enum class BCFBitmapFormat
    {
        PNG,
        JPG,
        UNKNOWN
	} bitmapFormat = BCFBitmapFormat::UNKNOWN;
	std::string reference; // file name of the bitmap in the topic folder
    BCFDVec3 location;
    BCFDVec3 normal;
    BCFDVec3 up;
	double height; // in world units (e.g. meters)
};

struct BCFViewpoint
{
    optStr guid;
    BCFComponents components;
	std::optional<BCFCamera> camera;
	std::vector<BCFLine> lines;
	std::vector<BCFClippingPlane> clippingPlanes;
	std::vector<BCFBitmap> bitmaps;
};


struct BCFDocument
{
    std::string version;
	std::optional<BCFProject> project;

    struct BCFTopicEntry
    {
        std::string guid;
		BCFMarkup markup;
		std::vector<BCFViewpoint> viewpoints;
		std::vector<std::string> snapshotRefs; // file names of the snapshots in the topic folder
    };

	std::vector<BCFTopicEntry> topicEntries;
};
