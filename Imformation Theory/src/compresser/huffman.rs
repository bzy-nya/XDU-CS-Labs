use std::collections::BinaryHeap;

pub struct HuffmanCompresser {
    trie: [[usize; 2]; 256],
    dict: [String; 128]
}

impl HuffmanCompresser {
    fn dfs(&mut self, x: usize, s: String) {
        if x < 128 { self.dict[x] = s; }
        else { self.dfs(self.trie[x][0], s.clone() + "0" ); self.dfs(self.trie[x][1], s + "1" ); }
    }
}

impl super::Compresser for HuffmanCompresser {
    fn build(frequence: &Vec<(u8, u64)>) -> Self {
        let mut trie = [[0; 2]; 256];
        let dict = [(); 128].map(|_| String::new() );

        let mut x = 127;
        
        let mut que : BinaryHeap<_> = frequence.iter()
            .map( |(p, q)| (- (*q as i64), *p) ).collect();

        while que.len() >= 2 {
            let u = que.pop().unwrap();
            let v = que.pop().unwrap();

            x += 1;

            trie[x][0] = u.1 as usize;
            trie[x][1] = v.1 as usize;

            que.push((u.0 + v.0, x as u8) );
        }

        trie[255] = trie[x];

        let mut ret = Self { trie, dict };

        ret.dfs(255, String::new());

        ret
    }

    fn encode(&self, s: &str) -> String {
        s.as_bytes().iter().map(|&c| &self.dict[c as usize] ).cloned().collect()
    }

    fn decode(&self, s: &str) -> String {
        let mut ans = String::new();
        let mut x = 255;
        for &u in s.as_bytes() {
            x = self.trie[x][u as usize - 48];
            x = if x <= 127 { ans.push(x as u8 as char); 255 } else {x}
        }
        ans
    }
}