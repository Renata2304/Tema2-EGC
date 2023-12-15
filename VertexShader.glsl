#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Output values to fragment shader
out vec3 world_position;
out vec3 world_normal;

uniform int hp;
uniform int mvp;

float rand(vec3 pos){
    return fract(sin(dot(pos, vec3(25, 82, 12))) * 21880) * 500;
}

void main()
{
    // TODO(student): Compute world space vertex position and normal,
    // and send them to the fragment shader
    world_position = (Model * vec4(v_position, 1)).xyz;
	world_normal = normalize( mat3(Model) * normalize(v_normal) );

    float rand_fl = 1;
    float rand_v = rand(v_position);
    vec3 new_v_position = v_position;
    if (mvp == 0 && hp != 3) {
        if (hp == 2)
            new_v_position = v_position - (vec3(int(rand_v) % 2) * 0.15 * 0.3);
        if (hp == 1)
            new_v_position = v_position - (vec3(int(rand_v) % 4) * 0.15 * 0.3);
        if (hp <= 0)
            new_v_position = v_position - (vec3(int(rand_v) % 6) * 0.15 * 0.3);
        gl_Position = Projection * View * Model * vec4(new_v_position, 1);
    } else {
        gl_Position = Projection * View * Model * vec4(v_position, 1);
    }

    //gl_Position = Projection * View * Model * vec4(v_position * rand_fl, 1);
    // / ((3 - max(0, hp)) * rand_fl * pow(10, (3 - max(0, hp))))
}