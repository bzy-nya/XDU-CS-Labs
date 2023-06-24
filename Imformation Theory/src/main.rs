mod compresser;

use std::time::Instant;
use compresser::Compresser;

const DATA: &str = include_str!("data");
//const DATA: &str = "1122";
const N : u64 = 1000;

fn main() {
    let frequency = compresser::count_frequency(DATA);

    println!( "------------------==Huffman test==--------------------" );
    let huffman_compresser = compresser::HuffmanCompresser::build(&frequency);
    let h1 = huffman_compresser.encode(DATA);

    assert!( huffman_compresser.decode(h1.as_str()) == DATA );
    println!( "Pass the correctness check!" );
    println!( "Everage length : {:.5} bits", h1.len() as f64 / DATA.len() as f64 );

    let start = Instant::now();
    for _ in 1..N { compresser::HuffmanCompresser::build(&frequency); }
    println!( "Build time     : {:?} ", start.elapsed() );
    let start = Instant::now();
    for _ in 1..N { huffman_compresser.encode(DATA); }
    println!( "Encode time    : {:?} ", start.elapsed() );
    let start = Instant::now();
    for _ in 1..N { huffman_compresser.decode(h1.as_str()); }
    println!( "Decode time    : {:?} ", start.elapsed() );

    println!( "------------------==Arithmetic test==--------------------" );
    let arithmetic_compresser = compresser::ArithmeticCompresser::build(&frequency);
    let h1 = arithmetic_compresser.encode(DATA);
    
    assert!( arithmetic_compresser.decode(h1.as_str()) == DATA );
    println!( "Pass the correctness check!" );
    println!( "Everage length : {:.5} bits", h1.len() as f64 / DATA.len() as f64 );

    let start = Instant::now();
    for _ in 1..N { compresser::ArithmeticCompresser::build(&frequency); }
    println!( "Build time     : {:?} ", start.elapsed() );
    let start = Instant::now();
    for _ in 1..2 { arithmetic_compresser.encode(DATA); }
    println!( "Encode time    : {:?} ", start.elapsed() );
    let start = Instant::now();
    for _ in 1..2 { arithmetic_compresser.decode(h1.as_str()); }
    println!( "Decode time    : {:?} ", start.elapsed() );

    println!( "------------------==LZ test==--------------------" );
    let lz_compresser = compresser::LzCompresser::build(&frequency);
    let h1 = lz_compresser.encode(DATA);

    assert!( lz_compresser.decode(h1.as_str()) == DATA );
    println!( "Pass the correctness check!" );
    println!( "Everage length : {:.5} bits", h1.len() as f64 / DATA.len() as f64 );

    let start = Instant::now();
    for _ in 1..N { compresser::LzCompresser::build(&frequency); }
    println!( "Build time     : {:?} ", start.elapsed() );
    let start = Instant::now();
    for _ in 1..N { lz_compresser.encode(DATA); }
    println!( "Encode time    : {:?} ", start.elapsed() );
    let start = Instant::now();
    for _ in 1..N { lz_compresser.decode(h1.as_str()); }
    println!( "Decode time    : {:?} ", start.elapsed() );
}