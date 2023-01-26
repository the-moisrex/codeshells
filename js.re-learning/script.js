const arr = [1, 2, 3, 4];
const [a, b, ...others] = arr;
console.log(a, b, others, ...others);

const people = {
    moisrex: {
        username: 'moisrex',
        email: 'moisrex@example.com'
    },
    mili: {
        username: 'mili',
        email: 'mili@example.com'
    }
}

const { moisrex, ...other_users } = people;
console.log(moisrex);
console.log(other_users);


function add(left, right, ...rest) {
    return left + right + (!rest.length ? 0 : add(...rest));
}

console.log(add(...arr));


// nullish
console.log(people.google ?? "no google");

// or assignment operator
people.google ||= {
    username: "google",
    email: "google@gmail.com",
    corpName: undefined
}

// nullish assignment operator
people.google.corpName ??= "Google Ins.";
console.log(people.google);

// and assignment operator
for (let person in people) {
    people[person].email &&= people[person].email
        .replace('@', ' [at] ')
        .replace('.', ' [dot] ');
}
console.log(people)


// getting an iterator of [index, value]
console.log(arr.entries())
for (const [index, item] of arr.entries()) {
    console.log(index, item);
}

const ob1 = {
    // ES6 Enhanced Object Literals
    arr,

    print() {
        console.log(this);
    },

    [`day-${2 + 2}`]: 'cool day'
};

ob1.print();
