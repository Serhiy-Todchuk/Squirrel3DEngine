uniform float uParallaxSteps;
uniform float uParallaxScale;

float fetchHeight(sampler2D map, vec2 tex)
{
	return texture2D ( map, tex ).a * 2.0 - 1.0;
}
		 
vec2 parallaxMapping(vec2 tex, vec3 eye, sampler2D map)
{
	float step   = 1.0 / uParallaxSteps;
	vec2  dtex   = -eye.xz * uParallaxScale / ( uParallaxSteps * eye.y ); // adjustment for one layer
	float height = 1.0;                                 // height of the layer
	float h      = fetchHeight(map, tex);				// get height
	while ( h < height )								// iterate through layers
	{
		 height	-= step;
		 tex	+= dtex;
		 h		= fetchHeight(map, tex);
	}
	// now find point via linear interpolation
	vec2  prev   = tex - dtex;                          // previous point
	float hPrev  = fetchHeight(map, prev) - (height + step);
	float hCur   = h - height;
	float weight = hCur / (hCur - hPrev );
	
	return weight * prev + (1.0 - weight) * tex;         // interpolate to get tex coords
}