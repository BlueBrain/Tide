/**
 * Wavefront format can be found here:
 * https://en.wikipedia.org/wiki/Wavefront_.obj_file#File_format
 */
import Iter from '../../iterator'
import Fillpoly from '../fillpoly'

import { IMeshDefinition, IMeshDict } from '../types'

export default { parseObjFile }


interface IParsingContext extends IMeshDefinition {
    meshCount: number,
    dict: IMeshDict,
    // Because faces can share vertices, we must store vertices in a dictionary to avoid
    // duplications. A key looks like this: `42/2/4`. That means:
    // `<vertex index>/<texture UV index>/<normal index>`
    verticesDict: {[key: string]: number},
    lineNumber: number
}

const RX_SEP = /[ \t\n\r,;]+/

function parseObjFile(content: string): IMeshDict {
    const ctx: IParsingContext = {
        meshCount: 0,
        dict: {},
        name: '',
        type: '?',
        verticesXYZ: [],
        normalsXYZ: [],
        verticesUV: [],
        faces: [],
        verticesDict: {},
        attributes: [],
        elements: [],
        verticesCount: 0,
        lineNumber: 0
    };

    for (const line of Iter.lines(content)) {
        ctx.lineNumber++;
        if (isCommentOrEmpty(line)) continue;
        if (isObjectName(ctx, line)) continue;
        if (isVertex(ctx, line)) continue;
        if (isTexture(ctx, line)) continue;
        if (isNormal(ctx, line)) continue;
        if (isFace(ctx, line)) continue;

        throw Error(`Unable to parse line #${ctx.lineNumber}:\n${line}`);
    }

    addCurrentMeshToDict(ctx);
    return ctx.dict;
}

function addCurrentMeshToDict(ctx: IParsingContext) {
    if (ctx.meshCount > 0) {
        const mesh: IMeshDefinition = {
            name: ctx.name,
            verticesXYZ: ctx.verticesXYZ.slice(),
            faces: ctx.faces.slice(),
            type: ctx.type,
            verticesUV: ctx.verticesUV.slice(),
            normalsXYZ: ctx.normalsXYZ.slice(),
            attributes: [],
            verticesCount: ctx.verticesCount,
            elements: []
        };
        ctx.dict[ctx.name] = mesh;
        // Reset vertices map.
        ctx.verticesDict = {};
    }
}

function isVertex(ctx: IParsingContext, line: string): boolean {
    if (!line.startsWith('v ')) return false;
    ctx.verticesXYZ.push(parseXYZ(line));
    return true;
}

function isTexture(ctx: IParsingContext, line: string): boolean {
    if (!line.startsWith('vt ')) return false;
    ctx.verticesUV.push(parseUV(line));
    return true;
}

function isNormal(ctx: IParsingContext, line: string): boolean {
    if (!line.startsWith('vn ')) return false;
    ctx.normalsXYZ.push(parseXYZ(line));
    return true;
}

function isFace(ctx: IParsingContext, line: string): boolean {
    if (!line.startsWith('f ')) return false;
    const facePoints: string[] = line.substr(2).split(RX_SEP);
    const faceDetails: number[][] = facePoints
        .map((face: string) => face.split('/')
        .map((n: string) => parseInt(n, 10)));
    const vertices = getFacesVertices(ctx, faceDetails);
    addResultingAttributesAndElements(ctx, facePoints, faceDetails, vertices);

    return true;
}

function getFacesVertices(ctx: IParsingContext, faceDetails: number[][]) {
    const vertices: [number,number,number][] = [];

    faceDetails.forEach(point => {
        const [idx] = point;
        const [x,y,z] = Fillpoly.elementAt(ctx.verticesXYZ, idx);
        vertices.push([x,y,z]);
    })
    return vertices;
}

function addResultingAttributesAndElements(ctx: IParsingContext,
                                           facePoints: string[],
                                           faceDetails: number[][],
                                           vertices: [number,number,number][])
{
    const indexes = facePoints.length === 3 ? [0,1,2] :
        Fillpoly.fillPolyline(
            Fillpoly.vertexArrayToAttribs(vertices)
        );
    for( const index of indexes ) {
        const key = facePoints[index];
        if (ctx.verticesDict[key]) continue;

    }
}

function isObjectName(ctx: IParsingContext, line: string): boolean {
    if (!line.startsWith('o ')) return false;

    addCurrentMeshToDict(ctx);

    ctx.meshCount++;
    ctx.name = line.substr(2).trim();
    ctx.verticesXYZ = [];
    ctx.normalsXYZ = [];
    ctx.verticesUV = [];
    ctx.faces = [];

    return true;
}

/**
 * Return `true` as soon as `line` is empty or represent a comment (starting with `#`).
 */
function isCommentOrEmpty(line: string): boolean {
    if (line.length === 0) return true;
    const firstChar = line.charAt(0);
    if (firstChar === '#') return true;
    return false;
}


function parseXYZ(text: string): [number, number, number] {
    const [x, y, z] = text.split(RX_SEP);
    const nx = parseFloat(x);
    const ny = parseFloat(y);
    const nz = parseFloat(z);
    if (isNaN(nx) || isNaN(ny) || isNaN(nz)) {
        throw Error(`Expected X,Y,Z coorcinates but got "${text}"!`)
    }
    return [nx, ny, nz];
}

function parseUV(text: string): [number, number] {
    const [u, v] = text.split(RX_SEP);
    const nu = parseFloat(u);
    const nv = parseFloat(v);
    if (isNaN(nu) || isNaN(nv)) {
        throw Error(`Expected U,V coorcinates but got "${text}"!`)
    }
    return [nu, nv];
}
