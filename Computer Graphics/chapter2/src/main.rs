#[macro_use]
extern crate glium;
    
use gl::camera;
use gl::shader;
use gl::action;
use gl::vertex;
use gl::vertex::Vertex;

use glium::Display;
use glium::vertex::VertexBufferAny;

fn abs(x: i32) -> i32 { if x < 0 {-x} else {x} }

fn algorithm_dda(display: &Display, begin: [i32; 2], end:[i32; 2]) -> VertexBufferAny {
    let mut ans = Vec::new();
    let step = std::cmp::max( abs(begin[0] - end[0]), abs(begin[1] - end[1]) );
    let dx = (end[0] - begin[0]) as f32 / step as f32;
    let dy = (end[1] - begin[1]) as f32 / step as f32;
    let [mut x, mut y] = [begin[0] as f32, begin[1] as f32];

    let (w, h) = display.get_framebuffer_dimensions();
    for _ in 0..step{
        ans.push( Vertex::new_2d(x/ w as f32, y / h as f32) );
        [x, y] = [x + dx, y + dy];
    }
    vertex::from_vertex(&display, &ans)
}

fn algorithm_bresenham(display: &Display, mut begin: [i32; 2], mut end:[i32; 2]) -> VertexBufferAny {
    let mut ans = Vec::new();

    let steep = abs(begin[0] - end[0]) < abs(begin[1] - end[1]);
    if steep {
        begin = [begin[1], begin[0]];
        end = [end[1], end[0]];
    }
    if begin[0] > begin[1] {
        (begin, end) = (end, begin);
    }
    let dx = end[0] - begin[0];
    let dy = abs(end[1] - begin[1]);
    let mut diff = dx / 2;
    let stepy = if end[1] < begin[1] {-1} else {1};

    let (mut x,mut y) = (begin[0], begin[1]);

    let (w, h) = display.get_framebuffer_dimensions();
    for _ in begin[0]..end[0] {
        diff -= dy;
        if diff < 0{
            y += stepy;
            diff += dx;
        }
        let (px, py) = if steep {(y, x)} else {(x, y)};
        ans.push( Vertex::new_2d(px as f32/ w as f32, py as f32 / h as f32) );
        x += 1;
    }
    vertex::from_vertex(&display, &ans)
}

fn algorithm_bresenham_circle(display: &Display, c: [i32; 2], r: i32 ) -> VertexBufferAny {
    let mut ans = Vec::new();
    let [mut x, mut y] = [0, r];
    let mut diff = 4 * r + 1;

    let (w, h) = display.get_framebuffer_dimensions();

    while x < y {
        ans.push( Vertex::new_2d((c[0] + x) as f32 / w as f32, (c[1] + y) as f32 / h as f32) );
        ans.push( Vertex::new_2d((c[0] - x) as f32 / w as f32, (c[1] + y) as f32 / h as f32) );
        ans.push( Vertex::new_2d((c[0] + x) as f32 / w as f32, (c[1] - y) as f32 / h as f32) );
        ans.push( Vertex::new_2d((c[0] - x) as f32 / w as f32, (c[1] - y) as f32 / h as f32) );
        ans.push( Vertex::new_2d((c[0] + y) as f32 / w as f32, (c[1] + x) as f32 / h as f32) );
        ans.push( Vertex::new_2d((c[0] - y) as f32 / w as f32, (c[1] + x) as f32 / h as f32) );
        ans.push( Vertex::new_2d((c[0] + y) as f32 / w as f32, (c[1] - x) as f32 / h as f32) );
        ans.push( Vertex::new_2d((c[0] - y) as f32 / w as f32, (c[1] - x) as f32 / h as f32) );

        x += 1;
        diff -= 4 * x - 2;
        if diff < 0 {
            y -= 1;
            diff += 4 * y - 2;
        }
    }
    vertex::from_vertex(&display, &ans)
}

fn main() {
    #[allow(unused_imports)]
    use glium::{glutin, Surface};

    let event_loop = glutin::event_loop::EventLoop::new();
    let wb = glutin::window::WindowBuilder::new();
    let cb = glutin::ContextBuilder::new().with_depth_buffer(24);
    let display = glium::Display::new(wb, cb, &event_loop).unwrap();

    let program = shader::get_default_shader(&display);

    let vertex_buffer1 = algorithm_dda(&display, [0, 0], [300, 500]);
    let vertex_buffer2 = algorithm_bresenham(&display, [0, -400], [300, 300]);
    let vertex_buffer3 = algorithm_bresenham_circle(&display, [0, 0], 233);
    let indices_buffer = glium::index::NoIndices(glium::index::PrimitiveType::Points);
    
    action::start_loop(event_loop, move |events| {

        let mut target = display.draw();
        target.clear_color_and_depth((0.0, 0.0, 0.0, 1.0), 1.0);

        let uniforms = uniform! {
            perspective: camera::CameraState::flat_perspective(),
            view: camera::CameraState::flat_view(),
            model: [
                [1.0, 0.0, 0.0, 0.0],
                [0.0, 1.0, 0.0, 0.0],
                [0.0, 0.0, 1.0, 0.0],
                [0.0, 0.0, 0.0, 1.0f32],
            ]
        };

        let params = glium::DrawParameters {
            depth: glium::Depth {
                test: glium::draw_parameters::DepthTest::IfLess,
                write: true,
                .. Default::default()
            },
            .. Default::default()
        };        

        target.draw(&vertex_buffer1, &indices_buffer, &program, &uniforms, &params).unwrap();
        target.draw(&vertex_buffer2, &indices_buffer, &program, &uniforms, &params).unwrap();
        target.draw(&vertex_buffer3, &indices_buffer, &program, &uniforms, &params).unwrap();

        target.finish().unwrap();
        
        let mut action = action::Action::Continue;

        // handling the events received by the window since the last frame
        for e in events {
            match e {
                glutin::event::Event::WindowEvent { event, .. } => match event {
                    glutin::event::WindowEvent::CloseRequested => action = action::Action::Stop,
                    _ => (),
                },
                _ => (),
            }
        }

        action
    });
}