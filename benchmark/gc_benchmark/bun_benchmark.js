function benchmarkBun(iterations) {
    const start = performance.now();

    for (let i = 0; i < iterations; i++) {
        // Allocate 128 bytes on the Heap
        const churn = new Uint8Array(128);
        churn[0] = i % 255; // avoid optimization
    }

    const end = performance.now();
    return Math.floor(end - start);
}

// 1,000 cycles of 100,000 iterations
for (let x = 0; x <= 1000; x++) {
    const d = benchmarkBun(100000);
    console.log(`${x} Bun Time: ${d} ms`);
}