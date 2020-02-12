const CAPACITY = 3;

/**
 * Helper to manage a pointer move.
 * It gives you the location, the speed and the acceleration of your pointer.
 *
 * @type {[type]}
 */
export default class Moves {
    private _x0: number;
    private _y0: number;
    private _time0: number;
    private _x: number[] = new Array(CAPACITY);
    private _y: number[] = new Array(CAPACITY);
    private _time: number[] = new Array(CAPACITY);
    private _cursor: number = 0;

    constructor(x: number = 0, y: number = 0) {
        this.init(x, y);
    }

    init(x: number, y: number) {
        this._time0 = this.now;
        this._x0 = x;
        this._y0 = y;
        this._x = this._x.fill(this._x0, 0, CAPACITY);
        this._y = this._y.fill(this._y0, 0, CAPACITY);
        this._time = this._time.fill(this._time0, 0, CAPACITY);
    }

    _index(shift: number) {
        return (this._cursor + shift) % CAPACITY;
    }

    get now() { return Date.now(); }

    add(x: number, y: number) {
        const idx = (this._cursor + CAPACITY - 1) % CAPACITY;
        this._time[idx] = this.now;
        this._x[idx] = x;
        this._y[idx] = y;
        this._cursor = idx;
    }

    get elapsedTime() { return this.now - this._time0; }

    get x() { return this._x[this._cursor]; }
    get y() { return this._y[this._cursor]; }

    get startX() { return this._x0; }
    get startY() { return this._y0; }

    get speedX() {
        const idx0 = this._cursor;
        const idx1 = this._index(1);
        const time = this._time[idx0] - this._time[idx1];
        if (time <= 0) return 0;
        return (this._x[idx0] - this._x[idx1]) / time;
    }

    get speedY() {
        const idx0 = this._cursor;
        const idx1 = this._index(1);
        const time = this._time[idx0] - this._time[idx1];
        if (time <= 0) return 0;
        return (this._y[idx0] - this._y[idx1]) / time;
    }

    get accelX() {
        const idx0 = this._index(0);
        const time0 = this._time[idx0];

        const idx1 = this._index(1);
        const time1 = this._time[idx1];

        const time01 = time0 - time1;

        if (time01 <= 0) return 0;

        const idx2 = this._index(2);
        const time2 = this._time[idx2];

        const time12 = time1 - time2;

        if (time12 <= 0) return 0;

        const x0 = this._x[idx0];
        const x1 = this._x[idx1];
        const x2 = this._x[idx2];

        const speed01 = (x0 - x1) / time01;
        const speed12 = (x1 - x2) / time12;

        return (speed01 - speed12) / time01;
    }

    get accelY() {
        const idx0 = this._index(0);
        const time0 = this._time[idx0];

        const idx1 = this._index(1);
        const time1 = this._time[idx1];

        const time01 = time0 - time1;

        if (time01 <= 0) return 0;

        const idx2 = this._index(2);
        const time2 = this._time[idx2];

        const time12 = time1 - time2;

        if (time12 <= 0) return 0;

        const y0 = this._y[idx0];
        const y1 = this._y[idx1];
        const y2 = this._y[idx2];

        const speed01 = (y0 - y1) / time01;
        const speed12 = (y1 - y2) / time12;

        return (speed01 - speed12) / time01;
    }
}
