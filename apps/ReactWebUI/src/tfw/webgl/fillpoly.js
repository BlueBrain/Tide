/**
 * @param {Float32Array} vertices - attributs of each vertex. The first must be X and the second Y.
 * @param {number=2} attributesCount - Number of attribute per vertex. At least 2 (X and Y).
 * @param {number=0}  offset - Index  of the first  element. Usefull when  you need to  concat several
 * elements buffers.
 * @return {Uint16Array}
 */
exports.fillPolyline = fillPolyline;
/**
 * Sometimes, your polygon  is expressed in 3 dimensions.   So, if you
 * want to use `fillPolyline`, tjhis function will flatten the polygon
 * for you.
 * @param {array} vectors3 - [[x, y, z], ...]
 * @return {Float32Array} [ x1, y1, x2, y2, x3, y3, x4, y3, ... ]
 */
exports.vertexArrayToAttribs = vertexArrayToAttribs;


function vertexArrayToAttribs(vectors3) {
    try {
        var output = [];
        if (vectors3.length < 3)
            throw "[webgl.fillpoly.vertexArrayToAttribs] A polyline must have at least 3 vertices!";

        var xI = vectors3[1][0] - vectors3[0][0];
        var yI = vectors3[1][1] - vectors3[0][1];
        var zI = vectors3[1][2] - vectors3[0][2];

        var xJ, yJ, zJ;
        var k, x, y, z;
        for (k = 2; k < vectors3.length; k++) {
            xJ = vectors3[k][0] - vectors3[0][0];
            yJ = vectors3[k][1] - vectors3[0][1];
            zJ = vectors3[k][2] - vectors3[0][2];

            x = yI * zJ - zI * yJ;
            y = zI * xJ - xI * zJ;
            z = xI * yJ - yI * xJ;
            if (x * x + y * y + z * z > 0) break;
        }

        // Dot products make a projection on plan (I, J).
        vectors3.forEach(function(vector3) {
            var x = vector3[0];
            var y = vector3[1];
            var z = vector3[2];
            output.push(
                xI * x + yI * y + zI * z,
                xJ * x + yJ * y + zJ * z
            );
        });

        return new Float32Array(output);
    } catch (ex) {
        console.error("[webgl.fillpoly.vertexArrayToAttribs] error = ", ex);
        console.error("[webgl.fillpoly.vertexArrayToAttribs] vectors3 = ", vectors3);
        throw ex + "\n...in webgl.fillpoly.vertexArrayToAttribs";
    }
}


//#(fillPolyline)
// @param {Float32Array}  vertices -  Attributs de chaque  vertex. Les
// deux premiers doivent être X et Y.
// @param   {number=2}  attributesCount   -  Nombre   d'attributs  par
// vertex. Au minimum 2 (X et Y).
// @param {number=0}  offset -  Index du premier  vertex. Utile  si on
// veut dessiner plusieurs polygônes avec le même buffer.
// @return {Uint16Array} Tableau des indices des vertex.
function fillPolyline(vertices, attributesCount, offset) {
    if (typeof attributesCount === 'undefined') attributesCount = 2;
    if (typeof offset === 'undefined') offset = 0;

    var polyline = new Polyline(vertices, attributesCount);
    var vertexCount = Math.floor(vertices.length / attributesCount);
    var expectedTriangles = vertexCount - 2;
    var elemBuff = doFillPolyline(-1, polyline, offset, attributesCount);
    if (elemBuff && elemBuff.length === 3 * expectedTriangles) {
        return elemBuff;
    }
    polyline = new Polyline(vertices, attributesCount);
    return new Uint16Array(doFillPolyline(+1, polyline, offset, attributesCount));
}

function doFillPolyline(orientation, polyline, offset, attributesCount) {
    var A, B, C;
    var elemBuff = [];
    while (polyline.length > 3) {
        var count = 0;
        while (!polyline.isCandidate(orientation) && count++ < polyline.length) {
            count++;
            polyline.next();
        }
        if (count >= polyline.length) return null;
        A = polyline.get();
        elemBuff.push(offset + A.index, offset + A.next, offset + A.prev);
        polyline.removeTriangle(A.index, A.next, A.prev);
    }

    if (!polyline.isCandidate(orientation)) return null;

    A = polyline.get();
    B = polyline.get(A.next);
    C = polyline.get(A.prev);
    elemBuff.push(offset + A.index, offset + B.index, offset + C.index);

    return elemBuff;
}

var PTR = 0;
var PREV = 1;
var NEXT = 2;

var Polyline = function(vertices, attributesCount) {
    var points = [];
    var size = Math.floor(vertices.length / attributesCount);
    for (var k = 0; k < size; k++) {
        points.push([k * attributesCount, (k + size - 1) % size, (k + 1) % size]);
    }
    this.length = size;
    this._vertices = vertices;
    this._cursor = 0;
    this._points = points;
};

Polyline.prototype.isCandidate = function(orientation) {
    var idxA = this._cursor;
    var A = this.get(idxA);
    var idxB = A.next;
    var idxC = A.prev;
    var AB = this.getVector(idxA, idxB);
    var AC = this.getVector(idxA, idxC);
    var crossprod = this.cross(AB, AC);

    if (crossprod > 0 && orientation > 0) return false;
    if (crossprod < 0 && orientation < 0) return false;

    // On vérifie maintenant qu'aucun point n'est à l'intérieur du triangle.
    for (var idxM = 0; idxM < this._points.length; idxM++) {
        if (idxM === idxA) continue;
        if (idxM === idxB) continue;
        if (idxM === idxC) continue;

        if (this.isInTriangle(idxA, idxM, orientation)) return false;
    }
    return true;
};

Polyline.prototype.get = function(index) {
    if (typeof index === 'undefined') index = this._cursor;
    var point = this._points[index];
    var pointer = point[PTR];
    return {
        index: index,
        next: point[NEXT],
        prev: point[PREV],
        x: this._vertices[pointer],
        y: this._vertices[pointer + 1]
    };
};

Polyline.prototype.getVector = function(idxA, idxB) {
    var A = this.get(idxA);
    var B = this.get(idxB);
    return {
        x: B.x - A.x,
        y: B.y - A.y
    };
};

Polyline.prototype.cross = function(u, v) {
    return u.x * v.y - u.y * v.x;
};

/**
 * @param {number} orientation - Signe du produit scalaire AB^AC.
 */
Polyline.prototype.isInTriangle = function(idxA, idxM, orientation) {
    var A = this.get(idxA);
    var idxB = A.next;
    var idxC = A.prev;

    if (orientation > 0) return this.isInTrianglePos(idxA, idxB, idxC, idxM);
    return this.isInTriangleNeg(idxA, idxB, idxC, idxM);
}

Polyline.prototype.isInTriangleNeg = function(idxA, idxB, idxC, idxM) {
    var AB = this.getVector(idxA, idxB);
    var AM = this.getVector(idxA, idxM);
    if (this.cross(AB, AM) < 0) return false;
    var BC = this.getVector(idxB, idxC);
    var BM = this.getVector(idxB, idxM);
    if (this.cross(BC, BM) < 0) return false;
    var CA = this.getVector(idxC, idxA);
    var CM = this.getVector(idxC, idxM);
    if (this.cross(CA, CM) < 0) return false;
    return true;
};

Polyline.prototype.isInTrianglePos = function(idxA, idxB, idxC, idxM) {
    var AB = this.getVector(idxA, idxB);
    var AM = this.getVector(idxA, idxM);
    if (this.cross(AB, AM) > 0) return false;
    var BC = this.getVector(idxB, idxC);
    var BM = this.getVector(idxB, idxM);
    if (this.cross(BC, BM) > 0) return false;
    var CA = this.getVector(idxC, idxA);
    var CM = this.getVector(idxC, idxM);
    if (this.cross(CA, CM) > 0) return false;
    return true;
};

Polyline.prototype.next = function() {
    var index = this._cursor;
    this._cursor = this._points[index][NEXT];
};

Polyline.prototype.removeTriangle = function(idxCur, idxNxt, idxPrv) {
    this._points[idxPrv][NEXT] = idxNxt;
    this._points[idxNxt][PREV] = idxPrv;
    this.length--;

    this._cursor = idxNxt;
    var txt = '';
    var index = idxNxt;
    do {
        txt += alpha(index);
        index = this._points[index][NEXT];
    } while (index !== idxNxt);
};

//#(fillPolyline)


var ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

function alpha(index) {
    var txt = '';
    Array.prototype.slice.call(arguments).forEach(function(arg) {
        txt += ALPHABET.charAt(arg);
    });
    return txt;
}


/*
 window.Jasmine = {
 describe: [],
 specCount: 0,
 failCount: 0
 };


 window.describe = function( name, slot ) {
 Jasmine.describe.push( name.trim() );
 try {
 slot();
 }
 catch( ex ) {
 console.error( "Failure! ", Jasmine.describe.join(" ") );
 console.error( ex );
 }
 Jasmine.describe.pop();
 if( Jasmine.describe.length === 0 ) {
 console.log("============================================================");
 console.log(" Number of tests: ", Jasmine.specCount );
 console.log(" Success: ", Jasmine.specCount - Jasmine.failCount );
 console.log(" Failure: ", Jasmine.failCount );
 console.log(" Quality: ",
 parseFloat((100*(Jasmine.specCount - Jasmine.failCount) / Jasmine.specCount).toFixed(1)),
 "%" );
 console.log("------------------------------------------------------------");
 }
 };

 window.it = function( name, slot ) {
 Jasmine.describe.push( name.trim() );
 Jasmine.specCount++;
 try {
 slot();
 }
 catch( ex ) {
 Jasmine.failCount++;
 console.error( "Failure! ", Jasmine.describe.join(" "), "\n", ex );
 }
 Jasmine.describe.pop();
 };


 describe('webgl.fillpoly', function() {
 describe('cross', function() {
 [
 [0,0, 1,0, 0,-1, -1],
 [0,0, 1,0, 0,1, 1]
 ].forEach(function (item) {
 var Ax = item[0];
 var Ay = item[1];
 var Bx = item[2];
 var By = item[3];
 var Mx = item[4];
 var My = item[5];
 var value = item[6];
 it("(" + Ax + "," + Ay + "," + Bx + "," + By + "," + Mx + "," + My + ") should give " + value, function() {
 var Ux = Bx - Ax;
 var Uy = By - Ay;
 var Vx = Mx - Ax;
 var Vy = My - Ay;
 var expected = Ux*Vy - Uy*Vx;
 if( value !== expected )
 throw("Expected " + value + " to be " + expected);
 });
 });
 });
 describe('hasPointInsideNeg', function() {
 [
 [0,0,1,0,0,-1, .2,.2, false],
 [0,0,1,0,0,-1, -.2,0, false],
 [0,0,1,0,0,-1, -.1,-.1, false],
 [0,0,1,0,0,-1, .5,-.5, true],
 [0,0,1,0,0,-1, .5,0, true],
 [0,0,1,0,0,-1, 0,-.5, true],
 [0,0,1,0,0,-1, .2,-.2, true],
 [0,0,1,0,0,-1, .3,-.1, true],
 [0,0,1,0,0,-1, 1,0, true],
 [0,0,1,0,0,-1, 0,-1, true],
 [0,0,1,0,0,-1, 0,0, true]
 ].forEach(function (item) {
 var Ax = item[0];
 var Ay = item[1];
 var Bx = item[2];
 var By = item[3];
 var Cx = item[4];
 var Cy = item[5];
 var Mx = item[6];
 var My = item[7];
 var value = item[8];
 it('(' + Ax + "," + Ay + "," + Bx + "," + By + "," + Cx + "," + Cy + "," + Mx + "," + My
 + ') should return ' + value, function(){
 var result = hasPointInsideNeg(Ax,Ay,Bx,By,Cx,Cy,Mx,My);
 if( result !== value )
 throw "Expected " + result + " to be " + value + "!";
 });
 });
 });
 describe('hasPointInsidePos', function() {
 [
 [0,0,1,0,0,1, .2,.2, true],
 [0,0,1,0,0,1, -.2,0, false],
 [0,0,1,0,0,1, -.1,-.1, false],
 [0,0,1,0,0,1, .5,-.5, false],
 [0,0,1,0,0,1, .5,0, true],
 [0,0,1,0,0,1, 0,-.5, false],
 [0,0,1,0,0,1, .2,-.2, false],
 [0,0,1,0,0,1, .3,-.1, false],
 [0,0,1,0,0,1, 1,0, true],
 [0,0,1,0,0,1, 0,-1, false],
 [0,0,1,0,0,1, 0,0, true]
 ].forEach(function (item) {
 var Ax = item[0];
 var Ay = item[1];
 var Bx = item[2];
 var By = item[3];
 var Cx = item[4];
 var Cy = item[5];
 var Mx = item[6];
 var My = item[7];
 var value = item[8];
 it('(' + Ax + "," + Ay + "," + Bx + "," + By + "," + Cx + "," + Cy + "," + Mx + "," + My
 + ') should return ' + value, function(){
 var result = hasPointInsidePos(Ax,Ay,Bx,By,Cx,Cy,Mx,My);
 if( result !== value )
 throw "Expected " + result + " to be " + value + "!";
 });
 });
 });
 });
 */