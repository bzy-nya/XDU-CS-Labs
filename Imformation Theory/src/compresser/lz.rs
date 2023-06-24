use std::collections::BTreeMap;

pub struct LzCompresser {}

impl super::Compresser for LzCompresser {
    fn build(_: &Vec<(u8, u64)>) -> Self { Self {} }

    fn decode(&self, s: &str) -> String {
        let mut ans = String::new();
        let block = 18;

        let mut vec = vec![String::from("")];

        for i in 0..s.len() / block {
            let x = i * block;
            let y = (i + 1) * block - 7;
            let z = (i + 1) * block;

            let pre  = usize::from_str_radix(&s[x..y], 2).unwrap();
            let c = usize::from_str_radix(&s[y..z], 2).unwrap();

            let mut str = vec[pre].clone();
            str.push(c as u8 as char);

            ans += str.as_str();

            vec.push(str);
        }
        
        ans
    }

    fn encode(&self, s: &str) -> String {
        let mut ans = String::new();
        let mut map = BTreeMap::<String, usize>::new();

        map.insert(String::new(), 0);

        let mut now = String::new();
        let mut pre = 0usize;
        let mut cnt = 0;

        for &c in s.as_bytes() {
            now.push(c as char);
            if !map.contains_key(&now) {
                cnt += 1;
                ans += format!("{:0>11b}",pre).as_str();
                ans += format!("{:0>7b}",c).as_str();
                map.insert(now, cnt);
                now = String::new();
            }
            pre = *map.get(&now).unwrap();
        }

        ans
    }
}