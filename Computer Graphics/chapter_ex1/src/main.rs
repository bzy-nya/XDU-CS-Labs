#[macro_use]
extern crate glium;

use gl::camera;
use gl::shader;
use gl::action;
use gl::vertex;
use glium::glutin::event::ElementState;
use glium::glutin::event::VirtualKeyCode;

fn main() {
    #[allow(unused_imports)]
    use glium::{glutin, Surface};

    let event_loop = glutin::event_loop::EventLoop::new();
    let wb = glutin::window::WindowBuilder::new();
    let cb = glutin::ContextBuilder::new().with_depth_buffer(24);
    let display = glium::Display::new(wb, cb, &event_loop).unwrap();

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

    let program = shader::get_default_shader(&display);

    let mut vertex = Vec::new();
    let mut fill = false;
    let mut pos = [0.0, 0.0f32];
    action::start_loop(event_loop, move |events| {
        let mut target = display.draw();
        target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);

        let vertex_buffer = vertex::from_vertex(&display, &vertex);

        let indices_buffer = if fill {
            glium::index::NoIndices(glium::index::PrimitiveType::TriangleFan)
        } else {
            glium::index::NoIndices(glium::index::PrimitiveType::LineStrip)
        };

        target.draw(&vertex_buffer, &indices_buffer, &program, &uniforms, &params).unwrap();
        target.finish().unwrap();
        
        let mut action = action::Action::Continue;

        for e in events {
            match e {
                glutin::event::Event::WindowEvent { event, .. } => match event {
                    glutin::event::WindowEvent::CloseRequested => 
                        { action = action::Action::Stop; },
                    glutin::event::WindowEvent::KeyboardInput { input, .. } 
                     => match input.state {
                            ElementState::Pressed => {
                                match input.virtual_keycode {
                                    Some(VirtualKeyCode::Escape) => {action = action::Action::Stop;}
                                    Some(VirtualKeyCode::Key1) => {vertex.pop();}
                                    Some(VirtualKeyCode::F) => { if vertex.len() >= 1 {vertex.remove(0);} }
                                    Some(VirtualKeyCode::Space) => { fill = !fill; }
                                    _ => {}
                                }
                            }
                            _ => {} 
                        }
                    glutin::event::WindowEvent::CursorMoved { position, .. }
                     => { let (h, w) = display.get_framebuffer_dimensions();
                            pos = [position.x as f32 * 2.0 / h as f32 - 1.0, 1.0 - position.y as f32 * 2.0/ w as f32]; }
                    glutin::event::WindowEvent::MouseInput { state, .. } 
                     => match state {
                            ElementState::Pressed => {
                                vertex.push( vertex::Vertex::new_2d(pos[0], pos[1]) );
                            }
                            _ => {}
                        }
                    _ => (),
                },
                _ => ()
            }
        }

        action
    });
}