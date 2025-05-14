#version 410 core

out vec4 FinalColor;

uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 cameraPos;

in vec3 Position; 
in vec3 RawPosition;
in vec3 Normal; 

// float getDistance(vec2 a, vec2 b) {
//     return length(b-a);
// }

// vec2 getBezierPoint(vec2 left, vec2 top, vec2 right, float t) {
// 	return mix(mix(left, top, t), mix(top, right, t), t);
// }

void main() {
	// const vec2 points[] = vec2[](
	// 	vec2(0.0f, 1.0f),
	// 	vec2(0.780624749799799775730861640117474307634119997248957038556621491f, 0.625f),
	// 	vec2(0.9757809372497497196635770501468428845426499965611962981957768641, -0.21875), 
	// 	vec2(0.4391014217623873738486096725660792980441924984525383341880995888, -0.8984375),
	// 	vec2(-0.4391014217623873738486096725660792980441924984525383341880995888, -0.8984375),
	// 	vec2(-0.9757809372497497196635770501468428845426499965611962981957768641, -0.21875), 
	// 	vec2(-0.780624749799799775730861640117474307634119997248957038556621491f, 0.625f)
	// );
	// vec2 rawPosition = vec2(RawPosition.x, RawPosition.y);
	// int n = points.length();
	// for (int p = 0; p < n; p++) {
	// 	int leftP = (p-1)%n;
	// 	if (p == 0) {
	// 		leftP = n-1;
	// 	}
	// 	int rightP = (p+1)%n;

	// 	// pull point P out of the shape
	// 	vec2 r1 = points[p] - points[leftP];
	// 	vec2 r2 = points[p] - points[rightP];
	// 	vec2 r = (r1+r2)/length(r1+r2);
	// 	vec2 pointP = points[p] + 0.3*r;

	// 	// create a line from pointP to the rawPosition vector
	// 	float m1 = (pointP.y - rawPosition.y) / (pointP.x - rawPosition.x);
	// 	float b1 = pointP.y - m1 * pointP.x;

	// 	int SEGMENTS=20;
	// 	float lastT = 0.0f;
	// 	float t = 0.0f;
	// 	for (int i = 1; i < SEGMENTS+1; i++) {
	// 		t = float(i)/float(SEGMENTS);
	// 		// Get vector A and B which forms the line of approximation to the Bezier curve between time lastT and t
	// 		vec2 A = getBezierPoint(points[leftP], pointP, points[rightP], lastT);
	// 		vec2 B = getBezierPoint(points[leftP], pointP, points[rightP], t);
	// 		// Create a line out of line segment AB
	// 		float m2 = (B.y - A.y) / (B.x - A.x);
	// 		float b2 = B.y - m2*B.x; 
	// 		// Find the POI between line connecting pointP and rawPosition with AB
	// 		if (m1-m2 != 0 && m2-m1 != 0) {
	// 			vec2 POI = vec2((b2-b1)/(m1-m2), (m2*b1-m1*b2)/(m2-m1));
	// 			// discard any fragment closer to pointP than the POI
	// 			if (getDistance(pointP, rawPosition) < getDistance(pointP, POI)) {
	// 				discard;
	// 				return;
	// 			}
	// 		}		
	// 		lastT = t;
	// 	}
	// }
	// vec3 norm = normalize(vec3(abs(Normal.x), abs(Normal.y), abs(Normal.z)));
	// ambience
	float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - Position);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(cameraPos - Position);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	FinalColor = vec4(result, 1.0f);
}