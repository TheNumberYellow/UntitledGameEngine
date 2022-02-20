//#include "Collisions.hpp"
//
//RayCastHit Collisions::RayCast(Ray ray, Triangle triangle)
//{
//    // TODO: Become better at math and understand this better :)
//
//    RayCastHit result;
//    result.hit = false;
//
//    Vec3f vecAtoB = triangle.b - triangle.a;
//    Vec3f vecAtoC = triangle.c - triangle.a;
//
//    Vec3f pVec = Math::cross(ray.direction, vecAtoC);
//    float det = Math::dot(vecAtoB, pVec);
//
//    if (abs(det) < 0.00001f)
//    {
//        return result;
//    }
//
//    float invDet = 1.0f / det;
//    Vec3f tVec = ray.origin - triangle.a;
//
//    float u = Math::dot(tVec, pVec) * invDet;
//
//    if (u < 0 || u > 1) return result;
//
//    Vec3f qVec = Math::cross(tVec, vecAtoB);
//    float v = Math::dot(ray.direction, qVec) * invDet;
//
//    if (v < 0 || u + v > 1.0f) return result;
//
//    float t = Math::dot(vecAtoC, qVec) * invDet;
//
//    if (t > 0.0f)
//    {
//        result.hit = true;
//        result.hitDistance = t;
//        result.hitPoint = ray.origin + (ray.direction * t);
//    }
//
//    return result;
//}
//
//RayCastHit Collisions::RayCast(Ray ray, CollisionMesh collisionMesh)
//{
//    RayCastHit result;
//    result.hit = false;
//
//    for (int i = 0; i < collisionMesh.m_Triangles.size(); ++i)
//    {
//        RayCastHit newHit = RayCast(ray, collisionMesh.m_Triangles[i]);
//        if (newHit.hit && newHit.hitDistance < result.hitDistance)
//        {
//            result = newHit;
//        }
//    }
//
//    return result;
//}
//
//RayCastHit Collisions::RayCast(Ray ray, Triangle triangle, Mat4x4f transform)
//{
//    triangle = TransformTriangle(triangle, transform);
//    return RayCast(ray, triangle);
//}
//
//RayCastHit Collisions::RayCast(Ray ray, CollisionMesh collisionMesh, Mat4x4f transform)
//{
//    RayCastHit result;
//    result.hit = false;
//
//    for (int i = 0; i < collisionMesh.m_Triangles.size(); ++i)
//    {
//        RayCastHit newHit = RayCast(ray, collisionMesh.m_Triangles[i], transform);
//        if (newHit.hit && newHit.hitDistance < result.hitDistance)
//        {
//            result = newHit;
//        }
//    }
//    return result;
//}
//
//RayCastHit Collisions::RayCast(Ray ray, Plane plane)
//{
//    RayCastHit result;
//    result.hit = false;
//
//    float denom = Math::dot(plane.normal, ray.direction);
//    if (abs(denom) > 0.0001f)
//    {
//        float t = Math::dot((plane.center - ray.origin), plane.normal) / denom;
//        if (t >= 0)
//        {
//            result.hit = true;
//            result.hitDistance = t;
//            result.hitPoint = ray.origin + (ray.direction * t);
//        }
//    }
//
//    return result;
//}
//
//RayCastHit Collisions::FirstHit(RayCastHit left, RayCastHit right)
//{
//    return left.hitDistance < right.hitDistance ? left : right;
//}
//
//CollisionMesh Collisions::GenerateCollisionGeometryFromMesh(Mesh_ID mesh, Renderer* renderer)
//{
//    std::vector<unsigned int*> meshElements = renderer->MapMeshElements(mesh);
//    std::vector<Vertex*> meshVertices = renderer->MapMeshVertices(mesh);
//
//    std::vector<Triangle> resultTriangles;
//
//    for (int i = 0; i < meshElements.size(); i += 3)
//    {
//        Triangle newTriangle;
//        newTriangle.a = meshVertices[*meshElements[i]]->position;
//        newTriangle.b = meshVertices[*meshElements[(size_t)i + 1]]->position;
//        newTriangle.c = meshVertices[*meshElements[(size_t)i + 2]]->position;
//
//        resultTriangles.push_back(newTriangle);
//    }
//    renderer->UnmapMeshElements(mesh);
//    renderer->UnmapMeshVertices(mesh);
//
//    CollisionMesh collisionMesh;
//    collisionMesh.m_Triangles = resultTriangles;
//    return collisionMesh;
//}
//
//Triangle Collisions::TransformTriangle(Triangle triangle, Mat4x4f transform)
//{
//    Triangle result;
//    result.a = Math::mult(triangle.a, transform);
//    result.b = Math::mult(triangle.b, transform);
//    result.c = Math::mult(triangle.c, transform);
//
//    return result;
//}
