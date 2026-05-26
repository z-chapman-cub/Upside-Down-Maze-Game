uniform vec2 lightPos[100];
uniform int lightCount;
uniform float radius;

void main()
{
  vec2 uv=gl_FragCoord.xy;

  float brightness=0.0;

  for(int i=0;i<lightCount;i++)
  {
    float dist=
      distance(
      uv,
      lightPos[i]
    );

    brightness+=
      1.0-
      smoothstep(
      0.0,
      radius,
      dist
    );
  }

  brightness=
    clamp(
    brightness,
    0.0,
    1.0
  );

  gl_FragColor=
    vec4(
    vec3(brightness),
    1.0
  );
}