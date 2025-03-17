function getSplitPoints(table, getValue = (val) => val) {
    let last = NaN;
    let start = 0;
    let tables = [];
    let i = 0;
    for (; i < table.length; ++i) {
        const cur = table.at(i);
        const curVal = getValue(cur);
        if (last !== curVal) {
            const length = i - start;
            if (length > 0) {
                tables.push({ start, length, commonValue: last });
            }
            start = i;
        }
        last = curVal;
    }
    const length = i - start;
    if (length > 0) {
        tables.push({ start, length, commonValue: last });
    }
    return tables.toSorted(({ length: lhs }, { length: rhs }) => lhs >= rhs);
}

function splitOn(table, splits = getSplitPoints(table)) {
    let tables = [];
    for (const { start, length, commonValue } of splits) {
        tables.push({
            start,
            length,
            commonValue,
            table: table.slice(start, start + length)
        });
    }
    return tables;
}


let table = [1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4];

console.log(table);
console.log(splitOn(table));
