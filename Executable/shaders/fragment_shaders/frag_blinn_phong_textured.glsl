#version 430

#include "../fragments/fs_common_inputs.glsl"

// We output a single color to the color buffer
layout(location = 0) out vec4 frag_color;

////////////////////////////////////////////////////////////////
/////////////// Instance Level Uniforms ////////////////////////
////////////////////////////////////////////////////////////////

// Represents a collection of attributes that would define a material
// For instance, you can think of this like material settings in 
// Unity
struct Material {
	sampler2D	Diffuse;
	sampler1D	DiffRamp;
	sampler1D	SpecRamp;
	float		Shininess;
	int			Mode;
	int			ColorGrade;
	bool		DiffuseRamp;
	bool		SpecularRamp;
};
// Create a uniform for the material
uniform Material u_Material;

////////////////////////////////////////////////////////////////
///////////// Application Level Uniforms ///////////////////////
////////////////////////////////////////////////////////////////

#include "../fragments/multiple_point_lights.glsl"

////////////////////////////////////////////////////////////////
/////////////// Frame Level Uniforms ///////////////////////////
////////////////////////////////////////////////////////////////

#include "../fragments/frame_uniforms.glsl"
#include "../fragments/color_correction.glsl"

// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {
	// Normalize our input normal
	vec3 normal = normalize(inNormal);
	vec3 lightAccumulation;
	vec3 result;

	// Get the albedo from the diffuse / albedo map
	vec4 textureColor = texture(u_Material.Diffuse, inUV);

	// DISCLAIMER: THIS CODE WAS WRITTEN WITH THE HELP OF https://www.roxlu.com/2014/037/opengl-rim-shader
	// There's not much I could really do to alter its implementation besides making it more understandable
	// due to the fact that rim lighting/fresnel shading is based on such a simple formula.
	vec3 n = normalize(mat3(u_ViewProjection) * normal);		// converting the object normal to view space
	vec3 camPosView = vec3(u_ViewProjection * u_CamPos);        // camera position in view space
	vec3 viewer = normalize(-camPosView);						// vector from point to the viewer
	float rimLight = max(dot(viewer, n), 0.0);					// dot product of surface normal and vector to viewer in view space

	switch(u_Material.Mode) {
	case 1: //unlit shader
		result = inColor * textureColor.rgb;
		break;
	case 2: //ambient lighting only
		result = vec3(0.1,0.1,0.2) * inColor * textureColor.rgb;
		break;
	case 3: //specular lighting only
		lightAccumulation = CalcAllLightContribution(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess, 2);
		result = lightAccumulation * inColor * textureColor.rgb;
		break;
	case 4: //ambient + specular
		lightAccumulation = CalcAllLightContribution(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess, 2);
		result = (vec3(0.1,0.1,0.2) + lightAccumulation) * inColor * textureColor.rgb;
		break;
	case 5: //full blinn-phong + rim light effect
		lightAccumulation = CalcAllLightContribution(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess, 1);
		result = (vec3(0.1,0.1,0.2) + lightAccumulation) * inColor * textureColor.rgb + rimLight;
		break;
	default: //full binn-phong with ambient, diffuse, and specular
		lightAccumulation = CalcAllLightContribution(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess, 1);
		result = (vec3(0.1,0.1,0.2) + lightAccumulation) * inColor * textureColor.rgb;
		break;
	}

	frag_color = vec4(ColorCorrect(result), textureColor.a);
}