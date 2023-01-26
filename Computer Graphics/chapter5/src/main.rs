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
use glium::glutin::event::VirtualKeyCode;

fn matrix_mutiple(x: [[f32; 4]; 4], y: [[f32; 4]; 4]) -> [[f32; 4]; 4] {
    let mut ret = [
        [0.0, 0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0, 0.0f32],
    ];
    for i in 0..4 {
        for j in 0..4 {
            for k in 0..4 {
                ret[i][j] += x[i][k] * y[k][j];
            }
        }
    }
    ret
}

pub fn draw_cube_move(display: &Display,t: f32) {
    let mut target = display.draw();
    target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);

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
            [1.0, 0.0, 0.0, 0.0],
            [0.0, 1.0, 0.0, 0.0],
            [0.0, 0.0, 1.0, 0.0],
            [-2.0, -1.5, 10.0 * t -3.0, 1.0f32],
        ]
    };

    target.draw(&vertex_buffer, &indices_buffer, &program, &uniforms, &params).unwrap();

    let program = shader::get_default_shader(&display);
    let camera = camera::CameraState::new();

    let vertex = vec![
        Vertex::new_3d_point(-1.0, -0.5, -10.0),
        Vertex::new_3d_point(-1.0, -0.5, 1000.0),
        Vertex::new_3d_point(-3.0, -0.5, -10.0),
        Vertex::new_3d_point(-3.0, -0.5, 1000.0),
        Vertex::new_3d_point(-1.0, -2.5, -10.0),
        Vertex::new_3d_point(-1.0, -2.5, 1000.0),
        Vertex::new_3d_point(-3.0, -2.5, -10.0),
        Vertex::new_3d_point(-3.0, -2.5, 1000.0),
    ];
    let vertex_buffer = vertex::from_vertex(display, &vertex);
    let indices_buffer = glium::index::NoIndices(glium::index::PrimitiveType::LinesList);

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
        model: camera::CameraState::element_matrix()
    };

    target.draw(&vertex_buffer, &indices_buffer, &program, &uniforms, &params).unwrap();

    target.finish().unwrap();    
}

fn draw_cube_rotate(display: &Display, t: f32) {
    let mut target = display.draw();
    target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);

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

    let t = t*6.28;
    let uniforms = uniform! {
        perspective: camera.get_perspective(),
        view: camera.get_view(),
        model: matrix_mutiple (
        [
            [1.0, 0.0, 0.0, 0.0],
            [0.0, 1.0, 0.0, 0.0],
            [0.0, 0.0, 1.0, 0.0],
            [0.0, -2.0, -2.0, 1.0],
        ],
        [
            [1.0*t.sin(), 0.0, 1.0*t.cos(), 0.0],
            [0.0, 1.0, 0.0, 0.0],
            [1.0*t.cos(), 0.0, 1.0*-t.sin(), 0.0],
            [0.0, 0.0, 0.0, 1.0f32],
        ]
        )
    };

    target.draw(&vertex_buffer, &indices_buffer, &program, &uniforms, &params).unwrap();

    let program = shader::get_default_shader(&display);
    let camera = camera::CameraState::new();

    let vertex = vec![
        Vertex::new_3d_point(0.0, -2.0, 0.0)
    ];
    let vertex_buffer = vertex::from_vertex(display, &vertex);
    let indices_buffer = glium::index::NoIndices(glium::index::PrimitiveType::Points);

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
                draw_cube_move(&display, t);
            }
            _ => {
                draw_cube_rotate(&display, t);

            }
        }
        
        for e in events {
            match e {
                glutin::event::Event::WindowEvent { event, .. } => match event {
                    glutin::event::WindowEvent::CloseRequested => 
                        { action = action::Action::Stop; },
                    glutin::event::WindowEvent::KeyboardInput { device_id: _, input, is_synthetic:_ } =>
                        { match input.state {
                            ElementState::Pressed => {
                                match input.virtual_keycode {
                                    Some(VirtualKeyCode::Space) => {step += 1;}
                                    _ => {}
                                }
                            }
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