#ifdef SQ_VERTEX_SHADER

varying mat3 tangentBasis;

//transformed vertex attribs

varying	vec2 texCoord;
varying	vec3 globalPos;
varying	vec4 projCoords;

//camera properties

uniform	vec3 eyePos;

//matrices

uniform mat3 uNormalMatrix;
uniform mat4 uMVPMatrix;
uniform mat4 uModelViewMatrix;

//vertex attribs

attribute vec4 inPosition;
attribute vec3 inNormal;

void main(void)
{
	vec4 pos	= uModelViewMatrix * inPosition;			// transformed point to world space
	vec3 normal	= normalize ( uNormalMatrix * inNormal );	// transformed normal
	globalPos	= pos.xyz;
	
    vec3 tangent    = cross(normal, vec3(0, 0, 1));
    vec3 binormal   = cross(tangent, normal);
	
	tangentBasis[0] = tangent;
	tangentBasis[1] = binormal;
	tangentBasis[2] = normal;

	gl_Position     = uMVPMatrix * inPosition;
	texCoord		= globalPos.xz;
	projCoords		= gl_Position;
}

#endif
#ifdef SQ_FRAGMENT_SHADER

varying mat3 tangentBasis;

varying	vec2 texCoord;
varying	vec3 globalPos;
varying	vec4 projCoords;

//camera properties

uniform	vec3 eyePos;

//fog properties

uniform vec4 fogColor;
uniform float fogStart;
uniform float fogEnd;

uniform sampler2D normalMap;

uniform sampler2D uReflectionMap;
uniform mat4 uReflectionMatrix;

uniform sampler2D uSceneColorMap;//refraction map

uniform sampler2D uSceneDepthMap;

uniform float uTime;
uniform vec2 uNearFarClips;

const vec4 waterColor = vec4(0.2, 0.3, 0.4, 1.0);
const float waterDensity = 0.5;

const float normalMapScale = 0.5;

const float texDistanceGrowth = 0.0;

float calculateLinearDepth(float value)
{
	return uNearFarClips.y * uNearFarClips.x /
    (uNearFarClips.y - value *
	(uNearFarClips.y - uNearFarClips.x) );
}

#include <common/lighting.glsl>
		 
#ifdef SHADOW_MAP
# include <common/shadow.glsl>
#endif

uniform float uInvShadowDarkness = 0.3;
	
void main (void)
{
	float distanceFromCamera = gl_FragCoord.z / gl_FragCoord.w;//distance(eyePos, globalPos);
	
	float lightIntensity;
	vec3 lightVec;
	computeLightVecAndIntensity(globalPos, lightVec, lightIntensity);

#ifdef SHADOW_MAP
	lightIntensity *= clamp(shadowCoef(globalPos), uInvShadowDarkness, 1.0);
#endif

	float texCoordScale = normalMapScale / (1.0 + distanceFromCamera * texDistanceGrowth);
	vec3 normal = 2.0 * texture2D(normalMap, texCoord * texCoordScale + vec2(uTime * 0.02) ).xzy - vec3(1.0);

	normal = normalize(normal + vec3(0.0, 1.0, 0.0));

	vec3 eyeVec		= normalize(eyePos - globalPos);
	
	vec3 proj_tc  = 0.5 * projCoords.xyz / projCoords.w + 0.5;
	// введем переменную для обозначения
	// масштабного коэффициента HDR изображения
	float fHDRscale = waterColor.w;
	// вычислим коэффициент Френеля для смешивания отражения и преломления
	float fresnel = 1.0 - dot(eyeVec, normal);

	//deal with depth

	// вычисляем расстояние от камеры до точки
	float fOwnDepth = calculateLinearDepth(proj_tc.z);
	// считываем глубину сцены
	float fSampledDepth = texture2D(uSceneDepthMap, proj_tc.xy).x;
	// преобразуем её в линейную (расстояние от камеры)
	fSampledDepth       = calculateLinearDepth(fSampledDepth);
	// получаем линейную глубину воды
	float fLinearDepth  = fSampledDepth - fOwnDepth;
	//Теперь у нас есть глубина, но она линейная, что не очень хорошо описывает изменение цвета воды в зависимости от глубины, и потом мы же хотим контролировать степень «мутности», поэтому преобразуем линейную глубину в экспоненциальную.
	float fExpDepth = 1.0 - exp( -waterDensity * fLinearDepth);
	float fExpDepthHIGH = 1.0 - exp( -0.95 * fLinearDepth );

	//calculate specular

	vec3 lightReflVec = reflect(-lightVec, normal); // вычисляем отраженный вектор
	// получаем скалярное произведение
	// отраженного вектора на вектор взгляда
	float VdotR = max( dot(lightReflVec, eyeVec), 0.0 );
	// аппроксимация Шлика a^b = a / (b – a*b + a) для a от нуля до единицы
	VdotR = pow(VdotR, 80.0);
	//VdotR /= 64.0 - VdotR * 64.0 + VdotR;
	vec3 specularValue = vec3(VdotR) * fExpDepthHIGH * lightIntensity;
	// вычисляем отраженный свет, с учетом затенения и глубины воды в данной точке.

	//calculate refraction

	// величина искажения – чем глубже, тем искажения больше
	float fDistortScale = 0.1 * fExpDepth;
	vec2 vDistort = normal.zx * fDistortScale; // смещение текстурных координат
	// читаем глубину в искаженных координатах
	float fDistortedDepth = texture2D(uSceneDepthMap, proj_tc.xy + vDistort).x;
	// преобразуем её в линейную
	fDistortedDepth = calculateLinearDepth(fDistortedDepth);
	float fDistortedExpDepth = 1.0 - exp( -waterDensity * (fDistortedDepth - fOwnDepth) );
	// вычисляем экспоненциальную глубину в искаженных координатах
	// теперь сравниваем расстояния – если расстояние до воды больше,
	// чем до прочитанной то пренебрегаем искажением
	if (fOwnDepth > fDistortedDepth)
	{
		vDistort = vec2(0.0);
		fDistortedExpDepth = fExpDepth;
	}
	// теперь читаем из текстуры преломлений цвет
	vec3 refraction = texture2D(uSceneColorMap, proj_tc.xy + vDistort).xyz;
	// и закрашиваем его цветом воды, в зависимости от глубины
	refraction = mix(refraction, waterColor.xyz * fHDRscale, fDistortedExpDepth);

	//sample reflection

	// коэффициент можно подобрать под свои нужды
	vDistort = normal.xz * 0.7;
	vec4 reflTexcoord = uReflectionMatrix * vec4(globalPos, 1.0);
	vec3 reflection = texture2DProj ( uReflectionMap, reflTexcoord + vec4(vDistort, 0.0, 0.0) ).rgb;

	//mix reflection and refraction

	// вычисляем коэффициент смешивания
	// как коэффициент Френеля плюс вычисленная глубина
	// для плавного перехода из «берега» в воду
	// смешиваем и умножаем на осветленную тень
	vec3 result_color = mix(refraction, reflection, fresnel * fExpDepthHIGH) * lightIntensity;
	result_color += specularValue; // добавляем отраженный свет

	float fog = clamp((distanceFromCamera - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
	result_color = mix(result_color, fogColor.xyz, fog * fog);

	gl_FragColor = vec4(result_color, 1.0);
}

#endif