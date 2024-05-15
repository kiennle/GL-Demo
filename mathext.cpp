#include "mathext.h"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>

/*
 * Copied from FlaxEngine.
 * http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition
 */
bool equalf(float a, float b)
{
	if (fabsf(a - b) < ZERO_TOLERANCE)
		return true;

	int32_t aInt = *(int32_t *)&a;
	int32_t bInt = *(int32_t *)&b;

	if ((aInt < 0) != (bInt < 0))
		return false;

	int ulp = fabsf(aInt - bInt);

	int maxUlp = 4;

	return ulp <= maxUlp;
}

/*
result = (2 * t^3 - 3 * t^2 + 1) * position0 +
	 (t^3 - 2 * t^2 + t) * tangent0 +
	 (-2 * t^3 + 3 * t^2) * position1 +
	 (t^3 - t^2) * tangent1
*/
float hermitef(float pos0, float tan0, float pos1, float tan1, float t)
{
	float h1, h2, h3, h4;
	float result;

	h1 = 2.0f * t * t * t - 3.0f * t * t + 1.0f;
	h2 = t * t * t - 2.0f * t * t + t;
	h3 = -2.0f * t * t * t + 3.0f * t * t;
	h4 = t * t * t - t * t;

	result = h1 * pos0 + h2 * tan0 + h3 * pos1 + h4 * tan1;
	return result;
}

/*
result = (2 * t^3 - 3 * t^2 + 1) * position0 +
	 (t^3 - 2 * t^2 + t) * tangent0 +
	 (-2 * t^3 + 3 * t^2) * position1 +
	 (t^3 - t^2) * tangent1
*/
float2 hermite2f(float2 pos0, float2 tan0, float2 pos1, float2 tan1, float t)
{
	float h1, h2, h3, h4;
	float2 result;

	h1 = 2.0f * t * t * t - 3.0f * t * t + 1.0f;
	h2 = t * t * t - 2.0f * t * t + t;
	h3 = -2.0f * t * t * t + 3.0f * t * t;
	h4 = t * t * t - t * t;

	result.x = h1 * pos0.x + h2 * tan0.x + h3 * pos1.x + h4 * tan1.x;
	result.y = h1 * pos0.y + h2 * tan0.y + h3 * pos1.y + h4 * tan1.y;
	return result;
}

/*
result = (2 * t^3 - 3 * t^2 + 1) * position0 +
	 (t^3 - 2 * t^2 + t) * tangent0 +
	 (-2 * t^3 + 3 * t^2) * position1 +
	 (t^3 - t^2) * tangent1
*/
float3 hermite3f(float3 pos0, float3 tan0, float3 pos1, float3 tan1, float t)
{
	float h1, h2, h3, h4;
	float3 result;

	h1 = 2.0f * t * t * t - 3.0f * t * t + 1.0f;
	h2 = t * t * t - 2.0f * t * t + t;
	h3 = -2.0f * t * t * t + 3.0f * t * t;
	h4 = t * t * t - t * t;

	result.x = h1 * pos0.x + h2 * tan0.x + h3 * pos1.x + h4 * tan1.x;
	result.y = h1 * pos0.y + h2 * tan0.y + h3 * pos1.y + h4 * tan1.y;
	result.z = h1 * pos0.z + h2 * tan0.z + h3 * pos1.z + h4 * tan1.z;
	return result;
}

/*
result = (2 * t^3 - 3 * t^2 + 1) * position0 +
	 (t^3 - 2 * t^2 + t) * tangent0 +
	 (-2 * t^3 + 3 * t^2) * position1 +
	 (t^3 - t^2) * tangent1
*/
float4 hermite4f(float4 pos0, float4 tan0, float4 pos1, float4 tan1, float t)
{
	float h1, h2, h3, h4;
	float4 result;

	h1 = 2.0f * t * t * t - 3.0f * t * t + 1.0f;
	h2 = t * t * t - 2.0f * t * t + t;
	h3 = -2.0f * t * t * t + 3.0f * t * t;
	h4 = t * t * t - t * t;

	result.x = h1 * pos0.x + h2 * tan0.x + h3 * pos1.x + h4 * tan1.x;
	result.y = h1 * pos0.y + h2 * tan0.y + h3 * pos1.y + h4 * tan1.y;
	result.z = h1 * pos0.z + h2 * tan0.z + h3 * pos1.z + h4 * tan1.z;
	result.w = h1 * pos0.w + h2 * tan0.w + h3 * pos1.w + h4 * tan1.w;
	return result;
}

half pack_half(float x) {
	half result;

	result.packed = DirectX::PackedVector::XMConvertFloatToHalf(x);
	return result;
}

half2 pack_half2(float x, float y) {
	half2 result;

	result.x = pack_half(x);
	result.y = pack_half(y);
	return result;
}

int64_t pack_snorm(double x, int precision) {
	int64_t max = ((int64_t)1 << (precision - 1)) - 1;

	return (int64_t)(x * (double)max); /* scale to range [-max, max] */
}

uint64_t pack_unorm(double x, int precision) {
	uint64_t max = ((uint64_t)1 << precision) - 1;

	return (uint64_t)(x * (double)max); /* scale to range [0, max] */
}

decn4 pack_decn4(float x, float y, float z, float w) {
	decn4 result;

	result.packed =
		((uint32_t)pack_snorm(w, 2) & 0x3) << 30 |
		((uint32_t)pack_snorm(z, 10) & 0x3ff) << 20 |
		((uint32_t)pack_snorm(y, 10) & 0x3ff) << 10 |
		((uint32_t)pack_snorm(x, 10) & 0x3ff);
	return result;
}

udecn4 pack_udecn4(float x, float y, float z, float w) {
	udecn4 result;

	result.packed =
		((uint32_t)pack_unorm(w, 2) & 0x3) << 30 |
		((uint32_t)pack_unorm(z, 10) & 0x3ff) << 20 |
		((uint32_t)pack_unorm(y, 10) & 0x3ff) << 10 |
		((uint32_t)pack_unorm(x, 10) & 0x3ff);
	return result;
}

double unpack_snorm(int64_t x, int precision) {
	int64_t max = ((int64_t)1 << (precision - 1)) - 1;

	x &= max; /* max is also the bitmask for value */
	return (double)x / (double)max;
}

double unpack_unorm(uint64_t x, int precision) {
	uint64_t max = ((uint64_t)1 << precision) - 1;

	x &= max; /* max is also the bitmask for value */
	return (double)x / (double)max;
}

float2 add2f(float2 lhs, float2 rhs)
{
	float2 result;

	result.x = lhs.x + rhs.x;
	result.y = lhs.y + rhs.y;
	return result;
}

float2 sub2f(float2 lhs, float2 rhs)
{
	float2 result;

	result.x = lhs.x - rhs.x;
	result.y = lhs.y - rhs.y;
	return result;
}

float2 mul2f(float2 lhs, float2 rhs)
{
	float2 result;

	result.x = lhs.x * rhs.x;
	result.y = lhs.y * rhs.y;
	return result;
}

float3 neg3f(float3 v)
{
	float3 result;

	result.x = v.x * -1.0f;
	result.y = v.y * -1.0f;
	result.z = v.z * -1.0f;
	return result;
}

float3 add3f(float3 lhs, float3 rhs) {
	float3 result;
	DirectX::XMVECTOR a = DirectX::XMVectorSet(lhs.x, lhs.y, lhs.z, 0.0f);
	DirectX::XMVECTOR b = DirectX::XMVectorSet(rhs.x, rhs.y, rhs.z, 0.0f);
	DirectX::XMVECTOR c = DirectX::XMVectorAdd(a, b);
	result.x = DirectX::XMVectorGetX(c);
	result.y = DirectX::XMVectorGetY(c);
	result.z = DirectX::XMVectorGetZ(c);
	return result;
}

float3 sub3f(float3 lhs, float3 rhs) {
	float3 result;
	DirectX::XMVECTOR a = DirectX::XMVectorSet(lhs.x, lhs.y, lhs.z, 0.0f);
	DirectX::XMVECTOR b = DirectX::XMVectorSet(rhs.x, rhs.y, rhs.z, 0.0f);
	DirectX::XMVECTOR c = DirectX::XMVectorSubtract(a, b);
	result.x = DirectX::XMVectorGetX(c);
	result.y = DirectX::XMVectorGetY(c);
	result.z = DirectX::XMVectorGetZ(c);
	return result;
}

float3 mul3f(float3 lhs, float3 rhs) {
	float3 result;
	DirectX::XMVECTOR a = DirectX::XMVectorSet(lhs.x, lhs.y, lhs.z, 0.0f);
	DirectX::XMVECTOR b = DirectX::XMVectorSet(rhs.x, rhs.y, rhs.z, 0.0f);
	DirectX::XMVECTOR c = DirectX::XMVectorMultiply(a, b);
	result.x = DirectX::XMVectorGetX(c);
	result.y = DirectX::XMVectorGetY(c);
	result.z = DirectX::XMVectorGetZ(c);
	return result;
}

float3 scale3f(float3 lhs, float rhs) {
	float3 result;
	DirectX::XMVECTOR a = DirectX::XMVectorSet(lhs.x, lhs.y, lhs.z, 0.0f);
	DirectX::XMVECTOR b = DirectX::XMVectorScale(a, rhs);
	result.x = DirectX::XMVectorGetX(b);
	result.y = DirectX::XMVectorGetY(b);
	result.z = DirectX::XMVectorGetZ(b);
	return result;
}

float3 div3f(float3 lhs, float3 rhs) {
	float3 result;
	DirectX::XMVECTOR a = DirectX::XMVectorSet(lhs.x, lhs.y, lhs.z, 0.0f);
	DirectX::XMVECTOR b = DirectX::XMVectorSet(rhs.x, rhs.y, rhs.z, 0.0f);
	DirectX::XMVECTOR c = DirectX::XMVectorDivide(a, b);
	result.x = DirectX::XMVectorGetX(c);
	result.y = DirectX::XMVectorGetY(c);
	result.z = DirectX::XMVectorGetZ(c);
	return result;
}

float length3f(float3 v)
{
	DirectX::XMVECTOR a = DirectX::XMVectorSet(v.x, v.y, v.z, 0.0f);
	DirectX::XMVECTOR b = DirectX::XMVector3Length(a);
	float result = DirectX::XMVectorGetX(b);
	return result;
}

float lengthsq3f(float3 v)
{
	DirectX::XMVECTOR a = DirectX::XMVectorSet(v.x, v.y, v.z, 0.0f);
	DirectX::XMVECTOR b = DirectX::XMVector3LengthSq(a);
	float result = DirectX::XMVectorGetX(b);
	return result;
}

float3 normalize3f(float3 v) {
	float3 result;
	DirectX::XMVECTOR tmp1, tmp2;
	tmp1 = DirectX::XMVectorSet(v.x, v.y, v.z, 0.0f);
	tmp2 = DirectX::XMVector3Normalize(tmp1);
	result.x = DirectX::XMVectorGetX(tmp2);
	result.y = DirectX::XMVectorGetY(tmp2);
	result.z = DirectX::XMVectorGetZ(tmp2);
	return result;
}

float3 cross3f(float3 a, float3 b) {
	float3 result;
	DirectX::XMVECTOR tmp1, tmp2, tmp3;
	tmp1 = DirectX::XMVectorSet(a.x, a.y, a.z, 0.0f);
	tmp2 = DirectX::XMVectorSet(b.x, b.y, b.z, 0.0f);
	tmp3 = DirectX::XMVector3Cross(tmp1, tmp2);
	result.x = DirectX::XMVectorGetX(tmp3);
	result.y = DirectX::XMVectorGetY(tmp3);
	result.z = DirectX::XMVectorGetZ(tmp3);
	return result;
}

float3 rotate3f(float3 v, float4 quat) {
	float3 result;
	DirectX::XMVECTOR a = DirectX::XMVectorSet(v.x, v.y, v.z, 0.0f);
	DirectX::XMVECTOR b = DirectX::XMVectorSet(quat.x, quat.y, quat.z, quat.w);
	DirectX::XMVECTOR c = DirectX::XMVector3Rotate(a, b);
	result.x = DirectX::XMVectorGetX(c);
	result.y = DirectX::XMVectorGetY(c);
	result.z = DirectX::XMVectorGetZ(c);
	return result;
}

float clamp(float value, float min, float max)
{
	if (value < min)
		value = min;
	else if (value > max)
		value = max;
	return value;
}

float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

float2 lerp2f(float2 a, float2 b, float t)
{
	float2 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	return result;
}

float3 lerp3f(float3 a, float3 b, float t)
{
	float3 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	result.z = lerp(a.z, b.z, t);
	return result;
}

float4 quat_slerp(float4 a, float4 b, float t)
{
	DirectX::XMVECTOR q1 = DirectX::XMVectorSet(a.x, a.y, a.z, a.w);
	DirectX::XMVECTOR q2 = DirectX::XMVectorSet(b.x, b.y, b.z, b.w);
	DirectX::XMVECTOR q3 = DirectX::XMQuaternionSlerp(q1, q2, t);
	float4 result;
	result.x = DirectX::XMVectorGetX(q3);
	result.y = DirectX::XMVectorGetY(q3);
	result.z = DirectX::XMVectorGetZ(q3);
	result.w = DirectX::XMVectorGetW(q3);
	return result;
}

float4 quat_rotateaxis(float3 axis, float radians) {
	DirectX::XMVECTOR a = DirectX::XMVectorSet(axis.x, axis.y, axis.z, 0.0f);
	DirectX::XMVECTOR v = DirectX::XMQuaternionRotationAxis(a, radians);
	float4 result;
	result.x = DirectX::XMVectorGetX(v);
	result.y = DirectX::XMVectorGetY(v);
	result.z = DirectX::XMVectorGetZ(v);
	result.w = DirectX::XMVectorGetW(v);
	return result;
}

float4 quat_mul(float4 lhs, float4 rhs) {
	float4 result;
	DirectX::XMVECTOR a = DirectX::XMVectorSet(lhs.x, lhs.y, lhs.z, lhs.w);
	DirectX::XMVECTOR b = DirectX::XMVectorSet(rhs.x, rhs.y, rhs.z, rhs.w);
	DirectX::XMVECTOR c = DirectX::XMQuaternionMultiply(a, b);
	result.x = DirectX::XMVectorGetX(c);
	result.y = DirectX::XMVectorGetY(c);
	result.z = DirectX::XMVectorGetZ(c);
	result.w = DirectX::XMVectorGetW(c);
	return result;
}

float4 quat_normalize(float4 quat)
{
	DirectX::XMVECTOR q = DirectX::XMVectorSet(quat.x, quat.y, quat.z, quat.w);
	DirectX::XMVECTOR v = DirectX::XMQuaternionNormalize(q);
	float4 result;
	result.x = DirectX::XMVectorGetX(v);
	result.y = DirectX::XMVectorGetY(v);
	result.z = DirectX::XMVectorGetZ(v);
	result.w = DirectX::XMVectorGetW(v);
	return result;
}

float4 quat_inverse(float4 quat)
{
	DirectX::XMVECTOR q = DirectX::XMVectorSet(quat.x, quat.y, quat.z, quat.w);
	DirectX::XMVECTOR v = DirectX::XMQuaternionInverse(q);
	float4 result;
	result.x = DirectX::XMVectorGetX(v);
	result.y = DirectX::XMVectorGetY(v);
	result.z = DirectX::XMVectorGetZ(v);
	result.w = DirectX::XMVectorGetW(v);
	return result;
}

float3 quat_right(float4 quat)
{
	return rotate3f(float3_right, quat);
}

float3 quat_left(float4 quat)
{
	return rotate3f(float3_left, quat);
}

float3 quat_up(float4 quat)
{
	return rotate3f(float3_up, quat);
}

float3 quat_down(float4 quat)
{
	return rotate3f(float3_down, quat);
}

float3 quat_forward(float4 quat)
{
	return rotate3f(float3_forward, quat);
}

float3 quat_backward(float4 quat)
{
	return rotate3f(float3_backward, quat);
}

float4x4 mat_from_floats(float *elems) {
	float4x4 m = {
		elems[0], elems[1], elems[2], elems[3],
		elems[4], elems[5], elems[6], elems[7],
		elems[8], elems[9], elems[10], elems[11],
		elems[12], elems[13], elems[14], elems[15]
	};
	return m;
}

float4x4 mat_mul(float4x4 lhs, float4x4 rhs) {
	DirectX::XMMATRIX &a = (DirectX::XMMATRIX &)lhs;
	DirectX::XMMATRIX &b = (DirectX::XMMATRIX &)rhs;
	DirectX::XMMATRIX result = DirectX::XMMatrixMultiply(a, b);
	return (float4x4 &)result;
}

float3 mat_transform_normal(float4x4 m, float3 normal) {
	float3 result;
	DirectX::XMVECTOR v = DirectX::XMVectorSet(normal.x, normal.y, normal.z, 0.0f);
	v = DirectX::XMVector3TransformNormal(v, (const DirectX::XMMATRIX &)m);
	result.x = DirectX::XMVectorGetX(v);
	result.y = DirectX::XMVectorGetY(v);
	result.z = DirectX::XMVectorGetZ(v);
	return result;
}

float4x4 mat_translate(float3 v) {
	DirectX::XMMATRIX result = DirectX::XMMatrixTranslation(v.x, v.y, v.z);
	return (float4x4 &)result;
}

float4x4 mat_rotate(float4 quat) {
	DirectX::XMVECTOR v = DirectX::XMVectorSet(quat.x, quat.y, quat.z, quat.w);
	DirectX::XMMATRIX result = DirectX::XMMatrixRotationQuaternion(v);
	return (float4x4 &)result;
}

float4x4 mat_rotateaxis(float3 axis, float radians) {
	DirectX::XMVECTOR a = DirectX::XMVectorSet(axis.x, axis.y, axis.z, 0.0f);
	DirectX::XMMATRIX result = DirectX::XMMatrixRotationAxis(a, radians);
	return (float4x4 &)result;
}

float4x4 mat_rotatex(float radians) {
	DirectX::XMMATRIX result = DirectX::XMMatrixRotationX(radians);
	return (float4x4 &)result;
}

float4x4 mat_rotatey(float radians) {
	DirectX::XMMATRIX result = DirectX::XMMatrixRotationY(radians);
	return (float4x4 &)result;
}

float4x4 mat_scale(float3 v) {
	DirectX::XMMATRIX result = DirectX::XMMatrixScaling(v.x, v.y, v.z);
	return (float4x4 &)result;
}

float4x4 mat_transpose(float4x4 m) {
	DirectX::XMMATRIX &a = (DirectX::XMMATRIX &)m;
	DirectX::XMMATRIX result = DirectX::XMMatrixTranspose(a);
	return (float4x4 &)result;
}

float4x4 mat_inverse(float4x4 m) {
	DirectX::XMMATRIX &a = (DirectX::XMMATRIX &)m;
	DirectX::XMMATRIX result = DirectX::XMMatrixInverse(NULL, a);
	return (float4x4 &)result;
}

float4x4 mat_lookto(float3 eyepos, float3 forward, float3 up) {
	DirectX::XMVECTOR e = DirectX::XMVectorSet(eyepos.x, eyepos.y, eyepos.z, 0.0f);
	DirectX::XMVECTOR d = DirectX::XMVectorSet(forward.x, forward.y, forward.z, 0.0f);
	DirectX::XMVECTOR u = DirectX::XMVectorSet(up.x, up.y, up.z, 0.0f);
	DirectX::XMMATRIX result = DirectX::XMMatrixLookToLH(e, d, u);
	return (float4x4 &)result;
}

float4x4 mat_lookto_rh(float3 eyepos, float3 forward, float3 up) {
	DirectX::XMVECTOR e = DirectX::XMVectorSet(eyepos.x, eyepos.y, eyepos.z, 0.0f);
	DirectX::XMVECTOR d = DirectX::XMVectorSet(forward.x, forward.y, forward.z, 0.0f);
	DirectX::XMVECTOR u = DirectX::XMVectorSet(up.x, up.y, up.z, 0.0f);
	DirectX::XMMATRIX result = DirectX::XMMatrixLookToRH(e, d, u);
	return (float4x4 &)result;
}

float4x4 mat_lookat(float3 eyepos, float3 target, float3 up) {
	DirectX::XMVECTOR e = DirectX::XMVectorSet(eyepos.x, eyepos.y, eyepos.z, 0.0f);
	DirectX::XMVECTOR t = DirectX::XMVectorSet(target.x, target.y, target.z, 0.0f);
	DirectX::XMVECTOR u = DirectX::XMVectorSet(up.x, up.y, up.z, 0.0f);
	DirectX::XMMATRIX result = DirectX::XMMatrixLookAtLH(e, t, u);
	return (float4x4 &)result;
}

float4x4 mat_lookat_rh(float3 eyepos, float3 target, float3 up) {
	DirectX::XMVECTOR e = DirectX::XMVectorSet(eyepos.x, eyepos.y, eyepos.z, 0.0f);
	DirectX::XMVECTOR t = DirectX::XMVectorSet(target.x, target.y, target.z, 0.0f);
	DirectX::XMVECTOR u = DirectX::XMVectorSet(up.x, up.y, up.z, 0.0f);
	DirectX::XMMATRIX result = DirectX::XMMatrixLookAtRH(e, t, u);
	return (float4x4 &)result;
}

float4x4 mat_perspective(float fov, float aspect, float zn, float zf) {
	DirectX::XMMATRIX result = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, zn, zf);
	return (float4x4 &)result;
}

float4x4 mat_perspective_rh(float fov, float aspect, float zn, float zf) {
	DirectX::XMMATRIX result = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, zn, zf);
	return (float4x4 &)result;
}

float4x4 mat_ortho(float w, float h, float zn, float zf) {
	DirectX::XMMATRIX result = DirectX::XMMatrixOrthographicLH(w, h, zn, zf);
	return (float4x4 &)result;
}

float4x4 mat_ortho_rh(float w, float h, float zn, float zf) {
	DirectX::XMMATRIX result = DirectX::XMMatrixOrthographicRH(w, h, zn, zf);
	return (float4x4 &)result;
}

float4x4 mat_transform(float3 translate, float4 rotate, float3 scale) {
	float4x4 s, r, t;
	s = mat_scale(scale);
	r = mat_rotate(rotate);
	t = mat_translate(translate);
	return mat_mul(mat_mul(s, r), t);
}

void mat_decompose(float4x4 m, float3 *translate, float4 *rotate, float3 *scale) {
	DirectX::XMVECTOR s, r, t;
	DirectX::XMMatrixDecompose(&s, &r, &t, (DirectX::XMMATRIX &)m);
	scale->x = DirectX::XMVectorGetX(s);
	scale->y = DirectX::XMVectorGetY(s);
	scale->z = DirectX::XMVectorGetZ(s);
	rotate->x = DirectX::XMVectorGetX(r);
	rotate->y = DirectX::XMVectorGetY(r);
	rotate->z = DirectX::XMVectorGetZ(r);
	rotate->w = DirectX::XMVectorGetW(r);
	translate->x = DirectX::XMVectorGetX(t);
	translate->y = DirectX::XMVectorGetY(t);
	translate->z = DirectX::XMVectorGetZ(t);
}

void mat_strafe(float4x4 *transform, float distance) {
	float3 v = scale3f(normalize3f(transform->right), distance);
	transform->position = add3f(transform->position, v);
}

void mat_updown(float4x4 *transform, float distance) {
	float3 v = scale3f(normalize3f(transform->up), distance);
	transform->position = add3f(transform->position, v);
}

void mat_walk(float4x4 *transform, float distance) {
	float3 v = scale3f(normalize3f(transform->forward), distance);
	transform->position = add3f(transform->position, v);
}

void transform_multiply(float3 pos1, float4 rot1, float3 scl1,
			float3 pos2, float4 rot2, float3 scl2,
			float3 *out_pos, float4 *out_rot, float3 *out_scl)
{
	*out_scl = mul3f(scl1, scl2);
	*out_rot = quat_mul(rot1, rot2);
	quat_normalize(*out_rot);
	*out_pos = add3f(pos1, rotate3f(mul3f(pos2, scl1), rot1));
}

void make_frustum_rh(struct frustum *p, float4x4 proj, float4x4 transform)
{
	DirectX::BoundingFrustum &bf = *(DirectX::BoundingFrustum *)p;
	DirectX::BoundingFrustum tmp;
	DirectX::BoundingFrustum::CreateFromMatrix(bf, (DirectX::XMMATRIX &)proj, true);
	bf.Transform(tmp, (DirectX::XMMATRIX &)transform);
	bf = tmp;
}

enum containment_type frustum_check_sphere(struct frustum *p, struct bsphere *bsphere)
{
	DirectX::BoundingFrustum &bf = *(DirectX::BoundingFrustum *)p;
	DirectX::BoundingSphere &sphere = *(DirectX::BoundingSphere *)bsphere;
	DirectX::ContainmentType type = bf.Contains(sphere);
	switch (type) {
	case DirectX::DISJOINT: return containment_disjoint;
	case DirectX::INTERSECTS: return containment_intersects;
	}
	return containment_contains;
}
