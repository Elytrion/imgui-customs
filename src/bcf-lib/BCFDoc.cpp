#include "BCFDoc.h"


bool BCFDocument::BCFTopic::isValid() const
{
	return markup.doc.IsValid() && !guid.empty();
}