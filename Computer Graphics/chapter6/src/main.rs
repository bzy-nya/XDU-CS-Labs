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
use glium::glutin::event::VirtualKeyCode;

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

type Point = [f32; 2];

fn draw_hermite_curve(display: &Display, p1: Point, p2: Point, v1: Point, v2: Point) {
    let mut target = display.draw();
    target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);

    let program = shader::get_default_shader(&display);

    let mut vertex = Vec::new();
    vertex.push(Vertex::new_2d(p1[0] + v1[0], p1[1] + v1[1]));
    for i in 0..100 {
        let t = i as f32 / 100.0;
        let t2 = t * t;
        let t3 = t * t2;
        let exp = 3.0 * t2 - 2.0 * t3;
        let x = (1.0 - exp) * p1[0] + exp * p2[0] + (t - 2.0 * t2 + t3) * v1[0] + (t3 - t2) * v2[0];
        let y = (1.0 - exp) * p1[1] + exp * p2[1] + (t - 2.0 * t2 + t3) * v1[1] + (t3 - t2) * v2[1];
        vertex.push(Vertex::new_2d(x, y));
    }
    vertex.push(Vertex::new_2d(p2[0] + v2[0], p2[1] + v2[1]));

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

    let camera = camera::CameraState::new();
    let uniforms = uniform! {
        perspective: camera.get_perspective(),
        view: camera.get_view(),
        model: camera::CameraState::element_matrix()
    };

    target.draw(&vertex_buffer, &indices_buffer, &program, &uniforms, &params).unwrap();

    target.finish().unwrap();    
}

fn draw_bezier_curve(display: &Display) {
    let p1 = [0.0, 0.0, 0.0f32];
    let p2 = [1.0, 1.0, 1.0f32];
    let p3 = [2.0, -1.0, -1.0f32];
    let p4 = [3.0, 0.0, 0.0f32];

    let mut target = display.draw();
    target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);

    let program = shader::get_default_shader(&display);

    let mut vertex = Vec::new();
    
    let indices_buffer = glium::index::NoIndices(glium::index::PrimitiveType::LineStrip);

    let params = glium::DrawParameters {
        depth: glium::Depth {
            test: glium::draw_parameters::DepthTest::IfLess,
            write: true,
            .. Default::default()
        },
        .. Default::default()
    }; 

    let mut camera = camera::CameraState::new();
    camera.set_position((0.0, 0.0, -3.0));
    camera.set_direction((0.0, 0.0, 1.0));
    let uniforms = uniform! {
        perspective: camera.get_perspective(),
        view: camera.get_view(),
        model: camera::CameraState::element_matrix()
    };

    vertex.push(Vertex::new_3d_point(p1[0], p1[1], p1[2]));
    vertex.push(Vertex::new_3d_point(p2[0], p2[1], p2[2]));
    vertex.push(Vertex::new_3d_point(p3[0], p3[1], p3[2]));
    vertex.push(Vertex::new_3d_point(p4[0], p4[1], p4[2]));
    let vertex_buffer = vertex::from_vertex(display, &vertex);
    target.draw(&vertex_buffer, &indices_buffer, &program, &uniforms, &params).unwrap();
    
    vertex.clear();
    for i in 0..1000 {
        let t = i as f32 / 1000.0;
        let x = (1.0 - t).powi(3) * p1[0] + 3.0 * (1.0 - t).powi(2) * t * p2[0] + 3.0 * (1.0 - t) * t * t * p3[0] + t * t * t * p4[0];
        let y = (1.0 - t).powi(3) * p1[1] + 3.0 * (1.0 - t).powi(2) * t * p2[1] + 3.0 * (1.0 - t) * t * t * p3[1] + t * t * t * p4[1];
        let z = (1.0 - t).powi(3) * p1[2] + 3.0 * (1.0 - t).powi(2) * t * p2[2] + 3.0 * (1.0 - t) * t * t * p3[2] + t * t * t * p4[2];
        vertex.push(Vertex::new_3d_point(x, y, z));
    }
    let vertex_buffer = vertex::from_vertex(display, &vertex);
    target.draw(&vertex_buffer, &indices_buffer, &program, &uniforms, &params).unwrap();

    target.finish().unwrap();    
}

fn draw_bezier_surface(display: &Display) {
    let p = [
        [ [-2.0, -1.0, 0.0], [-2.0, -1.0, 4.0], [2.0, -1.0, 4.0], [2.0, -1.0, 0.0] ],
        [ [-3.0, 0.0, 0.0], [-3.0, 0.0, 6.0], [3.0, 0.0, 6.0], [3.0, 0.0, 0.0] ],
        [ [-1.5, 0.5, 0.0], [-1.5, 0.5, 3.0], [1.5, 0.5, 3.0], [1.5, 0.5, 0.0] ],
        [ [-2.0, 1.0, 0.0], [-2.0, 1.0, 4.0], [2.0, 1.0, 4.0], [2.0, 1.0, 0.0f32] ]
    ];
    
    let mut target = display.draw();
    target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);

    let program = shader::get_default_shader(&display);

    let mut vertex = Vec::new();
    
    let indices_buffer = glium::index::NoIndices(glium::index::PrimitiveType::Points);

    let params = glium::DrawParameters {
        depth: glium::Depth {
            test: glium::draw_parameters::DepthTest::IfLess,
            write: true,
            .. Default::default()
        },
        .. Default::default()
    }; 

    let mut camera = camera::CameraState::new();
    camera.set_position((0.0, 0.0, 5.0));
    camera.set_direction((0.0, 0.0, -1.0));
    let uniforms = uniform! {
        perspective: camera.get_perspective(),
        view: camera.get_view(),
        model: camera::CameraState::element_matrix()
    };

    for i in 0..1000 {
        let t = i as f32 / 1000.0;
        let mut pp = [[0.0; 3]; 4];
        for x in 0..4 {
            pp[x] = [
                (1.0 - t).powi(3) * p[x][0][0] + 3.0 * (1.0 - t).powi(2) * t * p[x][1][0] + 3.0 * (1.0 - t) * t * t * p[x][2][0] + t * t * t * p[x][3][0],
                (1.0 - t).powi(3) * p[x][0][1] + 3.0 * (1.0 - t).powi(2) * t * p[x][1][1] + 3.0 * (1.0 - t) * t * t * p[x][2][1] + t * t * t * p[x][3][1],
                (1.0 - t).powi(3) * p[x][0][2] + 3.0 * (1.0 - t).powi(2) * t * p[x][1][2] + 3.0 * (1.0 - t) * t * t * p[x][2][2] + t * t * t * p[x][3][2]    
            ]
        }
        for j in 0..100 {
            let t = j as f32 / 100.0;
            let x = (1.0 - t).powi(3) * pp[0][0] + 3.0 * (1.0 - t).powi(2) * t * pp[1][0] + 3.0 * (1.0 - t) * t * t * pp[2][0] + t * t * t * pp[3][0];
            let y = (1.0 - t).powi(3) * pp[0][1] + 3.0 * (1.0 - t).powi(2) * t * pp[1][1] + 3.0 * (1.0 - t) * t * t * pp[2][1] + t * t * t * pp[3][1];
            let z = (1.0 - t).powi(3) * pp[0][2] + 3.0 * (1.0 - t).powi(2) * t * pp[1][2] + 3.0 * (1.0 - t) * t * t * pp[2][2] + t * t * t * pp[3][2];
            vertex.push(Vertex::new_3d_point(x, y, z));
        }
    }

    let vertex_buffer = vertex::from_vertex(display, &vertex);
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
        if t < 1.0 { t += 0.01; } else { t = 0.0 };
        match step {
            0 => draw_parametric_curve(&display),
            1 => draw_hermite_curve(&display, [0.0, 1.0], [3.0, 0.0], [-t, 1.0 - t], [-3.0 + t * 3.0, t * 1.0]),
            2 => draw_bezier_curve(&display),
            _ => draw_bezier_surface(&display)
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