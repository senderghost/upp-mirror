#ifndef _SurfaceCtrl_OBJLoader_h_
#define _SurfaceCtrl_OBJLoader_h_

#define LLOG(x) LOG(x)

namespace Upp{

	class OBJLoader{
		private:
			struct vec3 : public Moveable<vec3>{
				public:
					float x;
					float y;
					float z;
			};

			static bool LoadOBJText(const String& fileName, Vector<float>& vertices, Vector<float>& normals, Vector<float>& textures){
				int lineCpt = 0;
				FileIn in(fileName);
				if (!in.IsOpen()){
					LLOG(Format(t_("Impossible to open file '%s'"), fileName));
					return false;
				}
				Vector<vec3> allVertices;
				Vector<vec3> allTextures;
				Vector<vec3> allNormals;
				
				allVertices.Create(); //Create a dummy item
				allTextures.Create(); //Create a dummy item
				allNormals.Create(); //Create a dummy item
				
				String line;
				while (!in.IsEof()) {
					line = in.GetLine();
					lineCpt++;
					if(line.StartsWith("#"))
						continue;
					if(line.StartsWith("v ")){
						//vertices
						Vector<String> data = Split(line, " ");
						if(data.GetCount() != 4){
							LLOG(AsString(lineCpt) + ". " +t_("vertices with more than 3 floats !"));
							return false;
						}else{
							vec3& vec = allVertices.Create();
							vec.x = Atof(data[1]);
							vec.y = Atof(data[2]);
							vec.z = Atof(data[3]);
						}
						continue;
					}
					if(line.StartsWith("vt")){
						//Texture coordinate
						Vector<String> data = Split(line, " ");
						if(data.GetCount() != 3){
							LLOG(AsString(lineCpt) + ". " +t_("texture coordinate with more than 2 floats !"));
							return false;
						}else{
							vec3& vec = allTextures.Create();
							vec.x = Atof(data[1]);
							vec.y = Atof(data[2]);
						}
						continue;
					}
					if(line.StartsWith("vn")){
						//Normal
						Vector<String> data = Split(line, " ");
						if(data.GetCount() != 4){
							LLOG(AsString(lineCpt) + ". " +t_("normal with more than 3 floats !"));
							return false;
						}else{
							vec3& vec = allNormals.Create();
							vec.x = Atof(data[1]);
							vec.y = Atof(data[2]);
							vec.z = Atof(data[3]);
						}
						continue;
					}
					if(line.StartsWith("f ")){
						//All pannel
						Vector<String> data = Split(line, " ");
						for(int e = 1; e < 4; e++){
							Vector<String> data2 = Split(data[e], "/");
							vec3& vertex = allVertices[Atof(data2[0])];
							vec3& texture = allTextures[Atof(data2[1])];
							vec3& normal = allNormals[Atof(data2[2])];
							vertices << vertex.x << vertex.y << vertex.z;
							textures << texture.x << texture.y;
							normals << normal.x << normal.y << normal.z;
						}
						continue;
					}
				}
				return true;
			}
		public:
			
		static bool LoadOBJ(const String& fileName, Vector<float>& vertices, Vector<float>& normals, Vector<float>& textures){
			return LoadOBJText(fileName,  vertices, normals , textures);
		}
		
		static bool SaveSTL(const String& fileName,const Vector<float>& vertices, const Vector<float>& normals){
			//TODO
			return false;
		}
	};
}
#endif
