mod math;
mod user;

fn main() {
    let x = 2;
    let y = 3;
    println!("Hello, world {} times!", math::mul(x, y));

    let clair = user::User {
        name: "Clair".to_string(),
        email: "sister.clair@gmail.com".to_string(),
        birthyear: 1999,
    };

    clair.print();
}
