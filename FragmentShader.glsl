#version 410

uniform float MaxIterations;
uniform vec2 c;

out vec4 pixelColor;
in vec2 coord;
uniform sampler2D ourTexture;


void main()
{
    float   real  = coord.x;
    float   imag  = coord.y; 

    float r2 = 0.0;
	int count; 
    for (count = 0; count < MaxIterations && r2 < 4.0; ++count)
    {
        float temp = real;

        real = (temp * temp) - (imag * imag) + c.x;
        imag = 2.0 * temp * imag + c.y;
        r2   = (real * real) + (imag * imag);
		
    }

    vec3 color;

    if (r2 <= 4.0)
        color = vec3(0.0f, 0.0f, 0.0f);
    else
        color = vec3(count/7.0f, count/7.0f, count/7.0f);

    pixelColor = texture(ourTexture, gl_PointCoord) * vec4(color, 1.0);

}




