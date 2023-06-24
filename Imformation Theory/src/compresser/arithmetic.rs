use std::collections::BTreeMap;
use num_bigint::ToBigUint;

pub struct ArithmeticCompresser {
    segments: BTreeMap<u8, (u64, u64)>,
    length : u64,
    n: u64
}

impl super::Compresser for ArithmeticCompresser {
    fn build(frequence: &Vec<(u8, u64)>) -> Self {
        let mut sum = 0u64;
        let mut ret = BTreeMap::new();
        for (x, y) in frequence {
            ret.insert(*x, (sum, sum + y));
            sum += y;
        }
        Self{ segments: ret, length: sum, n: 6468 }
    }

    fn decode(&self, s: &str) -> String {
        let mut ans = String::new();
        let mut t = self.length.to_biguint().unwrap().pow(self.n as u32);
        let mut res = 0.to_biguint().unwrap();

        let mut bit = 1u64;
        for &c in s.as_bytes() {
            if c == 49u8 { res += &t >> bit; }
            bit += 1;
        }
        let mut l = 0.to_biguint().unwrap();
        let mut w = 1.to_biguint().unwrap();
        for _ in 0..self.n {
            t /= self.length.to_biguint().unwrap();
            let p = (&res - &l) / (&w * &t);
            for (c, (x,y)) in self.segments.iter() {
                if x.to_biguint().unwrap() <= p && y.to_biguint().unwrap() > p { 
                    l = &l + &t * &w * x; 
                    w *= y - x; 
                    ans.push(*c as char); 
                    break; 
                }
            }
        }
        ans
    }

    fn encode(&self, s: &str) -> String {
        let mut ans = String::new();
        let mut l = 0.to_biguint().unwrap();
        let mut r = 1.to_biguint().unwrap();
        let mut t = 1.to_biguint().unwrap();
        for c in s.as_bytes() {
            let (x, y) = self.segments.get(c).unwrap();
            let diff = &r - &l;
            l *= self.length.to_biguint().unwrap(); l += x.to_biguint().unwrap() * &diff;
            r *= self.length.to_biguint().unwrap(); r -= (self.length.to_biguint().unwrap() - y.to_biguint().unwrap() ) * &diff;
            t *= self.length.to_biguint().unwrap();
        }
        let mut sum = 0.to_biguint().unwrap();
        let mut bit = 1u64;
        while &sum < &l {
            let tmp = &sum + (&t >> bit);
            if tmp == 0.to_biguint().unwrap() {break;}
            if tmp < r { sum = tmp; ans += "1"; }
            else { ans += "0"; }
            bit += 1;
        }
        ans
    }
}