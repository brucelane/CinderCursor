/*{
	"CREDIT" : "Video texture by Bruce Lane",
	"CATEGORIES" : [
		"ci"
	],
	"DESCRIPTION": "",
	"INPUTS": [
		{
			"NAME": "inputImage",
			"TYPE" : "image"
		},
		{
			"NAME" :"iTextureSize",
			"TYPE" : "point2D",
			"DEFAULT" : [640.0, 480.0],
			"MAX" : [1920.0, 1080.0],
			"MIN" : [64.0, 48.0]
		}
	],
}
*/

void main(void)
{
    vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy; 
    vec3 rgb = IMG_NORM_PIXEL(inputImage, uv * iRenderXY.xy).xyz;
    fragColor=vec4(rgb, 1.0);	
}
