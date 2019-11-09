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

uniform sampler2D FullSampler;
uniform sampler2D yBlur;
uniform sampler2D xBlur;
uniform sampler2DShadow DepthSampler;

void main(void)
{
	vec4 depth = shadow2D(DepthSampler, texCoordz);

	float dof = depth.r * 0.005;

	vec4 col0 = texture2D(FullSampler, texCoord + vec2(dof,0.0) );
	vec4 col1 = texture2D(FullSampler, texCoord + vec2(-dof,0.0) );
	vec4 col2 = texture2D(FullSampler, texCoord + vec2(0.0,dof) );
	vec4 col3 = texture2D(FullSampler, texCoord + vec2(0.0,-dof) );

	vec4 col4 = texture2D(FullSampler, texCoord + vec2(dof,dof) );
	vec4 col5 = texture2D(FullSampler, texCoord + vec2(-dof,-dof) );
	vec4 col6 = texture2D(FullSampler, texCoord + vec2(-dof,dof) );
	vec4 col7 = texture2D(FullSampler, texCoord + vec2(-dof,-dof) );

	vec4 col8 = texture2D(FullSampler, texCoord);

	vec4 col = (col0 + col1 + col2 + col3 + col4 + col5 + col6 + col7 + col8)/9.0;

	vec4 bloomy = texture2D(yBlur, texCoord);
	vec4 bloomx = texture2D(xBlur, texCoord);

	col += (bloomy+bloomx) * 0.8;

	gl_FragColor = vec4(col.x, col.y, col.z, 1.0);
}

#endif