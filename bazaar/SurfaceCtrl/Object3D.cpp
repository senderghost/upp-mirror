#include "Object3D.h"
#include <Surface/Surface.h>

namespace Upp{
	
#define IMAGECLASS TexturesImg
#define IMAGEFILE <SurfaceCtrl/textures.iml>
#include <Draw/iml.h>
	
	
int Object3D::GlobalID = 0;

Object3D& Object3D::operator=(Object3D&& obj){
	meshes = pick(obj.meshes);
	
	drawType = obj.drawType;
	lineColor = obj.lineColor;
	lineOpacity = obj.lineOpacity;
	lineWidth = obj.lineWidth;
	normalColor = obj.normalColor;
	normalOpacity = obj.normalOpacity;
	normalLenght = obj.normalLenght;
	showMesh = obj.showMesh;
	showMeshLine = obj.showMeshLine;
	showMeshNormal = obj.showMeshNormal;
	showLight = obj.showLight;
	boundingBox = obj.boundingBox;
	showBoundingBox = obj.showBoundingBox;
	transform = obj.transform;
	obj.moved = true;
	obj.loaded = false;
	return *this;
}

Object3D& Object3D::operator=(const Object3D& obj){
	meshes.Append(obj.meshes);
	
	drawType = obj.drawType;
	lineColor = obj.lineColor;
	lineOpacity = obj.lineOpacity;
	lineWidth = obj.lineWidth;
	normalColor = obj.normalColor;
	normalOpacity = obj.normalOpacity;
	normalLenght = obj.normalLenght;
	showMesh = obj.showMesh;
	showMeshLine = obj.showMeshLine;
	showMeshNormal = obj.showMeshNormal;
	showLight = obj.showLight;
	boundingBox = obj.boundingBox;
	showBoundingBox = obj.showBoundingBox;
	transform = obj.transform;
	
	return *this;
}

Object3D::~Object3D(){
	for(const Texture& t  : textures){
		glDeleteTextures(1,&(t.id));
	}
	Clear();
}
/*
bool Object3D::LoadObj(const String& FileObj){
	Color color = Gray();
	if(OBJLoader::LoadOBJ(FileObj,meshes)){
		float r = color.GetR()/255.0f;
		float g = color.GetG()/255.0f;
		float b = color.GetB()/255.0f;
		for(Mesh& m : meshes){
			int cpt = m.GetVertices().GetCount() -1;
			for(int e = 0; e < cpt ; e = e + 3){
				m.GetColors() << r << g << b ;
			}
		}
		Load();
		return true;
	}
	return false;
}

bool Object3D::LoadStl(const String& StlFile, Color color){
	Mesh& mesh = meshes.Create();
	glm::vec3 col( color.GetR()/255.0f, color.GetG()/255.0f, color.GetB()/255.0f);
	if(STLLoader::LoadSTL(StlFile,mesh.GetVertices(),mesh.GetNormals())){
		int cpt = mesh.GetVertices().GetCount() -1;
		for(int e = 0; e < cpt ; e = e + 3){
			mesh.GetColors() << col.x << col.y << col.z ;
		}
		Load();
		return true;
	}
	return false;
}*/


/**
	LoadModel load multiple kind of object file using Assimp lib, you can specify some custom
	load routine by editing pFlags, if pFlags = 0 then SurfaceCtrl default behavior about
	assimp will be used
**/
Object3D& Object3D::LoadModel(const String& Filename, Color color, int alpha , unsigned int pFlags ){
	Clear();
    if( pFlags == 0){
		pFlags = aiProcess_JoinIdenticalVertices |// join identical vertices/ optimize indexing
		aiProcess_ValidateDataStructure |	// perform a full validation of the loader's output
		aiProcess_ImproveCacheLocality |	// improve the cache locality of the output vertices
		aiProcess_RemoveRedundantMaterials |// remove redundant materials
		aiProcess_GenUVCoords |				// convert spherical, cylindrical, box and planar mapping to proper UVs
		aiProcess_TransformUVCoords |		// pre-process UV transformations (scaling, translation ...)
		aiProcess_FindInstances |			// search for instanced meshes and remove them by references to one master
		aiProcess_LimitBoneWeights |		// limit bone weights to 4 per vertex
		aiProcess_OptimizeMeshes |			// join small meshes, if possible;
		aiProcess_PreTransformVertices |
		aiProcess_GenSmoothNormals |		// generate smooth normal vectors if not existing
		aiProcess_SplitLargeMeshes |		// split large, unrenderable meshes into sub-meshes
		aiProcess_Triangulate |				// triangulate polygons with more than 3 edges
		aiProcess_ConvertToLeftHanded |		// convert everything to D3D left handed space
		aiProcess_SortByPType;
    }
    
    Assimp::Importer Importer;
	const aiScene* pScene = Importer.ReadFile(Filename.ToStd().c_str(),pFlags);
	if(pScene){
        if(InitFromScene(pScene, Filename)){
            glm::vec4 col(color.GetR()/255.0f, color.GetG()/255.0f, color.GetB()/255.0f, alpha/255.0f);
            for(Mesh& m : meshes){
				Vector<float>& colors = m.GetColors();
				for(unsigned int e = 0; e < (m.GetVertices().GetCount()/3) ; e++){
					colors << col.x << col.y << col.z << col.w;
				}
            }
        }else{
			throw Exc(Format("Error initing data from '%s'\n", Filename));
        }
    }else {
        throw Exc(Format("Error parsing '%s': '%s'\n", Filename, String(Importer.GetErrorString())));
    }
    return *this;
}
int Object3D::LoadTexture(const Image& img , const String& name, int indiceWhereInsert){
	Image image = img;
	String trueName = name;
	if(IsNull(image)){
		image = TexturesImg::empty();
		trueName = AsString(image.GetSerialId());
	}
	for(int e = 0; e < textures.GetCount(); e++){
		if(textures[e].name.IsEqual(trueName)){
			return e;
		}
	}
	
	Size size = image.GetSize();
	int indice;
	Texture* t = nullptr;
	if(indiceWhereInsert == -1){
		indice = textures.GetCount();
		t = &(textures.Add());
	}else{
		if(indiceWhereInsert < textures.GetCount()){
			indice = indiceWhereInsert;
			t = &(textures[indiceWhereInsert]);
		}else{
			indice = textures.GetCount();
			t = &(textures.Add());
		}
	}
	
	t->name = trueName;
	glGenTextures(1, &(t->id));
	glBindTexture(GL_TEXTURE_2D, t->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.cy , size.cx, 0, GL_BGRA, GL_UNSIGNED_BYTE, ~image);
	glGenerateMipmap(GL_TEXTURE_2D);

	return indice;
}
/*
	Assimp loading function
*/
bool Object3D::InitFromScene(const aiScene* pScene, const String& Filename){
	meshes.AddN(pScene->mNumMeshes);
	textures.AddN(pScene->mNumMaterials);
	//Add Texture vector init here
	
	// Initialise les maillages de la scène, un par un
    for (unsigned int i = 0 ; i < meshes.GetCount() ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i, paiMesh);
    }
	return InitMaterials(pScene, Filename);
}
void Object3D::InitMesh(unsigned int Index, const aiMesh* paiMesh){
	//For texture / material data
	meshes[Index].SetTextureIndice(paiMesh->mMaterialIndex);
	
	Vector<float>& vertices = meshes[Index].GetVertices();
	Vector<float>& normals = meshes[Index].GetNormals();
	Vector<float>& textCoords = meshes[Index].GetTexCoords();
	
	struct Vertex : public Moveable<Vertex>{
		float pos[3];
		float normals[3];
		float texCoords[2];
		
		Vertex(float x, float y, float z, float n1 , float n2, float n3, float tc1 , float tc2){
			pos[0] = x; pos[1] = y; pos[2] = z;
			normals[0] = n1; normals[1] = n2; normals[2] = n3;
			texCoords[0] = tc1; texCoords[1] = tc2;
		}
	};
	
	Vector<Vertex> dummyVertices;
	Vector<float> dummyIndices;
	
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++) {
        const aiVector3D* pPos = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal = (&(paiMesh->mNormals[i]))? &(paiMesh->mNormals[i]) : &Zero3D;
        
        glm::vec3 norm(pNormal->x , pNormal->y , pNormal->z);
        norm = glm::abs(norm) * -1.0f;
        
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;
		
		//dummyVertices << Vertex( pPos->x , pPos->y , pPos->z , pNormal->x , pNormal->y , pNormal->z , pTexCoord->x , pTexCoord->y);
		dummyVertices << Vertex( pPos->x , pPos->y , pPos->z , norm.x , norm.y , norm.z , pTexCoord->x , pTexCoord->y);
    }
	
	for (unsigned int i = 0 ; i < paiMesh->mNumFaces ; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        ASSERT_(Face.mNumIndices == 3, "Face in Assimp strucure do not contain 3 points !");
        dummyIndices << Face.mIndices[0] << Face.mIndices[1] << Face.mIndices[2];
    }
	for(unsigned int i = 0 ; i < dummyIndices.GetCount(); i++){
		Vertex& vertex = dummyVertices[dummyIndices[i]];
		vertices <<  vertex.pos[0] << vertex.pos[1] << vertex.pos[2];
		normals << vertex.normals[0] << vertex.normals[1] << vertex.normals[2];
		textCoords << vertex.texCoords[0] << vertex.texCoords[1];
	}
}
bool Object3D::InitMaterials(const aiScene* pScene, const String& Filename){
	bool Ret = false;
	for (unsigned int i = 0 ; i < pScene->mNumMaterials ; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];
        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0){
            aiString Path;
            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                String FullPath =AppendFileName(GetFileFolder(Filename), String(Path.data));
                InsertTexture(FullPath,i);
            }
        }
    }
   return true;
}

Object3D& Object3D::LoadSurface(Surface& surface, Color color, int alpha){
	Mesh& mesh = meshes.Create();
	glm::vec4 col( color.GetR()/255.0f, color.GetG()/255.0f, color.GetB()/255.0f , alpha /255.0f);
	
	auto& vertices = mesh.GetVertices();
	auto& normals = mesh.GetNormals();
	auto& colors = mesh.GetColors();
	
	int indice =0;
	for (int ip = 0; ip < surface.panels.GetCount(); ++ip) {
		const Panel &panel = surface.panels[ip];
		//ADDING :
		if(panel.IsTriangle()){
			for(int e = 0; e < 3; e++){
				Point3D p = surface.nodes[panel.id[e]];
				vertices.Add(p.x);
				vertices.Add(p.y);
				vertices.Add(p.z);
				normals.Add(panel.normalPaint.x);
				normals.Add(panel.normalPaint.y);
				normals.Add(panel.normalPaint.z);
				colors.Add(col.x);
				colors.Add(col.y);
				colors.Add(col.z);
				colors.Add(col.w);
			}
		}else{//A quad is just 2 triangles
			Point3D SecondTriangle[3];
			for(int e = 0; e < 3; e++){
				Point3D p = surface.nodes[panel.id[e]];
				if(e == 0) SecondTriangle[2] = p;
				if(e == 2) SecondTriangle[0] = p;
				vertices.Add(p.x);
				vertices.Add(p.y);
				vertices.Add(p.z);
				
				normals.Add(panel.normalPaint.x);
				normals.Add(panel.normalPaint.y);
				normals.Add(panel.normalPaint.z);
				
				colors.Add(col.x);
				colors.Add(col.y);
				colors.Add(col.z);
				colors.Add(col.w);
			}
			SecondTriangle[1]  = surface.nodes[panel.id[3]];
			for(int e = 0; e < 3; e++){
				Point3D p = SecondTriangle[e];
				vertices.Add(p.x);
				vertices.Add(p.y);
				vertices.Add(p.z);
				
				normals.Add(panel.normalPaint.x);
				normals.Add(panel.normalPaint.y);
				normals.Add(panel.normalPaint.z);
				
				colors.Add(col.x);
				colors.Add(col.y);
				colors.Add(col.z);
				colors.Add(col.w);
			}
		}
	}
	return *this;
}

Surface Object3D::GetSurface(){ //return a surface Objectd
	Surface surf;
	for(Mesh& m : meshes){
		auto& vertices = m.GetVertices();
		auto& normals = m.GetNormals();
		for(int e = 0; e < vertices.GetCount(); e = e+9){
			Panel &panel = surf.panels.Add();
			
			panel.normalPaint.x = double(normals[e]);
			panel.normalPaint.y = double(normals[e+1]);
			panel.normalPaint.z = double(normals[e+2]);
			
			Point3D &node0 = surf.nodes.Add();
			node0.x = double(vertices[e]);
			node0.y = double(vertices[e+1]);
			node0.z = double(vertices[e+2]);
			panel.id[0] = surf.nodes.GetCount()-1;
			
			Point3D &node1 = surf.nodes.Add();
			node1.x = double(vertices[e+3]);
			node1.y = double(vertices[e+4]);
			node1.z = double(vertices[e+5]);
			panel.id[1] = surf.nodes.GetCount()-1;
			
			Point3D &node2 = surf.nodes.Add();
			node2.x = double(vertices[e+6]);
			node2.y = double(vertices[e+7]);
			node2.z = double(vertices[e+8]);
			panel.id[2] = surf.nodes.GetCount()-1;
			panel.id[3] = panel.id[0];
		}
	}
	return pick(surf);
}

//Check if texture provided is already load, then return the iterator of the texture
//object
int Object3D::InsertTexture(const String& filename, int indice)noexcept{ //insert texture in object3D
	Image m = StreamRaster::LoadFileAny(filename);
	return InsertTexture(m,indice);
}
int Object3D::InsertTexture(const Image& m, int indice)noexcept{ //insert texture in object3D
	return LoadTexture(m,AsString(m.GetSerialId()),indice);
}
int Object3D::InsertTexture(const TexturesMaterial& tm, int indice)noexcept{ //Insert one of SurfaceCtrl provided texture
	Image m;
	switch(tm){
		case TM_BRICK:
			m = TexturesImg::brick();
		break;
		case TM_METAL:
			m = TexturesImg::metal();
		break;
		case TM_STONE:
			m = TexturesImg::stone();
		break;
		case TM_WATER:
			m = TexturesImg::water();
		break;
		case TM_WOOD:
			m = TexturesImg::wood();
		break;
	}
	return InsertTexture(m,indice);
}
Object3D& Object3D::AttachTexture(int numTexture,int MeshNo, int count){//Attach a texture to the range of mes
	int textNo = numTexture;
	if(textNo > textures.GetCount()) textNo = 0;
	if(MeshNo < meshes.GetCount() && count > 0 &&  (MeshNo + count) <= meshes.GetCount()){
		for(int e = MeshNo; e < (MeshNo + count); e ++){
			meshes[e].SetTextureIndice(textNo);
		}
	}
	return *this;
}
Object3D& Object3D::GenerateTextureCoordinate(int MeshNo, int count , bool CustomTextureCoordinate, const Vector<float>& tc){// Generate new Texture coordinate for the selected range of mesh
	if(MeshNo < meshes.GetCount() && count > 0 && (MeshNo + count) <= meshes.GetCount()){
		for(int e = MeshNo; e < (MeshNo + count); e++){
			Mesh& m  = meshes[e];
			Vector<float>& tex = m.GetTexCoords();
			tex.Clear();
			int triangleCpt = (m.GetVertices().GetCount()/3) -1;
			for(int i = 0; i < triangleCpt; i++){
				if(CustomTextureCoordinate) tex.Append(tc);
				else tex << 0.0f << 0.0f << 1.0f << 0.0f << 0.5f << 1.0f;
			}
		}
	}
	return *this;
}

bool Object3D::Init(){ //Load all data in graphic memory It's called automaticly by using Load function, but you must call it if you set manually all data
	for(Mesh& m : meshes){
		m.Init();
	}
	CreateBoundingBox();
	loaded = true;
	return true;
}
Object3D& Object3D::Clear(){
	if(!moved){
		for(Mesh& m : meshes){
			m.Clear(true);
		}
		RemoveBoundingBox();
		loaded = false;
	}
	return *this;
}
Object3D& Object3D::Reload(){
	if(!moved){
		for(Mesh& m : meshes){
			m.Clear(false);
		}
		RemoveBoundingBox();
		loaded = false;
	}
	Init();
	return *this;
}


void Object3D::CreateBoundingBox(){
	float minX,minY,minZ;
	minX = minY = minZ = FLT_MAX;
	float maxX,maxY,maxZ;
	maxX = maxY = maxZ = -FLT_MAX;
	for(Mesh& m : meshes){
		auto& vertices = m.GetVertices();
		int cpt = vertices.GetCount() -1;
		for (int i = 0; i < cpt; i = i + 3) {
			maxX = max(maxX, vertices[i]);
			minX = min(minX, vertices[i]);
			maxY = max(maxY, vertices[i+1]);
			minY = min(minY, vertices[i+1]);
			maxZ = max(maxZ, vertices[i+2]);
			minZ = min(minZ, vertices[i+2]);
		}
	}
	boundingBox.SetBoundingBox(minX,minY,minZ,maxX,maxY,maxZ);
}

void Object3D::RemoveBoundingBox(){
	boundingBox = BoundingBox();
}

bool Object3D::UpdateBuffer(GLuint buffer, int SurfaceCount ,int SurfaceNumber, int count, int numberOfElement, const float * data)noexcept{
	glBindBuffer(GL_ARRAY_BUFFER,buffer);
	float* ptr = (float*) glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
	//float* ptr = (float*) glMapNamedBuffer(buffer,GL_WRITE_ONLY);
	if(ptr){
		if(SurfaceNumber < SurfaceCount){
			ptr += SurfaceNumber;
			for(int i = 0; i < count; i++){
				for(int e = 0; e < numberOfElement; e++){
					*(ptr++) = data[e];
				}
			}
			glUnmapBuffer(GL_ARRAY_BUFFER);
			return true;
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
		return false;
	}
	return false;
}
Vector<float> Object3D::ReadBuffer(GLuint buffer, int SurfaceCount , int SurfaceNumber, int count, int numberOfElement)noexcept{
//	float* ptr = (float*) glMapNamedBuffer(buffer,GL_READ_ONLY);
	glBindBuffer(GL_ARRAY_BUFFER,buffer);
	float* ptr = (float*) glMapBuffer(GL_ARRAY_BUFFER,GL_READ_ONLY);
	Vector<float> data;
	if(ptr){
		if(SurfaceNumber < SurfaceCount){
			ptr += SurfaceNumber;
			for(int i = 0; i < count; i++){
				for(int e = 0; e < numberOfElement; e++){
					data.Add(*ptr);
				}
			}
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	return data;
}
bool Object3D::UpdateColor(unsigned int MeshNo, unsigned int SurfaceNumber, int r, int g, int b, int alpha)noexcept{
	float data[]= {r/255.0f,g/255.0f,b/255.0f, alpha/255.0f};
	if(MeshNo < meshes.GetCount()){
		return UpdateBuffer(meshes[MeshNo].GetColorsVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber, 1 ,4,data);
	}else{
		return false;
	}
}
bool Object3D::UpdateColor(unsigned int MeshNo, unsigned int SurfaceNumber, float r, float g, float b, float alpha)noexcept{
	float data[] = {r,g,b, alpha};
	if(MeshNo < meshes.GetCount()){
		return UpdateBuffer(meshes[MeshNo].GetColorsVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber, 1 ,4,data);
	}else{
		return false;
	}
}
bool Object3D::UpdateColor(unsigned int MeshNo, unsigned int SurfaceNumber, glm::vec3 color, float alpha)noexcept{
	if(MeshNo < meshes.GetCount()){
		float data[]= {color.x,color.y,color.z, alpha};
		return UpdateBuffer(meshes[MeshNo].GetColorsVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber, 1 ,4,data);
	}else{
		return false;
	}
}
bool Object3D::UpdateColor(unsigned int MeshNo, unsigned int SurfaceNumber, Upp::Color color, int alpha)noexcept{
	float data[] = {color.GetR()/255.0f,color.GetG()/255.0f,color.GetB()/255.0f, alpha/255.0f};
	if(MeshNo < meshes.GetCount()){
		return UpdateBuffer(meshes[MeshNo].GetColorsVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber, 1 ,4,data);
	}else{
		return false;
	}
}
bool Object3D::UpdateColors(unsigned int MeshNo, unsigned int SurfaceNumber,int Count, const float * data)noexcept{
	if(MeshNo < meshes.GetCount()){
		return UpdateBuffer(meshes[MeshNo].GetColorsVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber, Count ,4,data);
	}else{
		return false;
	}
}
bool Object3D::UpdateNormal(unsigned int MeshNo, unsigned int SurfaceNumber, float x, float y, float z)noexcept{
	float data[] = {x,y,z};
	if(MeshNo < meshes.GetCount()){
		return UpdateBuffer(meshes[MeshNo].GetNormalsVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber, 1 ,3, data);
	}else{
		return false;
	}
}
bool Object3D::UpdateNormal(unsigned int MeshNo, unsigned int SurfaceNumber, glm::vec3 normal)noexcept{
	if(MeshNo < meshes.GetCount()){
		return UpdateBuffer(meshes[MeshNo].GetNormalsVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber, 1 ,3, &(normal.x));
	}else{
		return false;
	}
}
bool Object3D::UpdateNormals(unsigned int MeshNo, unsigned int SurfaceNumber,int Count, const float * data)noexcept{
	if(MeshNo < meshes.GetCount()){
		return UpdateBuffer(meshes[MeshNo].GetNormalsVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber, Count ,3, data);
	}else{
		return false;
	}
}
bool Object3D::UpdateVertice(unsigned int MeshNo, unsigned int SurfaceNumber, float x, float y, float z)noexcept{
	float data[] = {x,y,z};
	if(MeshNo < meshes.GetCount()){
		return UpdateBuffer(meshes[MeshNo].GetVerticesVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber, 1 ,3, data);
	}else{
		return false;
	}
}
bool Object3D::UpdateVertice(unsigned int MeshNo, unsigned int SurfaceNumber, glm::vec3 vertice)noexcept{
	if(MeshNo < meshes.GetCount()){
		return UpdateBuffer(meshes[MeshNo].GetVerticesVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber, 1 ,3, &(vertice.x));
	}else{
		return false;
	}
}
bool Object3D::UpdateVertices(unsigned int MeshNo, unsigned int SurfaceNumber,int Count, const float * data)noexcept{
	if(MeshNo < meshes.GetCount()){
		return UpdateBuffer(meshes[MeshNo].GetVerticesVBO(), meshes[MeshNo].GetVertices().GetCount()/3 ,SurfaceNumber, Count ,3, data);
	}else{
		return false;
	}
}
Vector<float> Object3D::ReadColors(int MeshNo, unsigned int SurfaceNumber, int count){
	if(MeshNo < meshes.GetCount()){
		return ReadBuffer(meshes[MeshNo].GetColorsVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber,count,4);
	}else{
		throw Exc(Format(t_("Object3D '%i' don't have meshes no '%i', colors can't be readed\n"), ID, MeshNo));
	}
}
Vector<float> Object3D::ReadNormals(int MeshNo, unsigned int SurfaceNumber, int count){
	if(MeshNo < meshes.GetCount()){
		return ReadBuffer(meshes[MeshNo].GetNormalsVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber,count,3);
	}else{
		throw Exc(Format(t_("Object3D '%i' don't have meshes no '%i', normals can't be readed\n"), ID, MeshNo));
	}
}
Vector<float> Object3D::ReadVertices(int MeshNo, unsigned int SurfaceNumber, int count){
	if(MeshNo < meshes.GetCount()){
		return ReadBuffer(meshes[MeshNo].GetVerticesVBO(), meshes[MeshNo].GetVertices().GetCount()/3,SurfaceNumber,count,3);
	}else{
		throw Exc(Format(t_("Object3D '%i' don't have meshes no '%i', vertices can't be readed\n"), ID, MeshNo));
	}
}
void Object3D::Draw(glm::mat4 projectionMatrix, glm::mat4 viewMatrix,glm::vec3 viewPosition)noexcept{
	if(visible){
		if(showMesh){
			if(NoLight.IsLinked() && Light.IsLinked()){
				OpenGLProgram&  prog = (showLight)? Light : NoLight;
				prog.Bind();
				if(showLight){
					prog.SetVec3("viewPos",viewPosition.x,viewPosition.y,viewPosition.z);
					if(material.ShouldBeUpdated()){
						prog.SetVec3("mat.Diffuse", material.GetDiffuse().x,material.GetDiffuse().y,material.GetDiffuse().z);
						prog.SetVec3("mat.Specular", material.GetSpecular().x,material.GetSpecular().y,material.GetSpecular().z);
						prog.SetFloat("mat.Shininess", material.GetShininess());
						material.HaveBeenUpdated();
					}
				}
				
				
				prog.SetMat4("ViewMatrix", viewMatrix);
				prog.SetMat4("ProjectionMatrix", projectionMatrix);
				prog.SetMat4("ModelMatrix", transform.GetModelMatrix());
	
				for(Mesh& m : meshes){
					glBindVertexArray(m.GetVAO());
					if(textures.GetCount()> 0){
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[m.GetTextureIndice()].id);
						prog.SetInt("tex", 0);
						prog.SetInt("useTexture", textures[m.GetTextureIndice()].id);
					}
					glDrawArrays(((prog.ContainTCS()) ? GL_PATCHES : drawType), 0, m.GetVertices().GetCount()/3);
				}
			}else{
				ONCELOCK{
					LOG("no Light/NoLight OpenGL Program have been provided, Object3D No " + AsString(ID) +" Can't be light/nolight draw");
				}
			}
		}
		if(showMeshLine){
			if(Line.IsLinked()){
				Line.Bind();
				glLineWidth(lineWidth);
				Line.SetMat4("ViewMatrix",viewMatrix);
				Line.SetMat4("ProjectionMatrix",projectionMatrix);
				Line.SetMat4("ModelMatrix",transform.GetModelMatrix());
				Line.SetVec4("CustomColor", lineColor.GetR() / 255.0f, lineColor.GetG() / 255.0f, lineColor.GetB() / 255.0f, lineOpacity );
				
				for(Mesh& m : meshes){
					glBindVertexArray(m.GetVAO());
					glDrawArrays(((Line.ContainTCS()) ? GL_PATCHES : GL_TRIANGLES), 0, m.GetVertices().GetCount()/3);
				}
				//glDrawArrays(((Line.ContainTCS()) ? GL_PATCHES : GL_TRIANGLES), 0, SurfaceCount);
			}else{
				ONCELOCK{
					LOG("no Line OpenGL Program have been provided, Object3D No " + AsString(ID) +" Can't be line draw");
				}
			}
		}
		if(showMeshNormal){
			if(Normal.IsLinked()){
				Normal.Bind();
				Normal.SetMat4("ViewMatrix", viewMatrix);
				Normal.SetMat4("ProjectionMatrix", projectionMatrix);
				Normal.SetMat4("ModelMatrix", transform.GetModelMatrix());
				Normal.SetVec4("CustomColor",normalColor.GetR() / 255.0f, normalColor.GetG() / 255.0f, normalColor.GetB() / 255.0f, normalOpacity);
				Normal.SetFloat("normal_length",normalLenght );
				for(Mesh& m : meshes){
					glBindVertexArray(m.GetVAO());
					glDrawArrays(GL_TRIANGLES, 0, m.GetVertices().GetCount()/3);
				}
				//glDrawArrays(GL_TRIANGLES, 0, SurfaceCount);
			}else{
				ONCELOCK{
					LOG("no Normal OpenGL Program have been provided, Object3D No " + AsString(ID) +" Can't be normal draw");
				}
			}
		}
		if(showBoundingBox){
			if(Line.IsLinked()){
				boundingBox.Draw(transform.GetModelMatrix(),viewMatrix,projectionMatrix,Line);
			}else{
				ONCELOCK{
					LOG("no Line OpenGL Program have been provided, Object3D No " + AsString(ID) +" Can't have is bounding box draw");
				}
			}
		}
	}
}
}