#version 110

varying vec2 gInterpolatedTexCoord;

varying vec3 gInterpolatedNormal;

void
main()
{
  gl_FragColor = vec4(gInterpolatedTexCoord, gInterpolatedNormal.x, 1.0);
}
