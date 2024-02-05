#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fragTexCoords;
out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

vec3 ambient;
float ambientStrength = 0.2f;//0.2f
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;//0.5f
float shininess = 32.0f;//32.0f

float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	vec3 lightPosEye = vec3(0.0f);///de schimbat mai incolo
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	//vec3 lightDirN = normalize(lightDir);
	vec3 lightDirN = normalize(lightPosEye - fPosEye.xyz);


	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

	vec3 halfVector = normalize(lightDirN + viewDirN);
		
		//compute distance to light
	float dist = length(lightPosEye - fPosEye.xyz);
	dist = dist /1.5f;
	//compute attenuation
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));


	//compute ambient light
	ambient = att * ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	


	//compute specular light
	//vec3 reflection = reflect(-lightDirN, normalEye);
	//float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	specular = att * specularStrength * specCoeff * lightColor;
}


float computeFog()
{
 float fogDensity = 0.0;
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
	computeLightComponents();
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	
	//ambient *= baseColor;
	//diffuse *= baseColor;
	//specular *= baseColor;
	
	ambient *= texture(diffuseTexture, fragTexCoords);
	diffuse *= texture(diffuseTexture, fragTexCoords);
	specular *= texture(specularTexture, fragTexCoords);

	vec3 color = min((ambient + diffuse) + specular, 1.0f);
    
    fColor = vec4(color, 1.0f);

	// for fog
	///FOG DENSITY TO BE CHANGED FOR LESS FOG
    //float fogFactor = computeFog();
    //vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    //vec4 finalColor = mix(fogColor, vec4(color, 1.0f), fogFactor);

   // fColor = finalColor;

}
