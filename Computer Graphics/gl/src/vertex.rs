use glium::{self, Display};
use glium::vertex::VertexBufferAny;
use obj::{ObjData, IndexTuple};

#[derive(Copy, Clone)]
pub struct Vertex {
    position: [f32; 3],
    normal: [f32; 3],
    texture: [f32; 2],
}
implement_vertex!(Vertex, position, normal, texture);

impl Vertex {
    pub fn new_2d(x: f32, y:f32) -> Vertex {
        Vertex {
            position: [x, y, 0.0],
            normal: [0.0, 0.0, 0.0],
            texture: [0.0, 0.0]
        }
    }

    pub fn new_3d_point(x: f32, y: f32, z: f32) -> Vertex {
        Vertex {
            position: [x, y, z],
            normal: [0.0, 0.0, 0.0],
            texture: [0.0, 0.0]
        }
    }

    pub fn new_3d(position: [f32; 3], normal: [f32; 3]) -> Vertex {
        Vertex { 
            position, 
            normal, 
            texture: [0.0, 0.0] 
        }
    }
}

pub fn load_wavefront(display: &Display, data: &[u8]) -> VertexBufferAny {
    let mut data = std::io::BufReader::new(data);
    let data = obj::ObjData::load_buf(&mut data).unwrap();

    let mut vertex_data = Vec::new();

    fn get_vertex(data: &ObjData, args: IndexTuple) -> Vertex {
        let position = data.position[args.0];
        let texture = args.1.map(|index| data.texture[index]);
        let normal = args.2.map(|index| data.normal[index]);

        let texture = texture.unwrap_or([0.0, 0.0]);
        let normal = normal.unwrap_or([0.0, 0.0, 0.0]);

        Vertex {
            position,
            normal,
            texture,
        }
    }

    for object in data.objects.iter() {
        for polygon in object.groups.iter().flat_map(|g| g.polys.iter()) {
            match polygon {
                obj::SimplePolygon(indices) => {
                    for i in 2..indices.len() {
                        vertex_data.push( get_vertex(&data, indices[0]) );
                        vertex_data.push( get_vertex(&data, indices[i - 1]) );
                        vertex_data.push( get_vertex(&data, indices[i]) );
                    }
                },
            }
        }
    }

    glium::vertex::VertexBuffer::new(display, &vertex_data).unwrap().into()
}

pub fn from_vertex(display: &Display, vertex: &Vec<Vertex>) -> VertexBufferAny {
    glium::vertex::VertexBuffer::new(display, vertex).unwrap().into()
}