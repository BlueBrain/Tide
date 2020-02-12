export default { elementAt, vertexArrayToAttribs, fillPolyline };

interface IVector2 {
    x: number,
    y: number
}

/**
 * Sometimes, your polygon  is expressed in 3 dimensions.   So, if you
 * want to use `fillPolyline`, this function will flatten the polygon
 * for you and project it onto a plane.
 * [[x1,y1,z1], [x2,y2,z2], ...]  =>  new Float32Array([xx1, yy1, xx2, yy2, ...])
 */
function vertexArrayToAttribs(vectors3: [number, number, number][]): Float32Array {
    try {
        const output: number[] = [];
        if (vectors3.length < 3)
            throw "[webgl/fillpoly.vertexArrayToAttribs] A polyline must have at least 3 vertices!";

        const xI = vectors3[1][0] - vectors3[0][0];
        const yI = vectors3[1][1] - vectors3[0][1];
        const zI = vectors3[1][2] - vectors3[0][2];

        // Look for a vector J that is not colinear to vector I.
        let xJ = 0;
        let yJ = 0;
        let zJ = 0;
        for (let k = 2; k < vectors3.length; k++) {
            xJ = vectors3[k][0] - vectors3[0][0];
            yJ = vectors3[k][1] - vectors3[0][1];
            zJ = vectors3[k][2] - vectors3[0][2];

            const x = yI * zJ - zI * yJ;
            const y = zI * xJ - xI * zJ;
            const z = xI * yJ - yI * xJ;
            if (x * x + y * y + z * z > 0) break;
        }

        // Dot products make a projection on plan (I, J).
        vectors3.forEach(function(vector3) {
            const x = vector3[0];
            const y = vector3[1];
            const z = vector3[2];
            output.push(
                xI * x + yI * y + zI * z,
                xJ * x + yJ * y + zJ * z
            );
        });

        return new Float32Array(output);
    }
    catch (ex) {
        console.error("[webgl/fillpoly.vertexArrayToAttribs] error = ", ex);
        console.error("[webgl/fillpoly.vertexArrayToAttribs] vectors3 = ", vectors3);
        throw ex + "\n...in webgl/fillpoly.vertexArrayToAttribs";
    }
}

/**
 * In wavefront th first element always has an index of 1.
 * But you can also use negative indexes to access elements.
 * -1 returns you the last element, -2 the one before the last, etc.
 */
function elementAt(arr: any[], idx: number): any {
    if (idx === 0) throw Error('0 is not a valid Wavefront index!');
    if (idx > 0) return arr[idx - 1];
    return arr[arr.length + idx];
}


/**
 * Return indexes of the faces.
 * The number of indexes is a multiple of 3 because each resulting face will have 3 vertices.
 *
 * >>> vertices - Array of vertices attributes.
 * We expect at least two attributes per vertex: X and Y.
 *
 * >>> attributesCount - Most of the time you will give only two attributes,
 * but if you need more, you can specify how many you are using,
 * provided that the first two always are X and Y.
 *
 * >>> offset - Number of vertices to skip.
 */
function fillPolyline(vertices: Float32Array,
                      attributesCount: number = 2,
                      offset: number = 0): number[] {
    const polyline = new Polyline(vertices, attributesCount);
    const vertexCount = Math.floor(vertices.length / attributesCount);
    const expectedTriangles = vertexCount - 2;
    const elemBuff = doFillPolyline(-1, polyline, offset, attributesCount);
    if (elemBuff && elemBuff.length === 3 * expectedTriangles) {
        return elemBuff;
    }
    const polyline2 = new Polyline(vertices, attributesCount);
    return doFillPolyline(+1, polyline2, offset, attributesCount);
}

function doFillPolyline(orientation: number,
                        polyline: Polyline,
                        offset: number,
                        attributesCount: number): number[] {
    const elemBuff = [];
    while (polyline.length > 3) {
        let count = 0;
        while (!polyline.isCandidate(orientation) && count++ < polyline.length) {
            count++;
            polyline.next();
        }
        if (count >= polyline.length) return [];
        const A = polyline.get();
        elemBuff.push(offset + A.index, offset + A.next, offset + A.prev);
        polyline.removeTriangle(A.next, A.prev);
    }

    if (!polyline.isCandidate(orientation)) return [];

    const A = polyline.get();
    const B = polyline.get(A.next);
    const C = polyline.get(A.prev);
    elemBuff.push(offset + A.index, offset + B.index, offset + C.index);

    return elemBuff;
}

const PTR = 0;
const PREV = 1;
const NEXT = 2;

class Polyline {
    length: number = 0;
    private _cursor = 0;
    private readonly _vertices: Float32Array;
    private readonly _points: [number,number,number][];

    constructor(vertices: Float32Array, attributesCount: number) {
        const points: [number,number,number][] = [];
        const size = Math.floor(vertices.length / attributesCount);
        for (let k = 0; k < size; k++) {
            points.push([k * attributesCount, (k + size - 1) % size, (k + 1) % size]);
        }
        this.length = size;
        this._vertices = vertices;
        this._cursor = 0;
        this._points = points;
    }

    isCandidate(orientation: number) {
        const idxA = this._cursor;
        const A = this.get(idxA);
        const idxB = A.next;
        const idxC = A.prev;
        const AB = this.getVector(idxA, idxB);
        const AC = this.getVector(idxA, idxC);
        const crossprod = this.cross(AB, AC);

        if (crossprod > 0 && orientation > 0) return false;
        if (crossprod < 0 && orientation < 0) return false;

        // On vérifie maintenant qu'aucun point n'est à l'intérieur du triangle.
        for (let idxM = 0; idxM < this._points.length; idxM++) {
            if (idxM === idxA) continue;
            if (idxM === idxB) continue;
            if (idxM === idxC) continue;

            if (this.isInTriangle(idxA, idxM, orientation)) return false;
        }
        return true;
    }

    get(index: number | undefined = undefined) {
        if (typeof index === 'undefined') index = this._cursor;
        const point = this._points[index];
        const pointer = point[PTR];
        return {
            index: index,
            next: point[NEXT],
            prev: point[PREV],
            x: this._vertices[pointer],
            y: this._vertices[pointer + 1]
        };
    }

    getVector(idxA: number, idxB: number): IVector2 {
        const A = this.get(idxA);
        const B = this.get(idxB);
        return {
            x: B.x - A.x,
            y: B.y - A.y
        };
    }

    cross(u: IVector2, v: IVector2) {
        return u.x * v.y - u.y * v.x;
    }

    /**
     * @param {number} orientation - Signe du produit scalaire AB^AC.
     */
    isInTriangle(idxA: number, idxM: number, orientation: number) {
        const A = this.get(idxA);
        const idxB = A.next;
        const idxC = A.prev;

        if (orientation > 0) return this.isInTrianglePos(idxA, idxB, idxC, idxM);
        return this.isInTriangleNeg(idxA, idxB, idxC, idxM);
    }

    isInTriangleNeg(idxA: number, idxB: number, idxC: number, idxM: number) {
        const AB = this.getVector(idxA, idxB);
        const AM = this.getVector(idxA, idxM);
        if (this.cross(AB, AM) < 0) return false;
        const BC = this.getVector(idxB, idxC);
        const BM = this.getVector(idxB, idxM);
        if (this.cross(BC, BM) < 0) return false;
        const CA = this.getVector(idxC, idxA);
        const CM = this.getVector(idxC, idxM);
        if (this.cross(CA, CM) < 0) return false;
        return true;
    }

    isInTrianglePos(idxA: number, idxB: number, idxC: number, idxM: number) {
        const AB = this.getVector(idxA, idxB);
        const AM = this.getVector(idxA, idxM);
        if (this.cross(AB, AM) > 0) return false;
        const BC = this.getVector(idxB, idxC);
        const BM = this.getVector(idxB, idxM);
        if (this.cross(BC, BM) > 0) return false;
        const CA = this.getVector(idxC, idxA);
        const CM = this.getVector(idxC, idxM);
        if (this.cross(CA, CM) > 0) return false;
        return true;
    }

    next() {
        const index = this._cursor;
        this._cursor = this._points[index][NEXT];
    }

    removeTriangle(idxNxt: number, idxPrv: number) {
        this._points[idxPrv][NEXT] = idxNxt;
        this._points[idxNxt][PREV] = idxPrv;
        this.length--;

        this._cursor = idxNxt;
        let index = idxNxt;
        do {
            index = this._points[index][NEXT];
        } while (index !== idxNxt);
    }
}
