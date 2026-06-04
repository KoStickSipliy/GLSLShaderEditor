void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    float T = iTime * PARAM1 / 20.0;
    vec2 uv = fragCoord / iResolution.xy;
    vec3 col = 0.5 + 0.5 * cos(T + uv.xyx + vec3(0.0, 2.0, 4.0));
    fragColor = vec4(col, 1.0);
}
