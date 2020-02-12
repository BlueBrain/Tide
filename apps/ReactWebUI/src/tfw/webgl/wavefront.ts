import Fillpoly from "./fillpoly";


export function parse(content) {
    const model = parseResult.call(this, content);
    console.info("[wdg.wavefront] model=", model);
    this.vertCount = model.vert.length / 6;
    this.elemCount = model.elem.length;
    this.translation.z = -model.radius;
    console.info("[wdg.wavefront] this.translation=", this.translation);

    var gl = this.gl;

    if (!this.buffVert) {
        this.buffVert = gl.createBuffer();
    }
    gl.bindBuffer(gl.ARRAY_BUFFER, this.buffVert);
    gl.bufferData(gl.ARRAY_BUFFER, model.vert, gl.STATIC_DRAW);

    if (!this.buffElem) {
        this.buffElem = gl.createBuffer();
    }
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.buffElem);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, model.elem, gl.STATIC_DRAW);
}

function parseResult(content) {
    var def = parseWaveFront.call(this, content);
    var vertArray = [];
    var elemArray = [];
    var vertices = {};
    var count = 0;
    var maxLength = 0;
    console.log("XYZ =", def.v);
    console.log("UV =", def.vt);
    def.f.forEach(function(triangle) {
        triangle.forEach(function(point) {
            var vertexIndex = vertices[point];
            if (typeof vertexIndex === 'undefined') {
                vertexIndex = count++;
                vertices[point] = vertexIndex;
                const pointPieces = point.split('/');
                const vIdx = pointPieces[0];
                const vtIdx = pointPieces[1];
                const vnIdx = pointPieces[2];
                const v = def.v[vIdx];
                const vn = def.vn[vnIdx];
                const vt = def.vt[vtIdx];
                const x = v[0], y = v[1], z = v[2];
                const length = x * x + y * y + z * z;
                maxLength = Math.max(maxLength, length);
                const xn = vn[0], yn = vn[1], zn = vn[2];
                vertArray.push(x, y, z, xn, yn, zn, 1.3 - vt[0], vt[1]);
            }
            elemArray.push(vertexIndex);
        });
    });
    return {
        vert: new Float32Array(vertArray),
        elem: new Uint16Array(elemArray),
        radius: Math.sqrt(maxLength)
    };
}


function parseWaveFront(content) {
    var def = waveFrontToDefinitions(content);
    this.vertexCount = def.v.length;
    this.normalCount = def.vn.length;
    this.faceCount = def.f.length;

    // [[x,y,z], ...]
    def.v = def.v.map(toVector3);
    def.v.unshift(null); // Because indexes must start from 1.
    // [[u,v], ...]
    def.vt = def.vt.map(toVector2);
    def.vt.unshift(null); // Because indexes must start from 1.
    // [[nx,ny,nz], ...]
    def.vn = def.vn.map(toVector3);
    def.vn.unshift(null); // Because indexes must start from 1.
    // [ ["7//1", "10//2", "4//3"], ... ]
    var F = [];
    cutFacesInTriangles(F, def);
    this.triangleCount = F.length;
    def.f = F;

    return def;
}


class TextFileIterator {
    constructor(content) {
        this.content = content;
        this.length = content.length;
        this.cursor = 0;
    };

    /**
     * @member TextFileIterator.nextLine
     */
    nextLine() {
        var cursor = this.cursor;
        if (cursor >= this.length) return null;
        var nextEndOfLine = this.content.indexOf('\n', cursor);
        if (nextEndOfLine < 0) {
            this.cursor = this.length;
            return this.content.substr(cursor);
        }
        this.cursor = nextEndOfLine + 1;
        return this.content.substr(cursor, this.cursor - cursor);
    };
}

var RX_PREFIX = /^([a-z]+)[ ]+/g;

function waveFrontToDefinitions(content) {
    var itr = new TextFileIterator(content);
    var line;
    var def = {};
    while (null != (line = itr.nextLine())) {
        RX_PREFIX.lastIndex = 0;
        var matcher = RX_PREFIX.exec(line);
        if (!matcher) continue;
        var key = matcher[1];
        var val = line.substr(matcher[0].length).trim();
        if (!Array.isArray(def[key])) def[key] = [];
        def[key].push(val);
    }
    return def;
}


function cutFacesInTriangles(triangles, def) {
    def.f.forEach(function(faceDef, faceDefIndex) {
        try {
            var poly = [];
            var face = faceDef.split(' ');
            if (face.length === 3) {
                triangles.push(face);
            }
            else {
                // More than 3 vertices, we must cut in triangles.
                face.forEach(function(vertex) {
                    var vertexPieces = vertex.split('/');
                    var idxVertex = parseInt(vertexPieces[0]);
                    poly.push(def.v[idxVertex]);
                });

                var vertices = Fillpoly.vertexArrayToAttribs(poly);
                var trianglesElements = Fillpoly.fillPolyline(vertices);
                for (var k = 0; k < trianglesElements.length / 3; k++) {
                    var idx0 = trianglesElements[k * 3 + 0];
                    var idx1 = trianglesElements[k * 3 + 1];
                    var idx2 = trianglesElements[k * 3 + 2];
                    triangles.push([face[idx0], face[idx1], face[idx2]]);
                }
            }
        }
        catch (ex) {
            console.error("[wdg.wavefront.cutFacesInTriangles] error = ", ex);
            console.error("[wdg.wavefront.cutFacesInTriangles] faceDef = ", faceDef);
            console.error("[wdg.wavefront.cutFacesInTriangles] faceDefIndex = ", faceDefIndex);
            console.error("[wdg.wavefront.cutFacesInTriangles] face = ", face);
            console.error("[wdg.wavefront.cutFacesInTriangles] poly = ", poly);
            console.error("[wdg.wavefront.cutFacesInTriangles] vertices = ", vertices);
            console.error("[wdg.wavefront.cutFacesInTriangles] trianglesElements = ", trianglesElements);
            throw ex + "\n...in wdg.wavefront.cutFacesInTriangles";
        }
    });
    return triangles;
}


function toVector3(txt) {
    return txt.split(" ").map(parseFloat);
}

function toVector2(txt) {
    return txt.split(" ").map(parseFloat);
}
