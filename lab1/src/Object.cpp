#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc

#include "Object.h"
#include <iostream>
#include <cstdio>

Object::Object(const char *path, const char *base_path) : path(path) {
  std::string err;
  std::string warn;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path, base_path);

  if (!warn.empty()) {
    std::cout << warn << std::endl;
  }

  if (!err.empty()) { // `err` may contain warning message.
    std::cerr << err << std::endl;
  }

  if (shapes.size() > 1) {
    std::cout << "MORE THAN 1 SHAPE!" << std::endl;
  }

  calculateVertexNormals();

  std::cout << "Successfully loaded " << path << "!" << std::endl;
}

void Object::calculateVertexNormals() {
  attrib.normals = std::vector<tinyobj::real_t>(attrib.vertices.size(), 0.0f);

  for (size_t i = 0; i < shapes.size(); i++) {
    float nx, ny, nz = 0.0f; // normal for current triangle
    float vx1, vx2, vx3 = 0.f; // vertex 1
    float vy1, vy2, vy3 = 0.f; // vertex 2
    float vz1, vz2, vz3 = 0.f; // vertex 3

    const tinyobj::shape_t& shape = shapes[i];
    std::vector<int> fv_indices;
    const unsigned char num_face_vertices = 3;
    for (size_t v = 0; v < shape.mesh.indices.size(); v++) {
      const int vi = shape.mesh.indices[v].vertex_index;

      fv_indices.push_back(vi);

      float x = attrib.vertices[num_face_vertices*vi];
      float y = attrib.vertices[num_face_vertices*vi+1];
      float z = attrib.vertices[num_face_vertices*vi+2];
      switch(v % 3) {
      case 0:
	// Defining first point in triangle
	vx1 = x;
	vy1 = y;
	vz1 = z;
	break;
      case 1:
	// Defining second point in triangle
	vx2 = x;
	vy2 = y;
	vz2 = z;
	break;
      case 2:
	// Defining third point in a triangle
	vx3 = x;
	vy3 = y;
	vz3 = z;
	float qx, qy, qz, px, py, pz;
	// Calculate q vector
	qx = vx2 - vx1;
	qy = vy2 - vy1;
	qz = vz2 - vz1;
	// Calculate p vector
	px = vx3 - vx1;
	py = vy3 - vy1;
	pz = vz3 - vz1;
	// Calculate normal
	nx = py * qz - pz * qy;
	ny = pz * qx - px * qz;
	nz = px * qy - py * qx;
	for (auto vi : fv_indices) {
	  // add normal to index
	  attrib.normals[num_face_vertices*vi] += nx;
	  attrib.normals[num_face_vertices*vi+1] += ny;
	  attrib.normals[num_face_vertices*vi+2] += nz;
	}
	fv_indices.clear();
	break;
      }
    }
  }

  // normalize normals :)
  for (int vi = 0; vi < attrib.vertices.size() / 3; vi++) {
    float x = attrib.normals[3*vi];
    float y = attrib.normals[3*vi+1];
    float z = attrib.normals[3*vi+2];

    const float len = sqrt(x*x + y*y + z*z);

    if (len > 0.0f) {
      attrib.normals[3*vi] /= len;
      attrib.normals[3*vi+1] /= len;
      attrib.normals[3*vi+2] /= len;
    }
  }
}

static void CalcNormal(float N[3], float v0[3], float v1[3], float v2[3]) {
  float v10[3];
  v10[0] = v1[0] - v0[0];
  v10[1] = v1[1] - v0[1];
  v10[2] = v1[2] - v0[2];

  float v20[3];
  v20[0] = v2[0] - v0[0];
  v20[1] = v2[1] - v0[1];
  v20[2] = v2[2] - v0[2];

  N[0] = v20[1] * v10[2] - v20[2] * v10[1];
  N[1] = v20[2] * v10[0] - v20[0] * v10[2];
  N[2] = v20[0] * v10[1] - v20[1] * v10[0];

  float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
  if (len2 > 0.0f) {
    float len = sqrtf(len2);

    N[0] /= len;
    N[1] /= len;
    N[2] /= len;
  }
}

void Object::printInfo() const {
  std::cout << "Info for " << path << std::endl;
  std::cout << "# of vertices  : " << (attrib.vertices.size() / 3) << std::endl;
  std::cout << "# of normals   : " << (attrib.normals.size() / 3) << std::endl;
  std::cout << "# of texcoords : " << (attrib.texcoords.size() / 2)
            << std::endl;

  std::cout << "# of shapes    : " << shapes.size() << std::endl;
  std::cout << "# of materials : " << materials.size() << std::endl;

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].mesh.indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));
    printf("Size of shape[%ld].path.indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].path.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.smoothing_group_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);
      printf("  face[%ld].smoothing_group_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.smoothing_group_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
					       shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("    bump_multiplier = %f\n", static_cast<const double>(materials[i].bump_texopt.bump_multiplier));
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", static_cast<const double>(materials[i].roughness));
    printf("  material.Pm     = %f\n", static_cast<const double>(materials[i].metallic));
    printf("  material.Ps     = %f\n", static_cast<const double>(materials[i].sheen));
    printf("  material.Pc     = %f\n", static_cast<const double>(materials[i].clearcoat_thickness));
    printf("  material.Pcr    = %f\n", static_cast<const double>(materials[i].clearcoat_thickness));
    printf("  material.aniso  = %f\n", static_cast<const double>(materials[i].anisotropy));
    printf("  material.anisor = %f\n", static_cast<const double>(materials[i].anisotropy_rotation));
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
							  materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
							     materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}
