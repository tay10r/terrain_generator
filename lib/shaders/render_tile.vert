#version 110

attribute vec3 gPosition;

attribute vec3 gNormal;

uniform mat4 gMVP;

uniform mat3 gNormalMVP;

varying vec2 gInterpolatedTexCoord;

varying vec3 gInterpolatedNormal;

void
main()
{
  gInterpolatedTexCoord = gPosition.xy;

  gInterpolatedNormal = gNormalMVP * gNormal;

  gl_Position = gMVP * vec4(gPosition, 1.0);
}
