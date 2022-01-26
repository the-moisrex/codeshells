pub struct User {
    pub name: String,
    pub email: String,
    pub birthyear: u16,
}

impl User {
    pub fn age(&self) -> u8 {
        (2022 as u16 - self.birthyear) as u8
    }

    pub fn print(&self) {
        println!("{} ({}) is {}", self.name, self.email, self.age());
    }
}
