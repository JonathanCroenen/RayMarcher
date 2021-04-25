#version 460 core
layout (local_size_x = 1, local_size_y = 1) in;
layout (binding = 0, rgba32f) uniform image2D image;

#define PI 3.1415926535
#define EPSILON 0.001
#define MAX_ITERATIONS 64
#define MAX_DIST 1000000

const float fovh = PI/2;
float fovv;

//uniform vec3 viewDirection;
//uniform vec3 viewPosition;
uniform mat4 cameraToWorld;
uniform mat4 invProjection;

struct Camera {
	vec3 position;
	vec3 direction;
};

struct Ray {
	vec3 origin;
	vec3 direction;
};

float intersectSDF(float distA, float distB)
{
	return max(distA, distB);
}

float unionSDF(float distA, float distB)
{
	return min(distA, distB);
}

float differenceSDF(float distA, float distB)
{
	return max(distA, -distB);
}

float sphereSDF(vec3 p, vec3 pos, float radius)
{
	p = p - pos;
	return length(p) - radius;
}

float boxSDF(vec3 p, vec3 pos, vec3 size)
{
	p = p - pos;
	vec3 q = abs(p) - size;
	return length(max(q, 0)) + min(max(q.x, max(q.y, q.z)), 0);
}


float sceneSDF(vec3 p)
{
	float sphere = sphereSDF(p, vec3(10, 0, 0), 1.2);
	float cube = boxSDF(p, vec3(10, 0, 0), vec3(1));
	return intersectSDF(sphere, cube);
}

vec3 estimateNormal(vec3 p)
{
	return normalize(vec3(sceneSDF(vec3(p.x + EPSILON, p.yz)) - sceneSDF(vec3(p.x - EPSILON, p.yz)),
					 sceneSDF(vec3(p.x, p.y + EPSILON, p.z)) - sceneSDF(vec3(p.x, p.y - EPSILON, p.z)),
					 sceneSDF(vec3(p.xy, p.z + EPSILON)) - sceneSDF(vec3(p.xy, p.z - EPSILON))));
}

float rayMarch(Ray ray)
{
	float closestDist = MAX_DIST;
	float travelledDist = 0;
	vec3 position = ray.origin;
	for (int i = 0; i < MAX_ITERATIONS; i++) {
		closestDist = sceneSDF(position);
		travelledDist += closestDist;

		if (closestDist < EPSILON) {
			return travelledDist;
		} else if (travelledDist > MAX_DIST) {
			return MAX_DIST;
		}

		position += closestDist * ray.direction;
	}
	return MAX_DIST;
}


void shading(vec2 pixel, Ray ray, float dist)
{
	if (dist != MAX_DIST) {
		vec3 p = ray.origin + dist * ray.direction;

		const int numLights = 3;
		vec3 light[numLights] = {vec3(4, 10, -10), vec3(4, 10, 10), vec3(-5, 10, 10)};

		vec3 normal = estimateNormal(p);

		vec4 color;
		for (int i = 0; i < numLights; i++) {
			color += vec4(max(dot(normalize(light[i] - p), normal), 0) * vec3(0.3, 0.4, 1.0), 1.0);
		}

		imageStore(image, ivec2(pixel), color);
	}
	else {
		imageStore(image, ivec2(pixel), vec4(0.7, 0.7, 0.9, 1.0));
	}
}

vec3 erot(vec3 p, vec3 ax, float ro)
{
	return mix(dot(p, ax)*ax, p, cos(ro)) + sin(ro)*cross(ax, p);
}

void main()
{
	vec2 dims = imageSize(image);
	vec2 pixel = gl_GlobalInvocationID.xy;

	//Camera camera = Camera(viewPosition, viewDirection);


	vec3 origin = (cameraToWorld * vec4(0, 0, 0, 1)).xyz;
	vec3 direction = (invProjection * vec4(2*pixel.x/dims.x - 1, 2*pixel.y/dims.y - 1, 0, 1)).xyz;
	direction = (cameraToWorld * vec4(direction, 0)).xyz;
	direction = normalize(direction);

	Ray ray = Ray(origin, direction);
	
	float dist = rayMarch(ray);
	shading(pixel, ray, dist);
	//imageStore(image, ivec2(pixel), vec4(direction, 1));
}