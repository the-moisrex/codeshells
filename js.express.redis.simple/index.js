const express = require('express');
const redis = require('ioredis');


const run = async () => {

    const client = new redis({
        host: "localhost",
        port: 6379,
        password: ""
    });
    const app = express();
    const port = 3000;

    app.get('/', async (req, res) => {
        let index = 0;
        try {
            index = parseInt(await client.get("index"));
        } catch (ex) {
            // nothing
        }
        index++;
        try {
            await client.set("index", String(index));
        } catch (ex) {
            console.log(ex);
        }
        res.send(String(index));
    });

    app.listen(port);
    console.log(`URL: http://localhost:${port}`);

};

run();

