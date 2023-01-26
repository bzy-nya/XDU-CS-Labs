use glium;
use glium::Display;

pub const VERTEX_SHADER: &str = r#"
    #version 140
    in vec3 position;
    in vec3 normal;
    out vec3 v_normal;
    out vec3 v_position;
    uniform mat4 perspective;
    uniform mat4 view;
    uniform mat4 model;
    void main() {
        mat4 modelview = view * model;
        v_normal = transpose(inverse(mat3(modelview))) * normal;
        gl_Position = perspective * modelview * vec4(position, 1.0);
        v_position = gl_Position.xyz / gl_Position.w;
    }
"#;

pub const FRAGMENT_SHADER: &str = r#"
    #version 140
    in vec3 v_normal;
    out vec4 f_color;
    const vec3 LIGHT = vec3(-0.2, 0.8, 0.1);
    void main() {
        float lum = max(dot(normalize(v_normal), normalize(LIGHT)), 0.0);
        vec3 color = (0.3 + 0.7 * lum) * vec3(1.0, 1.0, 1.0);
        f_color = vec4(color, 1.0);
    }
"#;

pub const DARK_FRAGMENT_SHADER: &str = r#"
    #version 140
    in vec3 v_normal;
    out vec4 f_color;
    void main() {
        vec3 color = 0.3 * vec3(1.0, 1.0, 1.0);
        f_color = vec4(color, 1.0);
    }
"#;

pub const PLAIN_FRAGMENT_SHADER: &str = r#"
    #version 140
    in vec3 v_normal;
    out vec4 f_color;
    void main() {
        vec3 color = vec3(1.0, 1.0, 1.0);
        f_color = vec4(color, 1.0);
    }
"#;

pub const SPECULAR_FRAGMENT_SHADER: &str = r#"
    #version 140
    in vec3 v_normal;
    out vec4 f_color;

    uniform vec3 light;
    uniform float shininess; 

    void main() {
        float lum  = max( dot(normalize(v_normal), normalize(light)), 0.0);
        vec3 half_dir = normalize(light + vec3(0.0, 0.0, -1.0));
        float spec = pow(max( dot( normalize(v_normal), half_dir ), 0.0), shininess);
        vec3 color = (0.1 + 0.1 * lum + 0.8 * spec) * vec3(1.0, 1.0, 1.0);
        f_color = vec4(color, 1.0);
    }
"#;

pub const COLORED_FRAGMENT_SHADER: &str = r#"
    #version 140
    in vec3 v_normal;
    out vec4 f_color;

    uniform vec3 light;
    uniform float shininess; 
    uniform vec3 color;

    void main() {
        float lum  = max( dot(normalize(v_normal), normalize(light)), 0.0);
        vec3 half_dir = normalize(light + vec3(0.0, 0.0, -1.0));
        float spec = pow(max( dot( normalize(v_normal), half_dir ), 0.0), shininess);
        vec3 ret_color = (0.1 + 0.1 * lum + 0.8 * spec) * color;
        f_color = vec4(color, 1.0);
    }
"#;

pub fn get_default_shader(display: &Display) -> glium::Program {
    return glium::Program::from_source(display, VERTEX_SHADER, FRAGMENT_SHADER, None).unwrap();
}

pub fn get_plain_shader(display: &Display) -> glium::Program {
    return glium::Program::from_source(display, VERTEX_SHADER, PLAIN_FRAGMENT_SHADER, None).unwrap();
}

pub fn get_ambient_shader(display: &Display) -> glium::Program {
    return glium::Program::from_source(display, VERTEX_SHADER, DARK_FRAGMENT_SHADER, None).unwrap();
}

pub fn get_specular_shader(display: &Display) -> glium::Program {
    return glium::Program::from_source(display, VERTEX_SHADER, SPECULAR_FRAGMENT_SHADER, None).unwrap();
}

pub fn get_colored_shader(display: &Display) -> glium::Program {
    return glium::Program::from_source(display, VERTEX_SHADER, COLORED_FRAGMENT_SHADER, None).unwrap();
}