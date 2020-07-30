SHADER(400 core,
	layout (triangles) in;
	layout (triangle_strip, max_vertices = 3) out;
	
	in vec3 fs_color[3];\n
	in vec3 fs_normal[3];\n
	in vec3 fs_fragPos[3];\n
	out vec3 color;\n
	out vec3 normal;\n
	out vec3 fragPos;\n

    void main()
    {
		int i;
		vec3 v1 = gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz;
		vec3 v2 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
		
		//vec3 v1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
		//vec3 v2 = gl_in[0].gl_Position.xyz - gl_in[2].gl_Position.xyz;
		vec3 v3 = cross(v1,v2);
		for(i = 0; i < gl_in.length(); i++){
			gl_Position =  gl_in[i].gl_Position;
			color = fs_color[i];
			normal = v3;
			fragPos = gl_in[i].gl_Position.xyz;
			EmitVertex();
		}
		
		
		gl_Position = gl_in[0].gl_Position;
		color = fs_color[0];
		normal = v3;
		fragPos = gl_in[0].gl_Position.xyz;
		EmitVertex();
		EndPrimitive();
    }
)