#ifdef SQ_VERTEX_SHADER

attribute vec4 inPosition;   
attribute vec2 inTexcoord;

uniform mat4 uMVPMatrix;
	  
varying vec2 texCoord;
  
void main(void)
{
	texCoord		= inTexcoord;
	gl_Position     = uMVPMatrix * inPosition;
}

#endif
#ifdef SQ_FRAGMENT_SHADER

varying vec2 texCoord;

uniform sampler2D TEXTURE;
uniform sampler2D lensDecal;
uniform sampler2D colorBuffer;

float INTENSITY					= 0.5;
float NSAMPLES					= 8.0;
float FLARE_DISPERSAL			= 0.2;
float FLARE_HALO_WIDTH			= 0.05;
vec3 FLARE_CHROMA_DISTORTION	= vec3(0.03, 0.05, 0.01);

vec3 textureDistorted(
	in sampler2D tex,
	in vec2 sample_center, // where we'd normally sample
	in vec2 sample_vector,
	in vec3 distortion // per-channel distortion coeffs
) {
	return vec3(
		texture2D(tex, sample_center + sample_vector * distortion.r).r,
		texture2D(tex, sample_center + sample_vector * distortion.g).g,
		texture2D(tex, sample_center + sample_vector * distortion.b).b
	);
}

void main(void)
{
	vec2 texCoordFlipped = -texCoord + 1.0;

	vec2 image_center = vec2(0.5);
	vec2 sample_vector = (image_center - texCoordFlipped) * FLARE_DISPERSAL;
	vec2 halo_vector = normalize(sample_vector) * FLARE_HALO_WIDTH;

	vec3 flareColor = textureDistorted(TEXTURE, texCoordFlipped + halo_vector, halo_vector, FLARE_CHROMA_DISTORTION).rgb;

	for (int i = 0; i < int(NSAMPLES); ++i) {
		vec2 offset = sample_vector * float(i);
		flareColor += textureDistorted(TEXTURE, texCoordFlipped + offset, offset, FLARE_CHROMA_DISTORTION).rgb;
	}

	vec3 sceneColor = texture2D(colorBuffer, texCoord).rgb;
	vec3 lensColor = texture2D(lensDecal, texCoord).rgb * flareColor * INTENSITY;
	const vec3 one = vec3(1.0);

	gl_FragColor = vec4((one - ((one - sceneColor) * (one - lensColor))), 1.0);
}

#endif
