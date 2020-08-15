SHADER(400 core,
	in vec3 fs_color;\n
	in vec3 fs_normal;\n
	in vec3 fs_fragPos;\n
\n
    out vec4 FragColor;\n
\n
	uniform vec3 viewPos;\n
\n
\n	//material
	struct Material{\n
		vec3 Diffuse;\n
		vec3 Specular;\n
		float Shininess;\n
	};\n
	uniform Material mat;\n
\n
\n	//DirLight
	struct DirLight{\n
	    vec3 Ambient;\n
	    vec3 Diffuse;\n
	    vec3 Specular;\n
	};\n
	DirLight dirlight = DirLight( vec3( 1.0, 1.0, 1.0) , vec3( 1.0, 1.0, 1.0), vec3( 0.64, 0.64, 0.64));\n
//	DirLight dirlight = DirLight( vec3( 0.0, 0.0, 0.0) , vec3( 0.0, 0.0, 0.0), vec3( 0.0, 0.0, 0.0));\n
\n
	struct PointLight{\n
	    vec3 Position;\n
	    vec3 Ambient;
	    vec3 Diffuse;
	    vec3 Specular;
	    float Constant;\n
	    float Linear;\n
	    float Quadratic;\n
	};
	PointLight pointLight = PointLight ( vec3( 0.0, 0.0, 0.0), vec3( 1.0,0.0,0.0) , vec3( 2.0, 2.0, 2.0), vec3(  1.0, 1.0, 1.0), 1.0, 0.022, 0.0019);
	
	vec3 CalcColorDirLight(Material material,DirLight light, vec3 normal, vec3 fragPos, vec3 viewPos);\n
	vec3 CalcPointLight(Material material,PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);\n
\n
    void main(){\n
        vec3 norm = normalize(fs_normal);
		FragColor =  vec4( (CalcColorDirLight(mat,dirlight,norm,fs_fragPos,viewPos)  + CalcPointLight(mat,pointLight,norm,fs_fragPos,viewPos)) , 1.0);\n
    }\n
\n
    vec3 CalcColorDirLight(Material material,DirLight dirlight, vec3 normal,vec3 fragPos, vec3 viewPos){\n
	    vec3 ambient  = dirlight.Ambient  * fs_color;\n
	    vec3 lightDir = vec3(0.0, -1.0,0.0);\n
	    float diff = max(dot(normal, lightDir), 0.0);\n
	    vec3 diffuse  = dirlight.Diffuse  * diff * material.Diffuse;\n
	    vec3 viewDir = normalize(viewPos - fragPos);
	    vec3 reflectDir = reflect(-lightDir, normal);\n
	    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);\n
	    vec3 specular = dirlight.Specular * (spec * material.Specular);\n
	    return (ambient + diffuse + specular);\n
	}\n
	
	vec3 CalcPointLight(Material material,PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewPos){\n
	    vec3 lightDir = normalize(pointLight.Position - fragPos);
	    // diffuse shading
	    float diff = max(dot(normal, lightDir), 0.0);
	    // specular shading
	    vec3 reflectDir = reflect(-lightDir, normal);
	    vec3 viewDir = normalize(viewPos);//normalize(viewPos - fragPos);
	    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);
	    // attenuation
	    float distance    = length(pointLight.Position - fragPos);
	    float attenuation = 1.0 / (pointLight.Constant + pointLight.Linear * distance + pointLight.Quadratic * (distance * distance));
	    // combine results
	    vec3 ambient  = pointLight.Ambient  * fs_color;
	    vec3 diffuse  = pointLight.Diffuse  * diff * material.Diffuse;
	    vec3 specular = pointLight.Specular * spec * material.Specular;
	    ambient  *= attenuation;
	    diffuse  *= attenuation;
	    specular *= attenuation;
	    return (ambient + diffuse + specular);
	}
    
)