console.log("Hello world")

class test {
    x = 2;
    y = 1;

    rot(r) {
        const { x, y } = this
        const s = Math.sin(r)
        const c = Math.cos(r)
        this.x = x * c - y * s
        this.y = x * s + y * c
    }
}

let t = new test();
t.rot(0);
t.rot(0);
t.rot(0);
t.rot(1);
t.rot(0);
console.log(t.x, t.y);
