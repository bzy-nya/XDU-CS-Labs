use glium::{Display, vertex::VertexBufferAny};

use crate::vertex::load_wavefront;

pub fn cube(display: &Display) -> VertexBufferAny {
    load_wavefront(display, include_bytes!("cube.obj"))
}

pub fn teapot(display: &Display) -> VertexBufferAny {
    load_wavefront(display, include_bytes!("teapot.obj"))
}