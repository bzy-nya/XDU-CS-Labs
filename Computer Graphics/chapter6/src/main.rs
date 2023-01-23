#[macro_use]
extern crate glium;

use gl::camera;
use gl::shader;
use gl::action;
use gl::vertex;
use gl::vertex::Vertex;
use glium::Display;
use glium::Surface;
use glium::glutin::event::ElementState;



pub fn draw_parametric_curve(display: &Display) {
    let mut target = display.draw();
    target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);

    let program = shader::get_default_shader(&display);

    let mut vertex = Vec::new();

    for i in 0..100 {
        let t = i as f32 / 100.0;
        vertex.push(Vertex::new_2d(t * t - 2.0 * t + 1.0, t * t * t - 2.0 * t * t + t));
    }
    for i in 0..100 {
        let t = i as f32 / 100.0;
        vertex.push(Vertex::new_2d(t * t, t * t * t) );
    }

    let vertex_buffer = vertex::from_vertex(display, &vertex);
    let indices_buffer = glium::index::NoIndices(glium::index::PrimitiveType::LineStrip);

    let params = glium::DrawParameters {
        depth: glium::Depth {
            test: glium::draw_parameters::DepthTest::IfLess,
            write: true,
            .. Default::default()
        },
        .. Default::default()
    }; 

    let uniforms = uniform! {
        perspective: camera::CameraState::flat_perspective(),
        view: camera::CameraState::flat_view(),
        model: camera::CameraState::element_matrix()
    };

    target.draw(&vertex_buffer, &indices_buffer, &program, &uniforms, &params).unwrap();

    target.finish().unwrap();    
}

fn main() {
    #[allow(unused_imports)]
    use glium::{glutin, Surface};

    let event_loop = glutin::event_loop::EventLoop::new();
    let wb = glutin::window::WindowBuilder::new();
    let cb = glutin::ContextBuilder::new().with_depth_buffer(24);
    let display = glium::Display::new(wb, cb, &event_loop).unwrap();
    
    let mut step = 0;
    let mut t: f32 = 0.0;
    action::start_loop(event_loop, move |events| {
        let mut action = action::Action::Continue;
        if t < 1.0 { t += 0.001; } else { t = 0.0 };
        match step {
            0 => {
                draw_parametric_curve(&display);
            }
            _ => {

            }
        }
        
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