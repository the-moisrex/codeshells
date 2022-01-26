extern crate users;
extern crate regex;

fn main() {
    let usr = users::User {
        firstname: "Jane".to_string(),
        lastname: "Doe".to_string(),
        username: "janedoe".to_string()
    };
    println!("{}, {}", usr.firstname, usr.lastname);
}
