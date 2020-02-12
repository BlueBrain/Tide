import Moves from "./moves"

// Mock for Date.now().
const DateBackup = Date;
window.Date = {
    time: 0,
    now() { return this.time; }
};
afterAll(() => {
    window.Date = DateBackup;
});


describe("Class Moves", () => {
    function check(moves) {
        window.Date.time = 1;
        moves.add(15, 12);
        window.Date.time = 2;
        moves.add(25, 10);
        window.Date.time = 4;
        moves.add(30, 15);
        it("should compute location X", () => { expect(moves.x).toBe(30); });
        it("should compute location Y", () => { expect(moves.y).toBe(15); });
        it("should compute start X", () => { expect(moves.startX).toBe(10); });
        it("should compute start Y", () => { expect(moves.startY).toBe(10); });
        it("should compute speed X", () => { expect(moves.speedX).toBeCloseTo(2.5, 6); });
        it("should compute speed Y", () => { expect(moves.speedY).toBeCloseTo(2.5, 6); });
        it("should compute accel X", () => { expect(moves.accelX).toBeCloseTo(-3.75, 6); });
        it("should compute accel Y", () => { expect(moves.accelY).toBeCloseTo(2.25, 6); });
        window.Date.time = 5;
        it("should compute elapsedTime", () => { expect(moves.elapsedTime).toEqual(5) });
    }

    describe("new Moves(10,10)", () => {
        window.Date.time = 0;
        const moves = new Moves(10, 10);
        //moves.add(Math.random(), Math.random());
        check(moves);
        describe("then init(10,10)", () => {
            window.Date.time = 0;
            moves.init(10, 10);
            check(moves);
        });
    });

    describe("new Moves(10,10) + random add(...)", () => {
        window.Date.time = 0;
        const moves = new Moves(10, 10);
        moves.add(Math.random(), Math.random());
        check(moves);
        describe("then init(10,10)", () => {
            window.Date.time = 0;
            moves.init(10, 10);
            moves.add(Math.random(), Math.random());
            check(moves);
        });
    });

    describe("new Moves(10,10) + random add(...)", () => {
        window.Date.time = 0;
        const moves = new Moves(10, 10);
        moves.add(Math.random(), Math.random());
        check(moves);
        describe("then init(10,10)", () => {
            window.Date.time = 0;
            moves.init(10, 10);
            moves.add(Math.random(), Math.random());
            check(moves);
        });
    });

    describe("new Moves(10,10) + 2 randoms add(...)", () => {
        window.Date.time = 0;
        const moves = new Moves(10, 10);
        moves.add(Math.random(), Math.random());
        moves.add(Math.random(), Math.random());
        check(moves);
        describe("then init(10,10)", () => {
            window.Date.time = 0;
            moves.init(10, 10);
            moves.add(Math.random(), Math.random());
            moves.add(Math.random(), Math.random());
            check(moves);
        });
    });
});
