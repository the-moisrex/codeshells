function packBoolsIntoInts(boolArray, blockSize = 8n) {
    const result = [];
    const len = BigInt(boolArray.length);
    const lastBit = 0b1n << (blockSize - 1n);
    let currentInt = 0n;

    let bit = 0b1n;
    for (const curBit of boolArray) {
        if (curBit === true) {
            currentInt |= bit;
        } else if (curBit !== false) {
            throw new Error(`Invalid boolean: ${curBit}`);
        }

        // If we've packed 8 booleans, push the current integer to the result
        if (bit === lastBit) {
            result.push(currentInt);
            currentInt = 0n; // Reset for the next integer
            bit = 0b1n;
        } else {
            bit <<= 1n;
        }
    }

    // If there are remaining booleans that don't fill a complete byte
    if (len % blockSize !== 0n) {
        result.push(currentInt);
    }

    return result;
}

console.log(packBoolsIntoInts([true, true, true, true, true, true, false, true], 32n).map(item => item.toString(2)));
