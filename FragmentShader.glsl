#version 330

// Input
in vec3 world_position;
in vec3 world_normal;
in vec3 color_crt;

// Uniforms for light properties
uniform vec3 light_direction;
uniform vec3 light_position;
uniform vec3 eye_position;
uniform vec3 light_position2;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;
uniform int hp;
uniform int mvp;

uniform float angle;

// TODO(student): Declare any other uniforms
uniform bool spotLight;
uniform vec3 object_color;

// Output
layout(location = 0) out vec4 out_color;


void main()
{   
    vec3 N = normalize(world_normal);
    vec3 L = normalize(light_position - world_position);
    vec3 V = normalize(eye_position - world_position);
    vec3 H = normalize(L + V);

    vec3 L2 = normalize(light_position2 - world_position);
    vec3 H2 = normalize(L2 + V);

    float diffuse_light2 = material_kd * max(dot(N, L2), 0);

    // TODO(student): Define ambient, diffuse and specular light components
    float ambient_light = 0.25;
    float diffuse_light = material_kd * max(dot(N,L), 0);
    float specular_light = 0;
    // It's important to distinguish between "reflection model" and
    // "shading method". In this shader, we are experimenting with the Phong
    // (1975) and Blinn-Phong (1977) reflection models, and we are using the
    // Phong (1975) shading method. Don't mix them up!
    if (diffuse_light > 0)
    {
        specular_light = material_ks * pow(max(dot(N, H), 0), material_shininess);
    }

    float specular_light2 = 0;

    if (diffuse_light2 > 0)
    {
        specular_light2 = material_ks * pow(max(dot(N, H2), 0), material_shininess);
    }


    // TODO(student): If (and only if) the light is a spotlight, we need to do
    // some additional things.
    float light_att_factor;
    if (spotLight)
    {
        float cut_off = radians(angle);
        float spot_light = dot(-L, light_direction);
        float spot_light_limit = cos(cut_off);

        if (spot_light > spot_light_limit)
        {
            // Quadratic attenuation
            float linear_att = (spot_light - spot_light_limit) / (1.0f - spot_light_limit);
            light_att_factor = pow(linear_att, 2);
        }
    }
    else
    {
        float distance = distance(light_position, world_position);
        light_att_factor = 1 / (1 + 0.12 * distance + 0.05 * distance * distance);
    }

    float light_att_factor2;
    if (spotLight)
    {
        float cut_off = radians(angle);
        float spot_light = dot(-L2, light_direction);
        float spot_light_limit = cos(cut_off);

        if (spot_light > spot_light_limit)
        {
            // Quadratic attenuation
            float linear_att = (spot_light - spot_light_limit) / (1.0f - spot_light_limit);
            light_att_factor2 = pow(linear_att, 2);
        }
    }
    else
    {
        float distance = distance(light_position2, world_position);
        light_att_factor2 = 1 / (1 + 0.12 * distance + 0.05 * distance * distance);
    }


    // TODO(student): Compute the total light. You can just add the components
    // together, but if you're feeling extra fancy, you can add individual
    // colors to the light components. To do that, pick some vec3 colors that
    // you like, and multiply them with the respective light components.
    float light = ambient_light + light_att_factor * (diffuse_light + specular_light);
    float light_total2 = ambient_light + light_att_factor2 * (diffuse_light2 + specular_light2);
    float final_light = light + light_total2;

    // TODO(student): Write pixel out color
    vec3 color = object_color;
    
    if (mvp == 1) 
        color = object_color - vec3(0.05 * (5 - max(0, hp)));
    else 
        color = object_color - vec3(0.1 * (3 - max(0, hp)));
    out_color =  vec4((color) * final_light, 1);
}