
pub struct User {
    pub firstname: String,
    pub lastname: String,
    pub username: String
}



#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        let result = 2 + 2;
        assert_eq!(result, 4);
    }
}
