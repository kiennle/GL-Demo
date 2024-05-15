#include "json.h"
#include "path.h"
#include "core.h"
#include "glutil.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

static int count_mesh_indices(aiMesh *mesh)
{
	int total = 0;
	int i;

	for (i = 0; i < mesh->mNumFaces; i++)
		total += mesh->mFaces[i].mNumIndices;

	return total;
}

static int count_scene_indices(const aiScene *scene)
{
	int total = 0;
	int i;

	for (i = 0; i < scene->mNumMeshes; i++)
		total += count_mesh_indices(scene->mMeshes[i]);

	return total;
}

static int count_scene_vertices(const aiScene *scene)
{
	int n = 0;
	int i;

	for (i = 0; i < scene->mNumMeshes; i++)
		n += scene->mMeshes[i]->mNumVertices;

	return n;
}

static int mesh_find_max_index(const aiMesh *mesh)
{
	int max = 0;
	int i, j;

	for (i = 0; i < mesh->mNumFaces; i++) {
		aiFace *face = &mesh->mFaces[i];

		for (j = 0; j < face->mNumIndices; j++) {
			int idx = face->mIndices[j];

			max = idx > max ? idx : max;
		}
	}

	return max;
}

static int scene_find_max_index(const aiScene *scene)
{
	int max = 0;
	int i;

	for (i = 0; i < scene->mNumMeshes; i++) {
		int idx = mesh_find_max_index(scene->mMeshes[i]);

		max = idx > max ? idx : max;
	}

	return max;
}

static void parse_vertices(struct gfx_buffer *p, const aiScene *scene)
{
	float3 *it;
	int i, j;

	p->stride = sizeof(it[0]);
	p->count = count_scene_vertices(scene);
	p->cpu_data = malloc(p->count * p->stride);
	it = (float3 *)p->cpu_data;

	for (i = 0; i < scene->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[i];

		for (j = 0; j < mesh->mNumVertices; j++) {
			aiVector3D pos = mesh->mVertices[j];

			it->x = pos.x;
			it->y = pos.y;
			it->z = pos.z;
			it++;
		}
	}
	p->id = gl_create_buffer(p->count * p->stride, p->cpu_data, GL_DYNAMIC_STORAGE_BIT);
}

static void parse_nortans(struct gfx_buffer *p, const aiScene *scene)
{
	struct nortan *it;
	int i, j;

	p->stride = sizeof(it[0]);
	p->count = count_scene_vertices(scene);
	p->cpu_data = malloc(p->count * p->stride);
	it = (struct nortan *)p->cpu_data;

	for (i = 0; i < scene->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[i];

		/* NODE: mesh->mTangents can be NULL in case Assimp failed to calculate or import tangents from mesh files. */
		if (!mesh->HasTangentsAndBitangents()) {
			fprintf(stderr, "ERROR: Assimp failed to import/generate tangent data.");
			abort();
		}

		for (j = 0; j < mesh->mNumVertices; j++) {
			aiVector3D nor = mesh->mNormals[j];
			aiVector3D tan = mesh->mTangents[j];

			it->normal.x = nor.x;
			it->normal.y = nor.y;
			it->normal.z = nor.z;
			it->tangent.x = tan.x;
			it->tangent.y = tan.y;
			it->tangent.z = tan.z;
			it++;
		}
	}
	p->id = gl_create_buffer(p->count * p->stride, p->cpu_data, GL_DYNAMIC_STORAGE_BIT);
}

static void parse_uvs(struct gfx_buffer *p, const aiScene *scene)
{
	float2 *it;
	int i, j;

	p->stride = sizeof(it[0]);
	p->count = count_scene_vertices(scene);
	p->cpu_data = malloc(p->count * p->stride);
	it = (float2 *)p->cpu_data;

	for (i = 0; i < scene->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[i];

		/* NODE: mesh->mTextureCoords can be NULL in case Assimp failed to calculate or import texcoords from mesh files. */
		if (!mesh->HasTextureCoords(0)) {
			fprintf(stderr, "ERROR: Assimp failed to import/generate texcoord data.");
			abort();
		}

		for (j = 0; j < mesh->mNumVertices; j++) {
			aiVector3D uv = mesh->mTextureCoords[0][j];

			it->x = uv.x;
			it->y = uv.y;
			it++;
		}
	}
	p->id = gl_create_buffer(p->count * p->stride, p->cpu_data, GL_DYNAMIC_STORAGE_BIT);
}

static void parse_indices(struct gfx_buffer *p, const aiScene *scene)
{
	uint32_t *it;
	int i, j, k;

	p->stride = sizeof(it[0]);
	p->count = count_scene_indices(scene);
	p->cpu_data = malloc(p->count * p->stride);
	it = (uint32_t *)p->cpu_data;

	for (i = 0; i < scene->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[i];

		for (j = 0; j < mesh->mNumFaces; j++) {
			aiFace *face = &mesh->mFaces[j];

			for (k = 0; k < face->mNumIndices; k++) {
				unsigned int idx = face->mIndices[k];

				*it = idx;
				it++;
			}
		}
	}
	p->id = gl_create_buffer(p->count * p->stride, p->cpu_data, GL_DYNAMIC_STORAGE_BIT);
}

static void parse_mesh_parts(struct mesh *p, const aiScene *scene)
{
	int total_vertices = 0;
	int total_indices = 0;
	int i;

	p->nr_parts = scene->mNumMeshes;
	p->parts = (struct mesh_part *)calloc(p->nr_parts, sizeof(p->parts[0]));

	for (i = 0; i < p->nr_parts; i++) {
		aiMesh *mesh = scene->mMeshes[i];
		struct mesh_part *part = &p->parts[i];
		int index_count = count_mesh_indices(mesh);

		part->vertex_offset = total_vertices;
		part->start_index = total_indices;
		part->index_count = index_count;

		total_vertices += mesh->mNumVertices;
		total_indices += index_count;
	}
}

static void load_mesh_materials(struct mesh *p, rapidjson::Document &doc)
{
	rapidjson::Document::MemberIterator materials = doc.FindMember("materials");
	int i;

	//if (materials == doc.MemberEnd())
	//	return;

	if (doc["materials"].Size() != p->nr_parts) {
		fprintf(stderr, "ERROR: Each mesh part must have a material assigned to it.");
		abort();
	}

	for (i = 0; i < p->nr_parts; i++) {
		char path[256];

		resolve_path(path, p->rnode.name, materials->value[i].GetString());
		p->parts[i].mat = getorload_material(path);
	}
}

static struct mesh *load_mesh_from_json(const char *abs_path)
{
	struct mesh *p;
	rapidjson::Document doc;
	rapidjson::Document::MemberIterator model_scale;
	const aiScene *scene;
	Assimp::Importer importer;
	char model_path[256];
	float global_scale;

	doc = parse_json(abs_path);
	resolve_path(model_path, abs_path, doc["model_path"].GetString());
	model_scale = doc.FindMember("model_scale");
	global_scale = model_scale != doc.MemberEnd() ? model_scale->value.GetFloat() : 1.0f;

	importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
	importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, global_scale);

	scene = importer.ReadFile(model_path,
				  aiProcess_ConvertToLeftHanded |
				  aiProcess_GenSmoothNormals |
				  aiProcess_GenUVCoords |
				  aiProcess_Triangulate |
				  aiProcess_CalcTangentSpace |
				  aiProcess_GlobalScale |
				  aiProcess_ImproveCacheLocality |
				  aiProcess_JoinIdenticalVertices |
				  aiProcess_TransformUVCoords |
				  aiProcess_FindInstances |
				  aiProcess_OptimizeMeshes |
				  aiProcess_FindInvalidData |
				  aiProcess_GenBoundingBoxes |
				  aiProcess_RemoveRedundantMaterials |
				  aiProcess_LimitBoneWeights |
				  aiProcess_ValidateDataStructure |
				  aiProcess_PopulateArmatureData);
	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		return NULL;
	if (!scene->HasMeshes()) {
		fprintf(stderr, "ERROR: Scene has no meshes.");
		abort();
	}

	p = (struct mesh *)calloc(1, sizeof(*p));
	if (!p)
		return NULL;
	rnode_init(&p->rnode, abs_path);
	rnode_add(&p->rnode);
	parse_vertices(&p->vertices, scene);
	parse_nortans(&p->nortans, scene);
	parse_uvs(&p->uvs, scene);
	parse_indices(&p->indices, scene);
	parse_mesh_parts(p, scene);
	load_mesh_materials(p, doc);
	return p;
}

struct mesh *getorload_mesh(const char *abs_path)
{
	struct mesh *p;

	p = lookup_resource(abs_path, struct mesh, rnode);
	if (!p)
		p = load_mesh_from_json(abs_path);
	return p;
}
