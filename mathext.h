#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define PI 3.141592654f

typedef struct float2 {
	float x, y;
} float2;

typedef struct float3 {
	float x, y, z;
} float3;

typedef struct float4 {
	union {
		struct {
			float x, y, z, w;
		};
		float3 xyz;
	};
} float4, quat, quatern;

typedef struct byte4 {
	uint8_t x, y, z, w;
} byte4;

typedef struct half {
	uint16_t packed;
} half;

typedef struct half2 {
	half x, y;
} half2;

typedef struct half4 {
	half x, y, z, w;
} half4;

/* Similar to XMDECN4 in DirectXMath. */
typedef struct decn4 {
	uint32_t packed;
} decn4;

/* Similar to XMUDECN4 in DirectXMath. */
typedef struct udecn4 {
	uint32_t packed;
} udecn4;

typedef struct float4x4 {
	union {
#ifdef __cplusplus
		alignas(16) float data[16];
#else
		_Alignas(16) float data[16];
#endif
		struct {
			float _00, _01, _02, _03;
			float _10, _11, _12, _13;
			float _20, _21, _22, _23;
			float _30, _31, _32, _33;
		};
		struct {
			float3 right;
			float _padding0; /* 0.0f */
			float3 up;
			float _padding1; /* 0.0f */
			float3 forward;
			float _padding2; /* 0.0f */
			float3 position;
			float _padding3; /* 1.0f */
		};
	};
} float4x4;

typedef struct float4x3 {
	union {
		float data[12];
		struct {
			float _00, _01, _02;
			float _10, _11, _12;
			float _20, _21, _22;
			float _30, _31, _32;
		};
		struct {
			float3 right;
			float3 up;
			float3 forward;
			float3 position;
		};
	};
} float4x3;

struct bsphere {
	float3 center;
	float radius;
};

struct bbox {
	float3 center, extents;
};

struct plane {
	float3 normal; /* normal of the plane */
	float distance; /* the distance the plane is from the origin */
};

enum containment_type {
	containment_disjoint,
	containment_intersects,
	containment_contains
};

/* Copied from struct BoundingFrustum in DirectXCollision.h */
struct frustum {
	float3 origin;               /* origin of the frustum (and projection) */
	float4 orientation;          /* quaternion representing rotation */
	float right_slope;           /* positive x (x/z) */
	float left_slope;            /* negative x */
	float top_slope;             /* positive y (y/z) */
	float bottom_slope;          /* negative y */
	float near_plane, far_plane; /* z of the near plane and far plane */
};

//struct frustum {
//	union {
//		struct plane planes[6];
//		struct {
//			struct plane left;
//			struct plane right;
//			struct plane bottom;
//			struct plane top;
//			struct plane near;
//			struct plane far;
//		};
//	};
//};

static const float4x4 mat_zero = {
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
};

static const float4x4 mat_identity = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

#define MAT_INIT_IDENTITY {	\
	1.0f, 0.0f, 0.0f, 0.0f,	\
	0.0f, 1.0f, 0.0f, 0.0f,	\
	0.0f, 0.0f, 1.0f, 0.0f,	\
	0.0f, 0.0f, 0.0f, 1.0f	\
}

static const float3 float3_zero = {0.0f, 0.0f, 0.0f};
static const float3 float3_one = {1.0f, 1.0f, 1.0f};
static const float3 float3_right = {1.0f, 0.0f, 0.0f};
static const float3 float3_left = {-1.0f, 0.0f, 0.0f};
static const float3 float3_up = {0.0f, 1.0f, 0.0f};
static const float3 float3_down = {0.0f, -1.0f, 0.0f};
static const float3 float3_forward = {0.0f, 0.0f, 1.0f};
static const float3 float3_backward = {0.0f, 0.0f, -1.0f};

static const float4 quat_identity = {0.0f, 0.0f, 0.0f, 1.0f};

#ifdef __cplusplus
extern "C" {
#endif

static inline float2 float2_from_xy(float x, float y)
{
	float2 v = {x, y};

	return v;
}

static inline float3 float3_from_xyz(float x, float y, float z)
{
	float3 v = {x, y, z};

	return v;
}

static inline float4 float4_from_xyzw(float x, float y, float z, float w)
{
	float4 v = {x, y, z, w};

	return v;
}

static inline int imin(int a, int b) { return a < b ? a : b; }
static inline int imax(int a, int b) { return a > b ? a : b; }

#define ZERO_TOLERANCE (1e-6f)
bool equalf(float a, float b);

float hermitef(float pos0, float tan0, float pos1, float tan1, float t);
float2 hermite2f(float2 pos0, float2 tan0, float2 pos1, float2 tan1, float t);
float3 hermite3f(float3 pos0, float3 tan0, float3 pos1, float3 tan1, float t);
float4 hermite4f(float4 pos0, float4 tan0, float4 pos1, float4 tan1, float t);

half pack_half(float x);
half2 pack_half2(float x, float y);
int64_t pack_snorm(double x, int precision); /* @x in range[-1.0, 1.0], precision in number of bits */
uint64_t pack_unorm(double x, int precision); /* @x in range[0.0, 1.0], precision in number of bits */
decn4 pack_decn4(float x, float y, float z, float w); /* @x, @y, @z, @w in range[-1.0, 1.0] */
udecn4 pack_udecn4(float x, float y, float z, float w); /* @x, @y, @z, @w in range[0.0, 1.0] */
double unpack_snorm(int64_t x, int precision); /* return value in range[-1.0, 1.0], precision in number of bits */
double unpack_unorm(uint64_t x, int precision); /* return value in range[0.0, 1.0], precision in number of bits */
float2 add2f(float2 lhs, float2 rhs);
float2 sub2f(float2 lhs, float2 rhs);
float2 mul2f(float2 lhs, float2 rhs);
float3 neg3f(float3 v);
float3 add3f(float3 lhs, float3 rhs);
float3 sub3f(float3 lhs, float3 rhs);
float3 mul3f(float3 lhs, float3 rhs);
float3 scale3f(float3 lhs, float rhs);
float3 div3f(float3 lhs, float3 rhs);
float length3f(float3 v);
float lengthsq3f(float3 v);
float3 normalize3f(float3 v);
float3 cross3f(float3 a, float3 b);
float3 rotate3f(float3 v, float4 quat);
float clamp(float value, float min, float max);
float lerp(float a, float b, float t);
float2 lerp2f(float2 a, float2 b, float t);
float3 lerp3f(float3 a, float3 b, float t);
float4 quat_slerp(float4 a, float4 b, float t);
float4 quat_rotateaxis(float3 axis, float radians); /* NOTE: If Axis is a normalized vector, it is faster to use XMQuaternionRotationNormal */
float4 quat_mul(float4 lhs, float4 rhs);
float4 quat_normalize(float4 quat);
float4 quat_inverse(float4 quat);
float3 quat_right(float4 quat);
float3 quat_left(float4 quat);
float3 quat_up(float4 quat);
float3 quat_down(float4 quat);
float3 quat_forward(float4 quat);
float3 quat_backward(float4 quat);
float4x4 mat_from_floats(float *elems);
float4x4 mat_mul(float4x4 lhs, float4x4 rhs);
float3 mat_transform_normal(float4x4 m, float3 normal);
float4x4 mat_translate(float3 v);
float4x4 mat_rotate(float4 quat);
float4x4 mat_rotateaxis(float3 axis, float radians);
float4x4 mat_rotatex(float radians);
float4x4 mat_rotatey(float radians);
float4x4 mat_scale(float3 v);
float4x4 mat_transpose(float4x4 m);
float4x4 mat_inverse(float4x4 m);
float4x4 mat_lookto(float3 eyepos, float3 forward, float3 up);
float4x4 mat_lookto_rh(float3 eyepos, float3 forward, float3 up);
float4x4 mat_lookat(float3 eyepos, float3 target, float3 up);
float4x4 mat_lookat_rh(float3 eyepos, float3 target, float3 up);
float4x4 mat_perspective(float fov, float aspect, float zn, float zf);
float4x4 mat_perspective_rh(float fov, float aspect, float zn, float zf);
float4x4 mat_ortho(float w, float h, float zn, float zf);
float4x4 mat_ortho_rh(float w, float h, float zn, float zf);
float4x4 mat_transform(float3 translate, float4 rotate, float3 scale);
void mat_decompose(float4x4 m, float3 *translate, float4 *rotate, float3 *scale);
void mat_strafe(float4x4 *transform, float distance);
void mat_updown(float4x4 *transform, float distance);
void mat_walk(float4x4 *transform, float distance);

void transform_multiply(float3 pos1, float4 rot1, float3 scl1,
			float3 pos2, float4 rot2, float3 scl2,
			float3 *out_pos, float4 *out_rot, float3 *out_scl);

static inline float bbox_radius(struct bbox bbox) { return length3f(bbox.extents); }

enum containment_type frustum_check_sphere(struct frustum *p, struct bsphere *bsphere);
void make_frustum_rh(struct frustum *p, float4x4 proj, float4x4 transform);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

inline float3 operator-(float3 lhs) {
	return scale3f(lhs, -1.0f);
}

inline float3 operator+(float3 lhs, float3 rhs) {
	return add3f(lhs, rhs);
}

inline float3 &operator+=(float3 &lhs, float3 rhs) {
	lhs = add3f(lhs, rhs);
	return lhs;
}

inline float3 operator-(float3 lhs, float3 rhs) {
	return sub3f(lhs, rhs);
}

inline float3 &operator-=(float3 &lhs, float3 rhs) {
	lhs = sub3f(lhs, rhs);
	return lhs;
}

inline float3 operator*(float3 lhs, float3 rhs) {
	return mul3f(lhs, rhs);
}

inline float3 &operator*=(float3 &lhs, float3 rhs) {
	lhs = mul3f(lhs, rhs);
	return lhs;
}

inline float3 operator*(float3 lhs, float rhs) {
	return scale3f(lhs, rhs);
}

inline float3 &operator*=(float3 &lhs, float rhs) {
	lhs = scale3f(lhs, rhs);
	return lhs;
}

inline float3 operator/(float3 lhs, float3 rhs) {
	return div3f(lhs, rhs);
}

inline float3 &operator/=(float3 &lhs, float3 rhs) {
	lhs = div3f(lhs, rhs);
	return lhs;
}

inline float3 operator/(float3 lhs, float rhs) {
	return scale3f(lhs, 1.0f / rhs);
}

inline float3 &operator/(float3 &lhs, float rhs) {
	lhs = scale3f(lhs, 1.0f / rhs);
	return lhs;
}

inline float4x4 operator*(float4x4 lhs, float4x4 rhs) {
	return mat_mul(lhs, rhs);
}

inline float4x4 &operator*=(float4x4 &lhs, float4x4 rhs) {
	lhs = mat_mul(lhs, rhs);
	return lhs;
}

#endif
