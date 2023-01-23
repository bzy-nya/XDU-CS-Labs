use glium::{index::{IndexBufferAny, PrimitiveType}, Display, IndexBuffer};

pub fn from_triangles(display: &Display, triangles: &Vec<u32>) -> IndexBufferAny {
    return IndexBuffer::new(display, PrimitiveType::TrianglesList, triangles).unwrap().into()
}

pub fn from_rectangles(display: &Display, rectangles: &Vec<[u32; 4]>) -> IndexBufferAny {
    let mut triangles = Vec::new();
    for x in rectangles {
        let mut new_triangles = vec![x[0], x[1], x[2], x[2], x[3], x[0]];
        triangles.append(&mut new_triangles);
    }
    return IndexBuffer::new(display, PrimitiveType::TrianglesList, &triangles).unwrap().into()
}