
export interface IMeshDefinition {
    verticesXYZ: [number, number, number][],
    // Each point of a face is a triplet of indices:
    // [vertex, texture?, normal?].
    faces: [number, number?, number?][][],
    normalsXYZ: [number, number, number][],
    verticesUV: [number, number][],
    name: string,
    // A mesh can be defined by only vertices ("N"),
    // or have also texture coordinates ("T")
    // or normal vectors ("N").
    // The number of attributes depends on that.
    type: "?" | "V" | "VN" | "VT" | "VTN",
    // Resulting vertices attributes.
    // Each element represents a vertex as an array of its attributes.
    attributes: number[][],
    verticesCount: number,
    // Resulting vertices indexes.
    elements: number[]

}

export interface IMeshDict {
    [key: string]: IMeshDefinition
}
