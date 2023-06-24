mod arithmetic;
mod huffman;
mod lz;

use std::collections::BTreeMap;

pub use arithmetic::ArithmeticCompresser;
pub use huffman::HuffmanCompresser;
pub use lz::LzCompresser;

pub trait Compresser {
    fn build(frequence: &Vec<(u8, u64)>) -> Self;
    fn encode(&self, s: &str) -> String;
    fn decode(&self, s: &str) -> String;
}

pub fn count_frequency(s: &str) -> Vec<(u8, u64)> {
    let mut m = BTreeMap::<u8, u64>::new();
    for c in s.as_bytes() {
        if !m.contains_key(&c) { m.insert(*c, 0); }
        let x = m.get(c).unwrap();
        m.insert(*c, x+1);
    }
    m.into_iter().collect()
}