#[macro_use]
extern crate glium;

use gl::camera;
use gl::shader;
use gl::action;
use gl::models;
use gl::vertex;
use gl::vertex::Vertex;
use glium::Display;
use glium::Surface;
use glium::glutin::event::ElementState;

pub fn draw_cube_stretch(display: &Display, t: f32) {
    let program = shader::get_default_shader(&display);
    let camera = camera::CameraState::new();

    let vertex_buffer = models::cube(&display);
    let indices_buffer = glium::index::NoIndices(glium::index::PrimitiveType::TrianglesList);

    let params = glium::DrawParameters {
        depth: glium::Depth {
            test: glium::draw_parameters::DepthTest::IfLess,
            write: true,
            .. Default::default()
        },
        .. Default::default()
    }; 

    let uniforms = uniform! {
        perspective: camera.get_perspective(),
        view: camera.get_view(),
        model: [
            [1.0 + t, 0.0, 0.0, 0.0],
            [0.0, 1.0, 0.0, 0.0],
            [0.0, 0.0, 1.0 - 0.5 * t, 0.0],
            [-10.0 * t, 0.0, 5.0 * t, 1.0f32],
        ]
    };

    let mut target = display.draw();
    target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);
    target.draw(&vertex_buffer, &indices_buffer, &program, &uniforms, &params).unwrap();
    target.finish().unwrap();
}

fn draw_rectangle_flip(display: &Display, rectangle: [i32; 4]) {
    let program = shader::get_default_shader(&display);
    let (h, w) = display.get_framebuffer_dimensions();
    let vertex = vec![
        Vertex::new_2d(rectangle[0] as f32 / h as f32, rectangle[1] as f32 / w as f32),
        Vertex::new_2d(rectangle[2] as f32 / h as f32, rectangle[1] as f32 / w as f32),
        Vertex::new_2d(rectangle[2] as f32 / h as f32, rectangle[3] as f32 / w as f32),
        Vertex::new_2d(rectangle[0] as f32 / h as f32, rectangle[3] as f32 / w as f32),
    ];
    let vertex_buffer = vertex::from_vertex(display, &vertex);
    let indices_buffer = glium::index::NoIndices(glium::index::PrimitiveType::LineLoop);
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
        model: [
            [0.0, 1.0, 0.0, 0.0],
            [1.0, 0.0, 0.0, 0.0],
            [0.0, 0.0, 1.0, 0.0],
            [-5.0 / w as f32, 5.0 / h as f32, 0.0, 1.0f32],
        ]
    };

    let mut target = display.draw();
    target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);
    target.draw(&vertex_buffer, indices_buffer, &program, &uniforms, &params).unwrap();

    target.finish().unwrap();
}

fn draw_triangle_step(display: &Display, step: i32, t : f32) {
    let program = shader::get_default_shader(&display);
    let vertex = vec![
        Vertex::new_2d( -0.5, 0.5 ),
        Vertex::new_2d( 0.0, -0.5 ),
        Vertex::new_2d( 0.5, 0.5 ),
    ];
    let vertex_buffer = vertex::from_vertex(display, &vertex);
    let indices_buffer = match step {
        2 => glium::index::NoIndices(glium::index::PrimitiveType::Points),
        3 => glium::index::NoIndices(glium::index::PrimitiveType::LineLoop),
        _ => glium::index::NoIndices(glium::index::PrimitiveType::TrianglesList),
    };
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
        model: [
            [t.sin(), t.cos(), 0.0, 0.0],
            [t.cos(), -t.sin(), 0.0, 0.0],
            [0.0, 0.0, 1.0, 0.0],
            [0.0, 0.0, 0.0, 1.0f32],
        ]
    };

    let mut target = display.draw();
    target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);
    target.draw(&vertex_buffer, indices_buffer, &program, &uniforms, &params).unwrap();

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
                draw_cube_stretch(&display, t);
            }
            1 => draw_rectangle_flip(&display, [0, 100, 50, 0]),
            _ => draw_triangle_step(&display, step, t * 6.28),
        }
        
        // handling the events received by the window since the last frame
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