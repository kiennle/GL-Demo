#include <assert.h>
#include "file.h"
#include "json.h"

int json_read_floats(float *dst, rapidjson::Value &js)
{
	rapidjson::Value::ValueIterator iter;

	assert(js.IsArray());
	for (iter = js.Begin(); iter != js.End(); iter++, dst++)
		*dst = iter->GetFloat();
	return js.Size();
}

rapidjson::Document parse_json(const char *path)
{
	rapidjson::Document doc;
	const char *str;
	int len;

	str = (const char *)mapfile(path, &len);
	doc.Parse(str, len);
	unmapfile(str, len);
	return doc;
}
