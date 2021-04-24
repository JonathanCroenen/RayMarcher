#version 460 core
layout (local_size_x = 1, local_size_y = 1) in;
layout (binding = 0, rgba32f) uniform image2D image;

uniform vec2 mousePos;

struct Circle {
	vec2 center;
	float radius;
};

struct Box {
	vec2 center;
	vec2 size;
};

struct Camera {
	vec2 position;
	vec2 direction;
};

const int numCircles = 3;
const Circle circles[numCircles] = {{vec2(100, 100), 30}, {vec2(300, 100), 40}, {vec2(600, 400), 70}};

const int numBoxes = 3;
const Box boxes[numBoxes] = {{vec2(100, 450), vec2(100, 80)}, {vec2(1000, 100), vec2(200, 70)}, {vec2(700, 700), vec2(140, 100)}};

float circleSDF(vec2 p, vec2 center, float radius)
{
	return length(center - p) - radius;
}

float boxSDF(vec2 p, vec2 center, vec2 size)
{
	vec2 offset = abs(p - center) - size;

	float unsignedDist = length(max(offset, 0));
	float distInside = min(max(offset.x, offset.y), 0);
	return unsignedDist + distInside;
}

vec4 blend(vec2 pos, vec4 color)
{
	vec4 currentColor = imageLoad(image, ivec2(pos));
	return vec4(color.a * color.rgb + (1.0 - color.a) * currentColor.rgb, 1.0);
}

void clear(vec2 pixelPos)
{
	imageStore(image, ivec2(pixelPos), vec4(1.0, 1.0, 1.0, 1.0));
}

void drawCircle(vec2 pixelPos, vec2 center, float radius, vec4 insideColor, vec4 edgeColor)
{
	float dist = length(pixelPos - center) - radius;
	if (dist <= 1){
		if (dist >= -1){
			imageStore(image, ivec2(pixelPos), blend(pixelPos, edgeColor));
		}
		else {
			imageStore(image, ivec2(pixelPos), blend(pixelPos, insideColor));
		}
	}
}

void drawShapes(vec2 pixelPos)
{
	for (int i = 0; i < numCircles; i++) {
		float dist = circleSDF(pixelPos, circles[i].center, circles[i].radius);
		if (dist <= 0) {
			imageStore(image, ivec2(pixelPos), vec4(0.4, 0.4, 0.7, 1.0));
		}
	}

	for (int i = 0; i < numBoxes; i++) {
		float dist = boxSDF(pixelPos, boxes[i].center, boxes[i].size);
		if (dist <= 0) {
			imageStore(image, ivec2(pixelPos), vec4(0.4, 0.4, 0.7, 1.0));
		}
	}
}	


float sceneSDF(vec2 p)
{
	float dist = 100000;

	for (int i = 0; i < numCircles; i++){
		dist = min(circleSDF(p, circles[i].center, circles[i].radius), dist);
	}

	for (int i = 0; i < numBoxes; i++){
		dist = min(boxSDF(p, boxes[i].center, boxes[i].size), dist);
	}

	return dist;
}


void main()
{
	vec2 pixel = gl_GlobalInvocationID.xy;
	
	clear(pixel);
	drawShapes(pixel);

	Camera camera = {vec2(100, 700), normalize(mousePos - vec2(100, 700))};
	drawCircle(pixel, camera.position, 10, vec4(0.5, 0.5, 0.5, 1.0), vec4(0.5, 0.5, 0.5, 1.0));

	float dist = 100000;
	vec2 pos = camera.position;
	int iterations = 0;
	while (dist > 1 && iterations <= 30) {
		dist = sceneSDF(pos);
		drawCircle(pixel, pos, dist, vec4(0.5, 0.5, 0.5, 0.1), vec4(0.4, 0.4, 0.4, 1.0));
		pos += dist * camera.direction;
		iterations++;
	}
}