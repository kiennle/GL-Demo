#pragma once
#include <rapidjson/document.h>

rapidjson::Document parse_json(const char *path);
int json_read_floats(float *dst, rapidjson::Value &js);

static inline const char *json_try_get_string(const rapidjson::Value &jsval, const char *member, const char *default_value)
{
	rapidjson::Document::ConstMemberIterator it = jsval.FindMember(member);

	if (it == jsval.MemberEnd())
		return default_value;
	return it->value.GetString();
}

static inline bool json_try_get_bool(const rapidjson::Value &jsval, const char *member, bool default_value)
{
	rapidjson::Document::ConstMemberIterator it = jsval.FindMember(member);

	if (it == jsval.MemberEnd())
		return default_value;
	return it->value.GetBool();
}

static inline int json_try_get_int(const rapidjson::Value &jsval, const char *member, int default_value)
{
	rapidjson::Document::ConstMemberIterator it = jsval.FindMember(member);

	if (it == jsval.MemberEnd())
		return default_value;
	return it->value.GetInt();
}

static inline float json_try_get_float(const rapidjson::Value &jsval, const char *member, float default_value)
{
	rapidjson::Document::ConstMemberIterator it = jsval.FindMember(member);

	if (it == jsval.MemberEnd())
		return default_value;
	return it->value.GetFloat();
}
