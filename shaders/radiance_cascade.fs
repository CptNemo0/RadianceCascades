#version 430
out vec4 FragColor;

in vec2 uv;

uniform vec2 resolution;

uniform float base;
uniform float cascadeIndex;
uniform float cascadeCount;
uniform bool lastIndex;
uniform float srgb;

uniform sampler2D sceneTexture;
uniform sampler2D distanceTexture;
uniform sampler2D lastTexture;

const float PI = 3.14159265;
const float TAU = 2.0 * PI;

const int maxSteps = 32;
const float EPS = 0.0001;

bool outOfBounds(vec2 uv) {
    return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
}

void main() {
    vec2 coord = floor(uv * resolution);

    vec4 radiance = vec4(0.0);

    float rayCount = pow(base, cascadeIndex + 1.0);
    float sqrtBase = sqrt(base);

    float oneOverRayCount = 1.0 / float(rayCount);
    float angleStepSize = TAU * oneOverRayCount;

    bool firstLevel = cascadeIndex == 0.0;

    float spacing = pow(sqrtBase, cascadeIndex);

    vec2 size = resolution / spacing;
    vec2 probeRelativePosition = mod(coord, size);
    vec2 rayPos = floor(coord / size);

    vec2 probeCenter = (probeRelativePosition + 0.5) * spacing;
    vec2 normalizedProbeCenter = probeCenter / resolution;

    vec2 oneOverSize = 1.0 / resolution;
    float shortestSide = min(resolution.x, resolution.y);
    vec2 scale = shortestSide * oneOverSize;

    // Hand-wavy rule that improved smoothing of other base ray counts
    float modifierHack = base < 16.0 ? 1.0 : 4.0;

    float intervalStart = firstLevel ? 0.0 : (
        modifierHack * pow(base, cascadeIndex - 1.0)
        ) / shortestSide;
    float intervalLength = (modifierHack * pow(base, cascadeIndex)) / shortestSide;

    // Calculate which set of rays we care about
    float baseIndex = float(base) * (rayPos.x + (spacing * rayPos.y));

    float minStepSize = min(oneOverSize.x, oneOverSize.y) * 0.5;

    for (float i = 0.0; i < base; i += 1) {
        float index = baseIndex + i;
        float angleStep = index + 0.5;
        float angle = angleStepSize * angleStep;
        vec2 rayDirection = vec2(cos(angle), -sin(angle));

        vec2 sampleUv = normalizedProbeCenter + intervalStart * rayDirection * scale;

        if (outOfBounds(sampleUv)) {
            continue;
        }

        vec4 radDelta = vec4(0.0);
        float traveled = 0.0;

        // We tested uv already (we know we aren't an object), so skip step 0.
        for (int step = 1; step < maxSteps; step++) {

            // How far away is the nearest object?
            float dist = texture(distanceTexture, sampleUv).r;

            // Go the direction we're traveling
            sampleUv += rayDirection * dist * scale;

            if (outOfBounds(sampleUv)) break;

            if (dist <= minStepSize) {
                vec4 colorSample = texture(sceneTexture, sampleUv);
                radDelta += vec4(
                        pow(colorSample.rgb, vec3(srgb)),
                        colorSample.a
                    );
                break;
            }

            traveled += dist;
            if (traveled >= intervalLength) break;
        }

        bool nonOpaque = radDelta.a == 0.0;

        // Only merge on non-opaque areas
        if (cascadeIndex < cascadeCount - 1.0 && nonOpaque) {
            float upperSpacing = pow(sqrtBase, cascadeIndex + 1.0);
            vec2 upperSize = floor(resolution / upperSpacing);
            vec2 upperPosition = vec2(
                    mod(index, upperSpacing), floor(index / upperSpacing)
                ) * upperSize;

            vec2 offset = (probeRelativePosition + 0.5) / sqrtBase;
            vec2 clamped = clamp(offset, vec2(0.5), upperSize - 0.5);

            vec4 upperSample = texture(
                    lastTexture,
                    (upperPosition + clamped) / resolution
                );

            radDelta += vec4(upperSample.rgb, upperSample.a);
        }

        // Accumulate total radiance
        radiance += radDelta;
    }

    vec4 totalRadiance = vec4(radiance.rgb / float(base), 1.0);

    FragColor = vec4(
            !lastIndex
            ? totalRadiance.rgb : pow(totalRadiance.rgb, vec3(1.0 / srgb)),
            1.0
        );
}
