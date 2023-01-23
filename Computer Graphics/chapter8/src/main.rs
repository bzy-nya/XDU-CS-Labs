#[macro_use]
extern crate glium;

use gl::camera;
use gl::shader;
use gl::action;
use gl::models;
use glium::glutin::event::ElementState;

fn main() {
    #[allow(unused_imports)]
    use glium::{glutin, Surface};

    let event_loop = glutin::event_loop::EventLoop::new();
    let wb = glutin::window::WindowBuilder::new();
    let cb = glutin::ContextBuilder::new().with_depth_buffer(24);
    let display = glium::Display::new(wb, cb, &event_loop).unwrap();

    let camera = camera::CameraState::new();

    let vertex_buffer = models::teapot(&display);

    let mut step = 0;
    let mut t: f32 = 0.0;
    action::start_loop(event_loop, move |events| {
        if t < 6.28 { t += 0.01; } else {t = 0.0};

        let indices_buffer = match step{
            0 => glium::index::NoIndices(glium::index::PrimitiveType::LinesList),
            _ => glium::index::NoIndices(glium::index::PrimitiveType::TrianglesList)
        };

        let program = match step {
            0 => shader::get_plain_shader(&display),
            1 => shader::get_plain_shader(&display),
            2 => shader::get_ambient_shader(&display),
            3 => shader::get_default_shader(&display),
            _ => shader::get_specular_shader(&display)
        };

        let mut target = display.draw();
        target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);

        let uniforms = match step {
            _ => uniform! {
                perspective: camera.get_perspective(),
                view: camera.get_view(),
                model: [
                    [0.04 * t.sin(), 0.0, 0.04*t.cos(), 0.0],
                    [0.0, 0.04, 0.0, 0.0],
                    [0.04*t.cos(), 0.0, -0.04*t.sin(), 0.0],
                    [0.0, 0.0, 0.0, 1.0f32],
                ],
                light: [0.0, 1.0, -1.0f32],
                shininess: 20.0f32
            }
        };

        let params = glium::DrawParameters {
            depth: glium::Depth {
                test: glium::draw_parameters::DepthTest::IfLess,
                write: true,
                .. Default::default()
            },
            .. Default::default()
        };        

        target.draw(&vertex_buffer, &indices_buffer, &program, &uniforms, &params).unwrap();
        target.finish().unwrap();
        
        let mut action = action::Action::Continue;

        for e in events {
            match e {
                glutin::event::Event::WindowEvent { event, .. } => match event {
                    glutin::event::WindowEvent::CloseRequested => 
                        { action = action::Action::Stop; },
                    glutin::event::WindowEvent::KeyboardInput { device_id: _, input, is_synthetic:_ } =>
                        { match input.state {
                            ElementState::Pressed => {step += 1;}
                            _ => {}
                        } }
                    _ => (),
                },
                _ => (),
            }
        }

        action
    });
}